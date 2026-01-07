// ====================================================================================
// ESP32-S3 Game Console UI - Arduino IDE Version
// ST7789 Driver with DMA + LVGL v9.3
// ====================================================================================

#include <Arduino.h>
#include <lvgl.h>
#include <SPI.h>
#include <SD.h>
#include <FS.h>

// Pin Definitions - Display
#define TFT_LEDK 39
#define TFT_DC 47
#define TFT_CS 14
#define TFT_MOSI 12
#define TFT_SCLK 48
#define TFT_RST 3

// Pin Definitions - Buttons
#define KEY_UP 7
#define KEY_DOWN 46
#define KEY_LEFT 45
#define KEY_RIGHT 6
#define KEY_MENU 18
#define KEY_OPTION 8
#define KEY_SELECT 16
#define KEY_START 17
#define KEY_A 15
#define KEY_B 5

// Pin Definitions - SD Card
#define SD_CS 10
#define SD_CMD 11
#define SD_CLK 13
#define SD_DAT0 9

// Display settings - LANDSCAPE MODE
#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 240
#define LVGL_BUFFER_SIZE (SCREEN_WIDTH * 40)

// ST7789 Commands
#define ST7789_NOP 0x00
#define ST7789_SWRESET 0x01
#define ST7789_SLPOUT 0x11
#define ST7789_NORON 0x13
#define ST7789_INVOFF 0x20
#define ST7789_INVON 0x21
#define ST7789_DISPON 0x29
#define ST7789_CASET 0x2A
#define ST7789_RASET 0x2B
#define ST7789_RAMWR 0x2C
#define ST7789_MADCTL 0x36
#define ST7789_COLMOD 0x3A

// Global objects
SPIClass *spi_tft = NULL;
static lv_display_t *disp;
static lv_indev_t *indev_keypad;

// UI Screens
lv_obj_t *screen_home;
lv_obj_t *screen_file_manager;
lv_obj_t *screen_notes;

// UI Elements
lv_obj_t *label_time;
lv_obj_t *label_date;
lv_obj_t *label_battery;
lv_obj_t *label_wifi;
lv_obj_t *file_list;
lv_obj_t *keyboard;
lv_obj_t *notes_textarea;

// State variables
uint8_t current_screen = 0;
String current_path = "/";
bool keyboard_visible = false;
uint8_t selected_app = 0; // Track selected app on home screen
bool app_focused = false;
lv_group_t *input_group;

// DMA Buffer for fast transfer
uint16_t *dma_buffer = NULL;

// ====================================================================================
// ST7789 Low-Level Driver Functions
// ====================================================================================

void tft_cmd(uint8_t cmd) {
    digitalWrite(TFT_DC, LOW);
    digitalWrite(TFT_CS, LOW);
    spi_tft->transfer(cmd);
    digitalWrite(TFT_CS, HIGH);
}

void tft_data(uint8_t data) {
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    spi_tft->transfer(data);
    digitalWrite(TFT_CS, HIGH);
}

void tft_data16(uint16_t data) {
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    spi_tft->transfer(data >> 8);
    spi_tft->transfer(data & 0xFF);
    digitalWrite(TFT_CS, HIGH);
}

void tft_init() {
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_LEDK, OUTPUT);
    
    digitalWrite(TFT_CS, HIGH);
    digitalWrite(TFT_DC, HIGH);
    
    // Hardware reset
    digitalWrite(TFT_RST, LOW);
    delay(10);
    digitalWrite(TFT_RST, HIGH);
    delay(120);
    
    // Software reset
    tft_cmd(ST7789_SWRESET);
    delay(150);
    
    // Sleep out
    tft_cmd(ST7789_SLPOUT);
    delay(120);
    
    // Color mode - 16bit RGB565
    tft_cmd(ST7789_COLMOD);
    tft_data(0x55);
    
    // Memory data access control - LANDSCAPE (rotation 270 degrees)
    tft_cmd(ST7789_MADCTL);
    tft_data(0xA0); // Rotation 270 (landscape flipped)
    
    // Normal display on
    tft_cmd(ST7789_NORON);
    delay(10);
    
    // Display on
    tft_cmd(ST7789_DISPON);
    delay(10);
    
    // Backlight on
    digitalWrite(TFT_LEDK, HIGH);
}

