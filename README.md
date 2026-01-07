# S3-PocKeTOs
ESP32-S3 Pocket Console - Complete Integrated System

## 📌 ESP32-S3 + TFT ST7789 + SD Card

Tài liệu này mô tả **định nghĩa các kết nối phần cứng** cho dự án launcher / home screen (phong cách Symbian S60) sử dụng **ESP32-S3**, màn hình **TFT 2.0 inch 320×240 (ST7789)**, **10 nút bấm** và **SD Card (SDMMC)**.

---

#### 🧠 Tổng quan hệ thống

* Vi điều khiển: **ESP32-S3**
* Màn hình: **TFT LCD 2.0 inch – 320×240 – ST7789 (SPI)**
* Giao diện: **Landscape (320×240)**
* Lưu trữ mở rộng: **SD Card (SDMMC)**
* Điều khiển: **10 nút bấm vật lý**

---

### 🖥️ Kết nối màn hình TFT (ST7789 – SPI)

| Chức năng    | Chân TFT   | GPIO ESP32-S3 | Ghi chú                  |
| ------------ | ---------- | ------------- | ------------------------ |
| Nguồn        | VCC        | VCC           | 3.3V                     |
| Mass         | GND        | GND           | Chung mass               |
| Đèn nền      | LEDK       | GPIO39        | PWM (điều chỉnh độ sáng) |
| Data/Command | D/C        | GPIO47        | Điều khiển SPI           |
| Chip Select  | CS         | GPIO14        | SPI CS                   |
| Clock        | SCL / SCK  | GPIO48        | SPI Clock                |
| Data         | SDA / MOSI | GPIO12        | SPI MOSI                 |
| Reset        | RESET      | GPIO3         | Reset LCD                |

### 🎮 Kết nối các nút bấm (Button Inputs)

Tất cả nút bấm sử dụng **INPUT_PULLUP** (kéo xuống GND khi nhấn).

| Tên nút    | GPIO   | Chức năng           |
| ---------- | ------ | ------------------- |
| KEY_UP     | GPIO7  | Di chuyển lên       |
| KEY_DOWN   | GPIO46 | Di chuyển xuống     |
| KEY_LEFT   | GPIO45 | Trái                |
| KEY_RIGHT  | GPIO6  | Phải                |
| KEY_MENU   | GPIO18 | Mở Menu screen      |
| KEY_OPTION | GPIO8  | Chỉnh sửa / sắp xếp |
| KEY_SELECT | GPIO16 | Tùy chọn            |
| KEY_START  | GPIO17 | Start / OK          |
| KEY_A      | GPIO15 | Phím chức năng A    |
| KEY_B      | GPIO5  | Phím chức năng B    |
`10 nút bấm (KEY_UP , KEY_DOWN , KEY_LEFT , KEY_RIGHT , KEY_MENU , KEY_OPTION , KEY_SELECT , KEY_START , KEY_A , KEY_B).`

#### 📌 Ví dụ khai báo trong code

```c
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
```

---

### 💾 Kết nối SD Card (SDMMC – 1-bit / 4-bit)

SD Card được dùng để lưu **theme**, **icon**, **config** và dữ liệu ứng dụng.

| Chức năng | Chân SD | GPIO ESP32-S3 | Ghi chú     |
| --------- | ------- | ------------- | ----------- |
| Nguồn     | VDD     | VCC           | 3.3V        |
| Mass      | GND     | GND           | Chung mass  |
| DAT3 / CD | CD/DAT3 | GPIO10        | Card detect |
| CMD       | CMD     | GPIO11        | SD Command  |
| Clock     | CLK     | GPIO13        | SD Clock    |
| Data      | DAT0    | GPIO9         | SD Data     |

#### 📌 Khởi tạo SD Card

```cpp
#include "SD_MMC.h"

bool initSD() {
  if (!SD_MMC.begin("/sdcard", true)) { // true = 1-bit mode
    Serial.println("SD Card mount failed");
    return false;
  }
  Serial.println("SD Card mounted");
  return true;
}
```

---

### 📂 Cấu trúc thư mục SD Card đề xuất

```
SD/
├── theme/
│   ├── default.bin
│   ├── dark.bin
│   └── godzilla.bin
├── icons/
├── config/
│   └── system.cfg
└── app/
```

---

#### 🎨 Định dạng file Theme (.bin)

| Thuộc tính   | Giá trị         |
| ------------ | --------------- |
| Độ phân giải | 320 × 240       |
| Màu          | RGB565 (16-bit) |
| Byte order   | Big Endian      |
| Dung lượng   | 153,600 bytes   |
| Header       | Không           |

---

#### ✅ Ghi chú quan trọng

* Không sử dụng RGB888 cho theme
* Không dùng dithering hoặc binary
* Luôn kiểm tra kích thước file `.bin`
* Đọc theme theo **từng dòng** để tiết kiệm RAM

---

#### 🚀 Mở rộng trong tương lai

* Theme động / nhiều lớp
* File browser
* Icon loader riêng
* Lưu cấu hình hệ thống
* Emulator / App framework

---

✍️ *Tài liệu dành cho dự án S3-PocKeTOs – Screen UI*
