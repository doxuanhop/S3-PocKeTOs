#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_ST7789.h>

/* ===== TFT PIN ===== */
#define TFT_MOSI   12
#define TFT_SCLK   48
#define TFT_CS     14
#define TFT_DC     47
#define TFT_RST    3
#define TFT_BL     39

/* ===== BUTTON PIN ===== */
#define KEY_UP     7
#define KEY_DOWN   46
#define KEY_LEFT   45
#define KEY_RIGHT  6
#define KEY_MENU   18
#define KEY_OPTION 8
#define KEY_SELECT 16
#define KEY_START  17
#define KEY_A      15
#define KEY_B      5

/* ===== TFT OBJECT ===== */
Adafruit_ST7789 tft = Adafruit_ST7789(
  TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST
);

/* ===== FUNCTION ===== */
void drawUI();
void showKey(const char *keyName);

/* ===== SETUP ===== */
void setup() {
  Serial.begin(115200);

  /* Backlight */
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  /* Button setup */
  pinMode(KEY_UP,     INPUT_PULLUP);
  pinMode(KEY_DOWN,   INPUT_PULLUP);
  pinMode(KEY_LEFT,   INPUT_PULLUP);
  pinMode(KEY_RIGHT,  INPUT_PULLUP);
  pinMode(KEY_MENU,   INPUT_PULLUP);
  pinMode(KEY_OPTION, INPUT_PULLUP);
  pinMode(KEY_SELECT, INPUT_PULLUP);
  pinMode(KEY_START,  INPUT_PULLUP);
  pinMode(KEY_A,      INPUT_PULLUP);
  pinMode(KEY_B,      INPUT_PULLUP);

  /* TFT init */
  tft.init(240, 320);
  tft.setRotation(1); // Landscape
  tft.fillScreen(ST77XX_BLACK);

  drawUI();
}

/* ===== LOOP ===== */
void loop() {
  if (!digitalRead(KEY_UP))     showKey("UP");
  if (!digitalRead(KEY_DOWN))   showKey("DOWN");
  if (!digitalRead(KEY_LEFT))   showKey("LEFT");
  if (!digitalRead(KEY_RIGHT))  showKey("RIGHT");
  if (!digitalRead(KEY_MENU))   showKey("MENU");
  if (!digitalRead(KEY_OPTION)) showKey("OPTION");
  if (!digitalRead(KEY_SELECT)) showKey("SELECT");
  if (!digitalRead(KEY_START))  showKey("START");
  if (!digitalRead(KEY_A))      showKey("A");
  if (!digitalRead(KEY_B))      showKey("B");

  delay(120);
}

/* ===== DRAW UI ===== */
void drawUI() {
  tft.fillScreen(ST77XX_BLACK);

  tft.setTextColor(ST77XX_CYAN);
  tft.setTextSize(2);
  tft.setCursor(10, 10);
  tft.println("ESP32-S3 GAME PAD");

  tft.drawRect(10, 50, 300, 180, ST77XX_WHITE);

  tft.setCursor(20, 70);
  tft.setTextSize(2);
  tft.println("Press any key...");
}

/* ===== SHOW KEY ===== */
void showKey(const char *keyName) {
  tft.fillRect(20, 120, 280, 40, ST77XX_BLACK);

  tft.setCursor(20, 120);
  tft.setTextColor(ST77XX_YELLOW);
  tft.setTextSize(3);
  tft.print("KEY: ");
  tft.print(keyName);

  Serial.println(keyName);
}