void tft_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    tft_cmd(ST7789_CASET);
    tft_data16(x0);
    tft_data16(x1);
    
    tft_cmd(ST7789_RASET);
    tft_data16(y0);
    tft_data16(y1);
    
    tft_cmd(ST7789_RAMWR);
}

void tft_push_colors(uint16_t *data, uint32_t len) {
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    
    spi_tft->transferBytes((uint8_t*)data, NULL, len * 2);
    
    digitalWrite(TFT_CS, HIGH);
}

void tft_fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    tft_set_addr_window(x, y, x + w - 1, y + h - 1);
    
    uint32_t total = w * h;
    digitalWrite(TFT_DC, HIGH);
    digitalWrite(TFT_CS, LOW);
    
    while(total--) {
        spi_tft->transfer16(color);
    }
    
    digitalWrite(TFT_CS, HIGH);
}

// ====================================================================================
// LVGL Display Driver
// ====================================================================================

void my_disp_flush(lv_display_t *disp_drv, const lv_area_t *area, uint8_t *px_map) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    
    tft_set_addr_window(area->x1, area->y1, area->x2, area->y2);
    tft_push_colors((uint16_t*)px_map, w * h);
    
    lv_disp_flush_ready(disp_drv);
}

// ====================================================================================
// Input Device Driver (Keypad)
// ====================================================================================

