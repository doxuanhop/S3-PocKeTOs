#include <Arduino_GFX_Library.h>
#include <SD_MMC.h>

/* --- Định nghĩa màu sắc (Fix lỗi NOT DECLARED) --- */
#define BLACK   0x0000
#define WHITE   0xFFFF
#define YELLOW  0xFFE0
#define GRAY    0x8410
#define SILVER  0xC618
#define DARKGRAY 0x4208

/* --- Cấu hình Màn hình ST7789 (Fix lỗi Arduino_ESP32LCDPanel) --- */
// Đối với ESP32-S3 sử dụng SPI cho ST7789:
Arduino_DataBus *bus = new Arduino_ESP32SPI(
    47 /* DC */, 14 /* CS */, 48 /* SCK */, 12 /* SDA/MOSI */, -1 /* MISO */
);

Arduino_GFX *gfx = new Arduino_ST7789(
    bus, 3 /* RST */, 3 /* Rotation: Landscape */, true /* IPS */
);

/* --- Định nghĩa chân cho 10 nút bấm --- */
#define NUM_KEYS 10
int keyPins[NUM_KEYS] = {7, 46, 45, 6, 18, 8, 16, 17, 15, 5};
enum {K_UP, K_DOWN, K_LEFT, K_RIGHT, K_MENU, K_OPTION, K_SELECT, K_START, K_A, K_B};

// Biến điều khiển
int activeApp = 0; 
const char* appNames[] = {"File Ma..", "Weather", "Calc", "Notes"};
unsigned long lastPress = 0;

void setup() {
  Serial.begin(115200);
  
  // Khởi tạo màn hình
  if (!gfx->begin()) {
    Serial.println("GFX begin() failed!");
  }
  gfx->fillScreen(BLACK);
  
  // Đèn nền
  pinMode(39, OUTPUT);
  digitalWrite(39, HIGH);

  // Nút bấm
  for(int i=0; i<NUM_KEYS; i++) {
    pinMode(keyPins[i], INPUT_PULLUP);
  }

  drawInterface();
}

void loop() {
  checkButtons();
}

void drawInterface() {
  // 1. Status Bar
  gfx->fillRect(0, 0, 320, 30, DARKGRAY);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(1);
  gfx->setCursor(10, 10); gfx->print("CrokPocket");
  gfx->setCursor(280, 10); gfx->print("100%");

  // 2. Clock Area (Phần màu bạc)
  gfx->fillRect(0, 30, 320, 100, SILVER);
  gfx->drawFastHLine(0, 130, 320, WHITE);
  
  gfx->setTextColor(BLACK); // Chữ đen trên nền bạc
  gfx->setTextSize(5);
  gfx->setCursor(20, 55); gfx->print("21:35"); 
  
  gfx->setTextSize(2);
  gfx->setCursor(210, 55); gfx->print("15-05");
  gfx->setCursor(210, 85); gfx->print("Tue");

  drawAppIcons();

  // 3. Softkeys bar
  gfx->fillRect(0, 210, 320, 30, DARKGRAY);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(2);
  gfx->setCursor(15, 218); gfx->print("Select");
  gfx->setCursor(135, 218); gfx->print("Menu");
  gfx->setCursor(230, 218); gfx->print("Option");
}

void drawAppIcons() {
  int startX = 15;
  int y = 145;
  int size = 60;
  int spacing = 15;

  for (int i = 0; i < 4; i++) {
    int x = startX + i * (size + spacing);
    
    // Xử lý Highlight khi chọn App
    if (i == activeApp) {
      gfx->drawRect(x-3, y-3, size+6, size+6, YELLOW); // Viền vàng dày
      gfx->drawRect(x-2, y-2, size+4, size+4, YELLOW);
      gfx->setTextColor(YELLOW);
    } else {
      gfx->drawRect(x-3, y-3, size+6, size+6, BLACK); // Xóa viền cũ
      gfx->setTextColor(WHITE);
    }

    // Vẽ Icon (Tạm thời là hình khối)
    gfx->fillRect(x, y, size, size, 0x2104); 
    gfx->drawRect(x, y, size, size, WHITE);
    
    gfx->setTextSize(1);
    gfx->setCursor(x, y + size + 8);
    gfx->print(appNames[i]);
  }
}

void checkButtons() {
  if (millis() - lastPress < 150) return;

  // Di chuyển trái phải
  if (digitalRead(keyPins[K_RIGHT]) == LOW) {
    activeApp = (activeApp + 1) % 4;
    drawAppIcons();
    lastPress = millis();
  }
  
  if (digitalRead(keyPins[K_LEFT]) == LOW) {
    activeApp = (activeApp - 1 + 4) % 4;
    drawAppIcons();
    lastPress = millis();
  }

  // Nút Select/A để mở App
  if (digitalRead(keyPins[K_SELECT]) == LOW || digitalRead(keyPins[K_A]) == LOW) {
    Serial.print("Opening: "); Serial.println(appNames[activeApp]);
    lastPress = millis();
  }
}