#include <TFT_eSPI.h>
#include <SPI.h>

/* ===== 1. CẤU HÌNH PHẦN CỨNG ===== */
TFT_eSPI tft = TFT_eSPI();

#define HARDWARE_RST 3    
#define TFT_BL       39   
#define KEY_A        15   
#define KEY_B        5    

#define KEY_UP       7
#define KEY_DOWN     46
#define KEY_LEFT     45
#define KEY_RIGHT    6

/* ===== 2. ĐỊNH NGHĨA TRẠNG THÁI ===== */
enum SystemMode { POWER_OFF, SPLASH, LAUNCHER, POWER_MENU };
SystemMode currentMode = POWER_OFF;

const char *appNames[] = {"ROUTER", "SDCARD", "BLU", "SHELL", "NOTES", "MAPS", "WEBS", "SETTING"};
const char *powerOptions[] = {"Shut down", "Restart", "Cancel"};
int selectedApp = 0;
int selectedPower = 0;

/* ===== 3. HÀM QUẢN LÝ MÀN HÌNH ===== */

void initHardware() {
  pinMode(HARDWARE_RST, OUTPUT);
  digitalWrite(HARDWARE_RST, HIGH);
  delay(50);
  digitalWrite(HARDWARE_RST, LOW);  
  delay(150);
  digitalWrite(HARDWARE_RST, HIGH); 
  delay(200);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);
}

void drawSplashScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_GREEN);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("PoChiTa Os", 160, 100, 4); 

  int bx = 60, by = 150, bw = 200, bh = 14;
  tft.drawRect(bx, by, bw, bh, TFT_WHITE);
  for (int i = 0; i <= 100; i++) {
    int progress = map(i, 0, 100, 0, bw - 4);
    tft.fillRect(bx + 2, by + 2, progress, bh - 4, TFT_GREEN);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawString(String(i) + "%", 160, 185, 2);
    delay(10); 
  }
}

void drawAppIcon(int index, bool selected) {
  int col = index % 4;
  int row = index / 4;
  int x = 12 + col * 76;
  int y = 20 + row * 105;

  // Xóa vùng bao quanh trước khi vẽ lại để tránh lem màu/nháy
  tft.drawRoundRect(x, y, 68, 75, 5, TFT_BLACK);

  if (selected) {
    tft.drawRoundRect(x, y, 68, 75, 5, TFT_YELLOW);
    tft.fillRoundRect(x, y + 80, 68, 20, 4, TFT_YELLOW);
    tft.setTextColor(TFT_BLACK);
  } else {
    tft.drawRoundRect(x, y, 68, 75, 5, 0x39C7);
    tft.fillRoundRect(x, y + 80, 68, 20, 4, 0x2104);
    tft.setTextColor(TFT_WHITE);
  }
  tft.setTextDatum(MC_DATUM);
  tft.drawString(appNames[index], x + 34, y + 90, 2);
  tft.fillRect(x + 14, y + 15, 40, 40, selected ? TFT_SKYBLUE : 0x0155);
}

void drawScrollBar() {
  int barX = 314;
  int thumbHeight = 40;
  tft.fillRect(barX, 20, 4, 200, 0x2104); // Trục
  int thumbY = map(selectedApp, 0, 7, 20, 220 - thumbHeight);
  tft.fillRect(barX, thumbY, 4, thumbHeight, TFT_WHITE); // Con chạy
}

void drawLauncherContent() {
  tft.fillScreen(TFT_BLACK);
  drawScrollBar();
  for (int i = 0; i < 8; i++) {
    drawAppIcon(i, (i == selectedApp));
  }
}

void drawPowerMenu() {
  tft.fillRoundRect(60, 50, 200, 140, 10, 0x18C3); 
  tft.drawRoundRect(60, 50, 200, 140, 10, TFT_WHITE);
  tft.setTextColor(TFT_WHITE);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("POWER MENU", 160, 70, 2);

  for (int i = 0; i < 3; i++) {
    int optY = 95 + i * 30;
    if (i == selectedPower) {
      tft.fillRoundRect(75, optY - 10, 170, 25, 4, TFT_YELLOW);
      tft.setTextColor(TFT_BLACK);
    } else {
      tft.fillRoundRect(75, optY - 10, 170, 25, 4, 0x18C3);
      tft.drawRoundRect(75, optY - 10, 170, 25, 4, TFT_WHITE);
      tft.setTextColor(TFT_WHITE);
    }
    tft.drawString(powerOptions[i], 160, optY + 2, 2);
  }
}

