#include <Arduino_GFX_Library.h>
#include <SD_MMC.h>

// --- Màu sắc chuẩn Symbian ---
#define SYM_BG_DARK      0x2104 
#define SYM_HEADER_BG    0x9492 
#define SYM_TEXT_WHITE   0xFFFF
#define SYM_TEXT_BLACK   0x0000
#define SYM_FOCUS_YELLOW 0xFFE0 
#define SYM_STATUS_GRAY  0xAD55

// --- Cấu hình phần cứng ---
Arduino_DataBus *bus = new Arduino_ESP32SPI(47, 14, 48, 12, -1);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 3, 3, true);

#define NUM_KEYS 10
const int keyPins[NUM_KEYS] = {7, 46, 45, 6, 18, 8, 16, 17, 15, 5};
enum { K_UP, K_DOWN, K_LEFT, K_RIGHT, K_MENU, K_OPTION, K_SELECT, K_START, K_A, K_B };
unsigned long lastPress = 0;

int activeIdx = 0;
const char* appNames[] = {"File Manager", "Weather", "Calculator", "Notes"};
const char* appIcons[] = {"/icons/filemanager.bmp", "/icons/weather.bmp", "/icons/calculator.bmp", "/icons/notes.bmp"};

// --- 1. SỬA LỖI VẼ STATUS BAR (Dùng hàm cơ bản) ---
void drawStatusIcons() {
  gfx->fillRect(0, 0, 320, 25, SYM_STATUS_GRAY);
  
  // Vẽ biểu tượng WiFi giả lập (Dùng 3 vạch cong vẽ bằng pixel/vòng tròn)
  int wx = 15, wy = 18;
  gfx->fillCircle(wx, wy, 2, SYM_TEXT_BLACK); // Gốc sóng
  gfx->drawCircle(wx, wy, 5, SYM_TEXT_BLACK); // Vòng sóng 1
  gfx->drawCircle(wx, wy, 8, SYM_TEXT_BLACK); // Vòng sóng 2
  // Xóa nửa dưới vòng tròn để tạo hình cánh cung (vẽ đè màu nền)
  gfx->fillRect(wx - 10, wy + 1, 20, 10, SYM_STATUS_GRAY);

  // Bluetooth (Vẽ bằng đường thẳng)
  int bx = 40, by = 6;
  gfx->drawFastVLine(bx+3, by, 12, SYM_TEXT_BLACK);
  gfx->drawLine(bx, by+3, bx+6, by+9, SYM_TEXT_BLACK);
  gfx->drawLine(bx+6, by+3, bx, by+9, SYM_TEXT_BLACK);
  gfx->drawLine(bx+3, by, bx+6, by+3, SYM_TEXT_BLACK);
  gfx->drawLine(bx+3, by+12, bx+6, by+9, SYM_TEXT_BLACK);

  // Đồng hồ nhỏ ở giữa
  gfx->setTextColor(SYM_TEXT_BLACK);
  gfx->setTextSize(1);
  gfx->setCursor(145, 8); gfx->print("10:00");

  // Pin Lithium (Đồ họa chuẩn)
  int px = 280, py = 8;
  gfx->drawRect(px, py, 22, 11, SYM_TEXT_BLACK); // Khung pin
  gfx->fillRect(px + 22, py + 3, 2, 5, SYM_TEXT_BLACK); // Cực dương
  gfx->fillRect(px + 2, py + 2, 14, 7, 0x03E0); // Màu xanh lá (70%)
}

