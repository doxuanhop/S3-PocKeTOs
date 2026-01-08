#include <TFT_eSPI.h>
#include <SPI.h>

TFT_eSPI tft = TFT_eSPI();

// Định nghĩa chân nút bấm
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

const int btnPins[] = {7, 46, 45, 6, 18, 8, 16, 17, 15, 5};
const char* btnLabels[] = {"UP", "DOWN", "LEFT", "RIGHT", "MENU", "OPTION", "SELECT", "START", "A", "B"};

void setup() {
  Serial.begin(115200);

  // 1. Quản lý đèn nền thủ công như Adafruit
  pinMode(39, OUTPUT);
  digitalWrite(39, HIGH); 

  // 2. Hard Reset thủ công (Đây là lý do bạn phải nhấn nút RST)
  // Chúng ta mô phỏng việc nhấn nút RST bằng code
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  delay(50);
  digitalWrite(3, LOW);
  delay(100);
  digitalWrite(3, HIGH);
  delay(150); 

  // 3. Khởi tạo TFT_eSPI sau khi đã Reset phần cứng
  tft.init();
  tft.setRotation(3);
  tft.setSwapBytes(true); // Giúp hiển thị đúng màu ST7789
  tft.fillScreen(TFT_BLACK);

  // Vẽ giao diện ban đầu
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("TFT_eSPI READY", 160, 100, 4);

  // 4. Khởi tạo nút bấm
  for (int i = 0; i < 10; i++) {
    pinMode(btnPins[i], INPUT_PULLUP);
  }
}

void loop() {
  static int lastBtn = -1;
  bool isPressed = false;

  for (int i = 0; i < 10; i++) {
    if (digitalRead(btnPins[i]) == LOW) {
      isPressed = true;
      if (lastBtn != i) {
        showKey(btnLabels[i]);
        lastBtn = i;
      }
    }
  }

  if (!isPressed && lastBtn != -1) {
    tft.fillRect(0, 140, 320, 40, TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.drawCentreString("Waiting for key...", 160, 150, 2);
    lastBtn = -1;
  }
}

void showKey(const char *keyName) {
  tft.fillRect(0, 140, 320, 40, TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString(keyName, 160, 140, 4);
  Serial.printf("Key Pressed: %s\n", keyName);
}