/* ===== 4. SETUP & LOOP ===== */

void setup() {
  Serial.begin(115200);
  initHardware();
  tft.init();
  tft.setRotation(3); // [2026-01-08] Xoay màn hình là 3
  tft.fillScreen(TFT_BLACK);

  pinMode(KEY_A, INPUT_PULLUP);
  pinMode(KEY_B, INPUT_PULLUP);
  pinMode(KEY_UP, INPUT_PULLUP);
  pinMode(KEY_DOWN, INPUT_PULLUP);
  pinMode(KEY_LEFT, INPUT_PULLUP);
  pinMode(KEY_RIGHT, INPUT_PULLUP);

  tft.setTextColor(TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("Hold [A] 10s to Power On", 160, 120, 2);
}

void loop() {
  static unsigned long pressA = 0, pressB = 0;

  // 1. Xử lý nút B - Giữ 10 giây để hiện Menu nguồn
  if (digitalRead(KEY_B) == LOW) {
    if (pressB == 0) pressB = millis();
    if (currentMode == LAUNCHER && (millis() - pressB > 10000)) {
      currentMode = POWER_MENU;
      selectedPower = 0;
      drawPowerMenu();
      pressB = 0;
      while(digitalRead(KEY_B) == LOW); 
    }
  } else { pressB = 0; }

  // 2. Xử lý POWER_OFF - Nhấn giữ nút A 10 giây để mở nguồn
  if (currentMode == POWER_OFF) {
    if (digitalRead(KEY_A) == LOW) {
      if (pressA == 0) pressA = millis();
      if (millis() - pressA > 10000) { // Đổi thành 10 giây (10000ms)
        digitalWrite(TFT_BL, HIGH); // Bật lại đèn nền
        currentMode = SPLASH;
        drawSplashScreen();
        currentMode = LAUNCHER;
        drawLauncherContent();
        pressA = 0;
      }
    } else { pressA = 0; }
  } 
  
  // 3. App Launcher (Điều khiển không nhấp nháy)
  else if (currentMode == LAUNCHER) {
    int oldApp = selectedApp;
    bool moved = false;
    
    if (digitalRead(KEY_RIGHT) == LOW) { if (selectedApp < 7) { selectedApp++; moved = true; } delay(150); }
    else if (digitalRead(KEY_LEFT) == LOW) { if (selectedApp > 0) { selectedApp--; moved = true; } delay(150); }
    else if (digitalRead(KEY_DOWN) == LOW) { if (selectedApp + 4 <= 7) { selectedApp += 4; moved = true; } delay(150); }
    else if (digitalRead(KEY_UP) == LOW) { if (selectedApp - 4 >= 0) { selectedApp -= 4; moved = true; } delay(150); }

    if (moved) {
      drawAppIcon(oldApp, false); 
      drawAppIcon(selectedApp, true);
      drawScrollBar();
    }
  }

  // 4. Power Menu
  else if (currentMode == POWER_MENU) {
    if (digitalRead(KEY_DOWN) == LOW) { selectedPower = (selectedPower + 1) % 3; drawPowerMenu(); delay(200); }
    if (digitalRead(KEY_UP) == LOW) { selectedPower = (selectedPower + 2) % 3; drawPowerMenu(); delay(200); }
    
    if (digitalRead(KEY_A) == LOW) {
      if (selectedPower == 0) { // SHUT DOWN
        tft.fillScreen(TFT_BLACK);
        digitalWrite(TFT_BL, LOW); // Tắt đèn nền
        currentMode = POWER_OFF;
        delay(500);
      } else if (selectedPower == 1) { // RESTART
        ESP.restart();
      } else { // CANCEL
        currentMode = LAUNCHER;
        drawLauncherContent();
      }
      delay(300);
    }
  }

}
