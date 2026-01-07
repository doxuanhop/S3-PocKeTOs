# 🚀 S3-PocKeTOs

**S3-PocKeTOs** là một **hệ điều hành mini (system UI / launcher)** dành cho các thiết bị DIY sử dụng **ESP32-S3**, lấy cảm hứng từ **Symbian S60**, các máy handheld cổ điển và tinh thần **vọc – học – đam mê**.

> ⚠️ **Lưu ý quan trọng:**
> Dự án **đang trong quá trình phát triển (Work In Progress)**.
> Code được xây dựng **có sự hỗ trợ của A.I**.
> Tác giả **không phải là người có nền tảng ESP32 chuyên sâu**, mọi thứ được tạo ra từ **đam mê, thời gian, công sức và chi phí cá nhân**.

---

## 🎯 Mục tiêu của dự án

* Tạo ra một **giao diện hệ thống hoàn chỉnh** cho thiết bị cầm tay DIY
* Học hỏi về **ESP32-S3, đồ họa, hệ thống file, UI/UX nhúng**
* Cho phép **tùy biến cao** (theme, menu, icon)
* Không hướng tới thương mại – **chỉ vì đam mê** ❤️

---

## 🧠 Tổng quan hệ thống

| Thành phần      | Thông tin                       |
| --------------- | ------------------------------- |
| Tên hệ thống    | **S3-PocKeTOs**                 |
| Nền tảng        | ESP32-S3                        |
| Ngôn ngữ        | C / C++ (Arduino Framework)     |
| Thư viện đồ họa | Arduino_GFX (moononournation)   |
| Màn hình        | TFT LCD 2.0" – 320×240 (ST7789) |
| Hướng hiển thị  | Landscape                       |
| Lưu trữ         | SD Card (SDMMC)                 |
| Điều khiển      | 10 nút bấm vật lý               |

---

## 🖥️ Giao diện người dùng (UI)

### 🏠 Home Screen

* Đồng hồ thời gian thực (HH:MM)
* Ngày / thứ
* Thanh trạng thái (pin, tín hiệu – demo)
* Menu icon dạng hàng ngang
* Footer hiển thị chức năng nút (Select / Menu / Option)

### 📂 Menu Screen

* Danh sách ứng dụng
* Điều hướng bằng nút vật lý
* Hiển thị tên ứng dụng khi chọn

### 🎨 Theme System

* Theme dạng **file `.bin` RGB565**
* Lưu trên **SD Card**
* Hỗ trợ:

  * Quét tự động `/theme/*.bin`
  * Preview theme
  * Hiệu ứng chuyển theme (fade)
  * Đổi theme **không cần nạp lại firmware**

---

## 🎮 Hệ thống điều khiển

Thiết bị sử dụng **10 nút bấm vật lý**, lấy cảm hứng từ handheld console:

* Điều hướng: UP / DOWN / LEFT / RIGHT
* Hệ thống: MENU / OPTION / SELECT
* Hành động: START / A / B

➡️ Toàn bộ UI được thiết kế **không cần cảm ứng**.

---

## 💾 SD Card – Trái tim của hệ thống

SD Card đóng vai trò **bộ nhớ mở rộng**:

* Lưu theme nền
* Lưu icon
* Lưu cấu hình hệ thống
* Lưu dữ liệu ứng dụng

### 📂 Cấu trúc thư mục đề xuất

```
SD/
├── theme/      # Theme nền (.bin)
├── icons/      # Icon ứng dụng
├── config/     # Cấu hình hệ thống
└── app/        # Dữ liệu ứng dụng
```

---

## 🎨 Định dạng Theme (.bin)

| Thuộc tính   | Giá trị         |
| ------------ | --------------- |
| Độ phân giải | 320 × 240       |
| Màu          | RGB565 (16-bit) |
| Byte order   | Big Endian      |
| Dung lượng   | 153,600 bytes   |
| Header       | Không           |

---

## 📦 Các ứng dụng dự kiến

* 📁 Files (trình duyệt SD Card)
* 🌦 Weather (demo)
* 🧮 Calculator
* 📝 Notes
* ⚙️ Settings
* 🎨 Theme Manager

*(Một số ứng dụng hiện đang ở mức giao diện / demo)*

---

## ⚠️ Trạng thái phát triển

* 🚧 Dự án **chưa hoàn thiện**
* 🚧 Code có thể chưa tối ưu
* 🚧 Có thể thay đổi cấu trúc bất kỳ lúc nào

➡️ Đây là **DIY project**, không phải firmware thương mại.

---

## 🤖 Về A.I trong dự án

* A.I được sử dụng để:

  * Hỗ trợ viết code
  * Gợi ý cấu trúc hệ thống
  * Giải thích lỗi và cách sửa

* Quyết định cuối cùng, thử nghiệm, đấu nối và chịu trách nhiệm:
  **Người làm dự án**.

---

## ❤️ Tuyên bố cá nhân

> Tôi **không phải kỹ sư chuyên nghiệp**, không có nền tảng ESP32 bài bản.
> Tôi chỉ là một người **thích DIY**, thích mày mò, sẵn sàng bỏ **thời gian – công sức – tiền bạc**
> để tạo ra thứ mình thích.
>
> **S3-PocKeTOs tồn tại vì đam mê.**

---

## 🌐 Mạng xã hội & chia sẻ dự án

📺 **YouTube – Innovaboard**
[https://youtube.com/@innovaboard?si=hZb2OKZxTwQiSdB5](https://youtube.com/@innovaboard?si=hZb2OKZxTwQiSdB5)

> Nơi chia sẻ quá trình DIY, thử nghiệm phần cứng, UI hệ thống, và hành trình học hỏi với ESP32-S3.

---

## 📜 Giấy phép

Dự án mang tính **học tập – cá nhân – DIY**.

Bạn có thể:

* Tham khảo
* Fork
* Tùy biến cho mục đích cá nhân

⚠️ Không khuyến khích dùng cho mục đích thương mại.

---

✨ *S3-PocKeTOs – A tiny OS born from passion, curiosity, and persistence.*