void read_keypad(lv_indev_t *indev, lv_indev_data_t *data) {
    static uint32_t last_key_time = 0;
    static bool was_pressed = false;
    uint32_t current_time = millis();
    
    // Check if any key is pressed
    bool any_pressed = false;
    
    // Navigation keys
    if (digitalRead(KEY_UP) == LOW) {
        data->key = LV_KEY_UP;
        any_pressed = true;
    } else if (digitalRead(KEY_DOWN) == LOW) {
        data->key = LV_KEY_DOWN;
        any_pressed = true;
    } else if (digitalRead(KEY_LEFT) == LOW) {
        data->key = LV_KEY_LEFT;
        any_pressed = true;
    } else if (digitalRead(KEY_RIGHT) == LOW) {
        data->key = LV_KEY_RIGHT;
        any_pressed = true;
    } 
    // Action keys
    else if (digitalRead(KEY_SELECT) == LOW || digitalRead(KEY_A) == LOW) {
        data->key = LV_KEY_ENTER;
        any_pressed = true;
    } else if (digitalRead(KEY_MENU) == LOW || digitalRead(KEY_B) == LOW) {
        data->key = LV_KEY_ESC;
        any_pressed = true;
    } else if (digitalRead(KEY_OPTION) == LOW) {
        data->key = LV_KEY_BACKSPACE;
        any_pressed = true;
    } else if (digitalRead(KEY_START) == LOW) {
        data->key = LV_KEY_HOME;
        any_pressed = true;
    }
    
    if (any_pressed) {
        if (!was_pressed || (current_time - last_key_time > 150)) {
            data->state = LV_INDEV_STATE_PRESSED;
            last_key_time = current_time;
            was_pressed = true;
            
            // Debug print
            Serial.print("Key pressed: ");
            Serial.println(data->key);
        } else {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
        was_pressed = false;
    }
    
    data->continue_reading = false;
}

// ====================================================================================
// Status Bar Functions
// ====================================================================================

void create_status_bar(lv_obj_t *parent) {
    lv_obj_t *status_bar = lv_obj_create(parent);
    lv_obj_set_size(status_bar, SCREEN_WIDTH, 20);
    lv_obj_align(status_bar, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(status_bar, lv_color_hex(0x808080), 0);
    lv_obj_set_style_border_width(status_bar, 0, 0);
    lv_obj_set_style_pad_all(status_bar, 2, 0);
    lv_obj_set_style_radius(status_bar, 0, 0);
    
    // WiFi icon
    lv_obj_t *lbl_wifi = lv_label_create(status_bar);
    lv_label_set_text(lbl_wifi, LV_SYMBOL_WIFI);
    lv_obj_align(lbl_wifi, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_text_color(lbl_wifi, lv_color_hex(0xFFFFFF), 0);
    
    // BLE icon
    lv_obj_t *lbl_ble = lv_label_create(status_bar);
    lv_label_set_text(lbl_ble, LV_SYMBOL_BLUETOOTH);
    lv_obj_align(lbl_ble, LV_ALIGN_LEFT_MID, 30, 0);
    lv_obj_set_style_text_color(lbl_ble, lv_color_hex(0xFFFFFF), 0);
    
    // Time
    lv_obj_t *time_label = lv_label_create(status_bar);
    lv_label_set_text(time_label, "9:23");
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xFFFFFF), 0);
    
    // Battery
    lv_obj_t *lbl_battery = lv_label_create(status_bar);
    lv_label_set_text(lbl_battery, LV_SYMBOL_BATTERY_FULL);
    lv_obj_align(lbl_battery, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_style_text_color(lbl_battery, lv_color_hex(0xFFFFFF), 0);
}

void create_button_bar(lv_obj_t *parent, const char *left, const char *center, const char *right) {
    lv_obj_t *btn_bar = lv_obj_create(parent);
    lv_obj_set_size(btn_bar, SCREEN_WIDTH, 25);
    lv_obj_align(btn_bar, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(btn_bar, lv_color_hex(0x404040), 0);
    lv_obj_set_style_border_width(btn_bar, 0, 0);
    lv_obj_set_style_radius(btn_bar, 0, 0);
    
    lv_obj_t *lbl_select = lv_label_create(btn_bar);
    lv_label_set_text(lbl_select, left);
    lv_obj_align(lbl_select, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_text_color(lbl_select, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t *lbl_menu = lv_label_create(btn_bar);
    lv_label_set_text(lbl_menu, center);
    lv_obj_align(lbl_menu, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_text_color(lbl_menu, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t *lbl_option = lv_label_create(btn_bar);
    lv_label_set_text(lbl_option, right);
    lv_obj_align(lbl_option, LV_ALIGN_RIGHT_MID, -20, 0);
    lv_obj_set_style_text_color(lbl_option, lv_color_hex(0xFFFFFF), 0);
}

// ====================================================================================
// Home Screen
// ====================================================================================

void home_key_handler(lv_event_t *e) {
    uint32_t key = lv_event_get_key(e);
    
    if (key == LV_KEY_ESC) {
        // Menu button - back to home or show menu
        Serial.println("Menu pressed on home");
    }
}

void app_clicked_cb(lv_event_t *e) {
    uint32_t idx = (uint32_t)(uintptr_t)lv_event_get_user_data(e);
    
    Serial.print("App clicked: ");
    Serial.println(idx);
    
    if (idx == 0) {
        lv_screen_load_anim(screen_file_manager, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
        current_screen = 1;
    } else if (idx == 3) {
        lv_screen_load_anim(screen_notes, LV_SCR_LOAD_ANIM_MOVE_LEFT, 300, 0, false);
        current_screen = 2;
    }
}

void create_home_screen() {
    screen_home = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_home, lv_color_hex(0x000000), 0);
    
    // Add key event handler for home screen
    lv_obj_add_event_cb(screen_home, home_key_handler, LV_EVENT_KEY, NULL);
    
    create_status_bar(screen_home);
    
    // Clock - left side, larger and better positioned
    label_time = lv_label_create(screen_home);
    lv_label_set_text(label_time, "21:35");
    lv_obj_set_style_text_font(label_time, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_pos(label_time, 25, 60);
    
    // Date - right side, aligned with clock vertically
    label_date = lv_label_create(screen_home);
    lv_label_set_text(label_date, "15:05\nTue");
    lv_obj_set_style_text_align(label_date, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label_date, lv_color_hex(0xAAAAAA), 0);
    lv_obj_set_style_text_font(label_date, &lv_font_montserrat_20, 0);
    lv_obj_set_pos(label_date, SCREEN_WIDTH - 75, 70);
    
    // App icons - centered horizontally at bottom, larger size
    const char *app_symbols[] = {LV_SYMBOL_DIRECTORY, LV_SYMBOL_SETTINGS, LV_SYMBOL_LIST, LV_SYMBOL_EDIT};
    const char *app_labels[] = {"File Ma..", "Settings", "Games", "Notes"};
    const uint32_t app_colors[] = {0xFF9900, 0x6699CC, 0x66CC66, 0xFFCC00};
    
    int icon_size = 60;
    int spacing = 75;
    int total_width = (icon_size * 4) + (spacing - icon_size) * 3;
    int start_x = (SCREEN_WIDTH - total_width) / 2;
    int y_pos = SCREEN_HEIGHT - 105;
    
    for (int i = 0; i < 4; i++) {
        int x_pos = start_x + (i * spacing);
        
        // App button/icon - larger
        lv_obj_t *app = lv_button_create(screen_home);
        lv_obj_set_size(app, icon_size, icon_size);
        lv_obj_set_pos(app, x_pos, y_pos);
        lv_obj_set_style_bg_color(app, lv_color_hex(app_colors[i]), 0);
        lv_obj_set_style_radius(app, 10, 0);
        lv_obj_set_style_border_width(app, 0, 0);
        lv_obj_set_style_shadow_width(app, 8, 0);
        lv_obj_set_style_shadow_color(app, lv_color_hex(0x000000), 0);
        lv_obj_set_style_shadow_opa(app, LV_OPA_30, 0);
        
        // Focus style - highlight when selected
        lv_obj_set_style_border_width(app, 3, LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(app, lv_color_hex(0xFFFFFF), LV_STATE_FOCUSED);
        lv_obj_set_style_shadow_width(app, 12, LV_STATE_FOCUSED);
        
        lv_obj_add_event_cb(app, app_clicked_cb, LV_EVENT_CLICKED, (void*)(uintptr_t)i);
        
        // Icon inside button - larger
        lv_obj_t *icon = lv_label_create(app);
        lv_label_set_text(icon, app_symbols[i]);
        lv_obj_center(icon);
        lv_obj_set_style_text_color(icon, lv_color_hex(0x000000), 0);
        lv_obj_set_style_text_font(icon, &lv_font_montserrat_28, 0);
        
        // Label below button - centered under each icon
        lv_obj_t *label = lv_label_create(screen_home);
        lv_label_set_text(label, app_labels[i]);
        lv_obj_set_pos(label, x_pos + (icon_size / 2), y_pos + icon_size + 5);
        lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
        lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
        
        // Center the label under icon
        lv_obj_align_to(label, app, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);
    }
    
    create_button_bar(screen_home, "Select", "Menu", "Option");
}

// ====================================================================================
// File Manager Screen
// ====================================================================================

void file_back_event(lv_event_t *e) {
    uint32_t key = lv_event_get_key(e);
    if (key == LV_KEY_ESC) {
        // Menu/Back button pressed - go back to home
        lv_screen_load_anim(screen_home, LV_SCR_LOAD_ANIM_MOVE_RIGHT, 300, 0, false);
        current_screen = 0;
    }
}

void file_option_cb(lv_event_t *e) {
    lv_obj_t *mbox = (lv_obj_t*)lv_event_get_current_target(e);
    lv_msgbox_close(mbox);
}

void file_item_cb(lv_event_t *e) {
    lv_obj_t *btn = (lv_obj_t*)lv_event_get_target(e);
    const char *txt = lv_list_get_button_text(file_list, btn);
    
    Serial.print("File selected: ");
    Serial.println(txt);
    
    lv_obj_t *mbox = lv_msgbox_create(NULL);
    lv_msgbox_add_title(mbox, "Options");
    lv_msgbox_add_text(mbox, txt);
    
    lv_obj_t *btn_cont = lv_msgbox_add_footer_button(mbox, "Open");
    lv_obj_add_event_cb(btn_cont, file_option_cb, LV_EVENT_CLICKED, NULL);
    
    btn_cont = lv_msgbox_add_footer_button(mbox, "Delete");
    lv_obj_add_event_cb(btn_cont, file_option_cb, LV_EVENT_CLICKED, NULL);
    
    lv_msgbox_add_close_button(mbox);
}

void create_file_manager_screen() {
    screen_file_manager = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_file_manager, lv_color_hex(0x808080), 0);
    
    // Add key handler for back button
    lv_obj_add_event_cb(screen_file_manager, file_back_event, LV_EVENT_KEY, NULL);
    
    create_status_bar(screen_file_manager);
    
    lv_obj_t *title = lv_label_create(screen_file_manager);
    lv_label_set_text(title, "File manager");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 25);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
    file_list = lv_list_create(screen_file_manager);
    lv_obj_set_size(file_list, SCREEN_WIDTH - 20, SCREEN_HEIGHT - 80);
    lv_obj_align(file_list, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_bg_color(file_list, lv_color_hex(0x000000), 0);
    
    lv_obj_t *btn;
    btn = lv_list_add_button(file_list, LV_SYMBOL_DIRECTORY, "sdcard");
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFFCC00), 0);
    lv_obj_add_event_cb(btn, file_item_cb, LV_EVENT_CLICKED, NULL);
    
    btn = lv_list_add_button(file_list, LV_SYMBOL_DIRECTORY, "storage");
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF9900), 0);
    lv_obj_add_event_cb(btn, file_item_cb, LV_EVENT_CLICKED, NULL);
    
    btn = lv_list_add_button(file_list, LV_SYMBOL_DIRECTORY, "system");
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xFF9900), 0);
    lv_obj_add_event_cb(btn, file_item_cb, LV_EVENT_CLICKED, NULL);
    
    create_button_bar(screen_file_manager, "Select", "Menu", "Option");
}

// ====================================================================================
// Notes Screen
// ====================================================================================

void show_keyboard_cb(lv_event_t *e) {
    if (!keyboard_visible) {
        lv_obj_remove_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
        lv_obj_move_foreground(keyboard);
        keyboard_visible = true;
    }
}

void create_notes_screen() {
    screen_notes = lv_obj_create(NULL);
    lv_obj_set_style_bg_color(screen_notes, lv_color_hex(0x808080), 0);
    
    create_status_bar(screen_notes);
    
    lv_obj_t *title = lv_label_create(screen_notes);
    lv_label_set_text(title, "Notes");
    lv_obj_align(title, LV_ALIGN_TOP_LEFT, 10, 25);
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    
    notes_textarea = lv_textarea_create(screen_notes);
    lv_obj_set_size(notes_textarea, SCREEN_WIDTH - 20, 100);
    lv_obj_align(notes_textarea, LV_ALIGN_TOP_MID, 0, 50);
    lv_textarea_set_text(notes_textarea, "Today: Don't forget to take your medicine.");
    lv_obj_add_event_cb(notes_textarea, show_keyboard_cb, LV_EVENT_FOCUSED, NULL);
    
    keyboard = lv_keyboard_create(screen_notes);
    lv_obj_set_size(keyboard, SCREEN_WIDTH - 20, 60);
    lv_obj_align(keyboard, LV_ALIGN_BOTTOM_MID, 0, -30);
    lv_keyboard_set_textarea(keyboard, notes_textarea);
    lv_obj_add_flag(keyboard, LV_OBJ_FLAG_HIDDEN);
    
    create_button_bar(screen_notes, "Select", "Menu", "Clear");
}

// ====================================================================================
// Setup and Main Loop
// ====================================================================================

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("\n\n=========================");
    Serial.println("ESP32-S3 Game Console Starting...");
    Serial.println("=========================");
    
    // Init button pins with detailed logging
    Serial.println("Initializing buttons...");
    pinMode(KEY_UP, INPUT_PULLUP);
    pinMode(KEY_DOWN, INPUT_PULLUP);
    pinMode(KEY_LEFT, INPUT_PULLUP);
    pinMode(KEY_RIGHT, INPUT_PULLUP);
    pinMode(KEY_MENU, INPUT_PULLUP);
    pinMode(KEY_OPTION, INPUT_PULLUP);
    pinMode(KEY_SELECT, INPUT_PULLUP);
    pinMode(KEY_START, INPUT_PULLUP);
    pinMode(KEY_A, INPUT_PULLUP);
    pinMode(KEY_B, INPUT_PULLUP);
    
    // Test buttons
    Serial.println("Testing buttons (press any to test)...");
    delay(500);
    Serial.print("UP: "); Serial.println(digitalRead(KEY_UP));
    Serial.print("DOWN: "); Serial.println(digitalRead(KEY_DOWN));
    Serial.print("LEFT: "); Serial.println(digitalRead(KEY_LEFT));
    Serial.print("RIGHT: "); Serial.println(digitalRead(KEY_RIGHT));
    Serial.print("SELECT: "); Serial.println(digitalRead(KEY_SELECT));
    Serial.print("MENU: "); Serial.println(digitalRead(KEY_MENU));
    
    // Init SPI for display
    Serial.println("Initializing SPI...");
    spi_tft = new SPIClass(HSPI);
    spi_tft->begin(TFT_SCLK, -1, TFT_MOSI, TFT_CS);
    spi_tft->setFrequency(40000000); // 40MHz
    
    // Init display
    Serial.println("Initializing display...");
    tft_init();
    tft_fill_rect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x0000);
    
    Serial.println("Display initialized");
    
    // Init SD card
    Serial.println("Initializing SD card...");
    SPI.begin(SD_CLK, SD_DAT0, SD_CMD, SD_CS);
    if (SD.begin(SD_CS)) {
        Serial.println("SD Card mounted successfully");
    } else {
        Serial.println("SD Card mount failed");
    }
    
    // Init LVGL
    Serial.println("Initializing LVGL...");
    lv_init();
    
    // Create display
    disp = lv_display_create(SCREEN_WIDTH, SCREEN_HEIGHT);
    lv_display_set_flush_cb(disp, my_disp_flush);
    
    // Allocate buffers
    static uint8_t buf1[LVGL_BUFFER_SIZE];
    static uint8_t buf2[LVGL_BUFFER_SIZE];
    lv_display_set_buffers(disp, buf1, buf2, sizeof(buf1), LV_DISPLAY_RENDER_MODE_PARTIAL);
    
    Serial.println("Display configured");
    
    // Create input device
    Serial.println("Creating input device...");
    indev_keypad = lv_indev_create();
    lv_indev_set_type(indev_keypad, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_read_cb(indev_keypad, read_keypad);
    
    Serial.println("Input device created");
    
    // Create UI screens
    Serial.println("Creating UI screens...");
    create_home_screen();
    create_file_manager_screen();
    create_notes_screen();
    
    // Load home screen
    lv_screen_load(screen_home);
    
    // Setup input group for navigation
    input_group = lv_group_create();
    lv_group_set_default(input_group);
    lv_indev_set_group(indev_keypad, input_group);
    
    Serial.println("Input group configured");
    
    Serial.println("\n=========================");
    Serial.println("System ready!");
    Serial.println("=========================");
    Serial.println("Button Controls:");
    Serial.println("UP/DOWN/LEFT/RIGHT - Navigate");
    Serial.println("SELECT/A - Confirm/Enter");
    Serial.println("MENU/B - Back/Cancel");
    Serial.println("OPTION - Clear/Delete");
    Serial.println("START - Home");
    Serial.println("=========================\n");
}

void loop() {
    // Handle LVGL tasks
    lv_timer_handler();
    
    // Debug: Print button states every 2 seconds if pressed
    static uint32_t last_debug = 0;
    if (millis() - last_debug > 2000) {
        bool any_pressed = false;
        if (digitalRead(KEY_UP) == LOW) { Serial.println("DEBUG: UP pressed"); any_pressed = true; }
        if (digitalRead(KEY_DOWN) == LOW) { Serial.println("DEBUG: DOWN pressed"); any_pressed = true; }
        if (digitalRead(KEY_LEFT) == LOW) { Serial.println("DEBUG: LEFT pressed"); any_pressed = true; }
        if (digitalRead(KEY_RIGHT) == LOW) { Serial.println("DEBUG: RIGHT pressed"); any_pressed = true; }
        if (digitalRead(KEY_SELECT) == LOW) { Serial.println("DEBUG: SELECT pressed"); any_pressed = true; }
        if (digitalRead(KEY_MENU) == LOW) { Serial.println("DEBUG: MENU pressed"); any_pressed = true; }
        
        if (any_pressed) {
            last_debug = millis();
        }
    }
    
    delay(5);
}