// --- 2. Hàm vẽ BMP (Cơ bản) ---
void drawBmp(const char *filename, int16_t x, int16_t y) {
  File file = SD_MMC.open(filename);
  if (!file) {
    gfx->fillRoundRect(x, y, 60, 60, 8, SYM_HEADER_BG);
    return;
  }

  uint8_t header[54];
  file.read(header, 54);
  uint32_t offset = *(uint32_t *)&header[10];
  file.seek(offset);

  // Đọc theo dòng để tăng tốc
  int w = 60, h = 60;
  uint8_t line[w * 3];
  for (int i = h - 1; i >= 0; i--) {
    file.read(line, w * 3);
    for (int j = 0; j < w; j++) {
      uint8_t b = line[j * 3];
      uint8_t g = line[j * 3 + 1];
      uint8_t r = line[j * 3 + 2];
      gfx->drawPixel(x + j, y + i, gfx->color565(r, g, b));
    }
  }
  file.close();
}

// --- 3. Căn chỉnh Đồng hồ Header ---
void drawCenteredClock() {
  gfx->fillRoundRect(5, 30, 310, 85, 12, SYM_HEADER_BG);
  gfx->drawRoundRect(5, 30, 310, 85, 12, SYM_TEXT_WHITE);
  gfx->drawFastVLine(160, 40, 65, 0xCE79); // Vách ngăn

  gfx->setTextColor(SYM_TEXT_WHITE);
  gfx->setTextSize(5); gfx->setCursor(20, 55); gfx->print("21:35");
  gfx->setTextSize(3); gfx->setCursor(180, 48); gfx->print("15-05");
  gfx->setTextSize(2); gfx->setCursor(210, 85); gfx->print("Tue");
}

// --- 4. Render Main Screen ---
void renderHome() {
  // Nền Theme Gradient
  for(int i=25; i<215; i++) gfx->drawFastHLine(0, i, 320, gfx->color565(i/10, i/10, i/8));
  
  drawStatusIcons();
  drawCenteredClock();

  for (int i = 0; i < 4; i++) {
    int x = 18 + (i * 76);
    int y = 135;
    
    if (activeIdx == i) {
      gfx->drawRoundRect(x-3, y-3, 66, 66, 8, SYM_FOCUS_YELLOW);
      gfx->drawRoundRect(x-4, y-4, 68, 68, 8, SYM_FOCUS_YELLOW);
      
      gfx->setTextColor(SYM_TEXT_WHITE);
      gfx->setCursor(x, y + 68);
      String name = appNames[i];
      if(name.length() > 7) name = name.substring(0, 5) + "..";
      gfx->print(name);
    }
    drawBmp(appIcons[i], x, y);
    gfx->drawRoundRect(x, y, 60, 60, 8, SYM_TEXT_WHITE); // Bo viền icon
  }

  // Softkeys
  gfx->fillRect(0, 215, 320, 25, SYM_BG_DARK);
  gfx->drawFastHLine(0, 215, 320, 0x7BEF);
  gfx->setTextColor(SYM_TEXT_WHITE);
  gfx->setCursor(15, 222); gfx->print("Select");
  gfx->setCursor(135, 222); gfx->print("Menu");
  gfx->setCursor(240, 222); gfx->print("Option");
}

void setup() {
  Serial.begin(115200);
  gfx->begin();
  pinMode(39, OUTPUT); digitalWrite(39, HIGH); 
  for (int i = 0; i < NUM_KEYS; i++) pinMode(keyPins[i], INPUT_PULLUP);

  gfx->fillScreen(SYM_TEXT_BLACK);
  gfx->setCursor(60, 110); gfx->setTextColor(SYM_TEXT_WHITE);
  gfx->print("Booting Symbian...");
  
  SD_MMC.setPins(13, 11, 9);
  if (!SD_MMC.begin("/sdcard", true)) {
    Serial.println("SD card error");
  }
  delay(1000);
  renderHome();
}

void loop() {
  for (int i = 0; i < NUM_KEYS; i++) {
    if (digitalRead(keyPins[i]) == LOW && (millis() - lastPress > 200)) {
      lastPress = millis();
      if (i == K_LEFT)  { activeIdx = (activeIdx - 1 + 4) % 4; renderHome(); }
      if (i == K_RIGHT) { activeIdx = (activeIdx + 1) % 4; renderHome(); }
    }
  }
}