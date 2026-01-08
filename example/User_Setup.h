// Thư mục: /Documents/Arduino/libraries/TFT_eSPI/User_Setup.h

// ###### ĐỊNH NGHĨA DRIVER ######
#define ST7789_DRIVER      // Sử dụng driver cho màn hình ST7789

// ###### CẤU HÌNH KÍCH THƯỚC ######
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ###### CẤU HÌNH CHÂN KẾT NỐI (Sơ đồ bạn cung cấp) ######
#define TFT_BL   39        // LEDK
#define TFT_DC   47        // D/C
#define TFT_CS   14        // CS
#define TFT_SCLK 48        // SCL
#define TFT_MOSI 12        // SDA
#define TFT_RST  3         // RESET
#define TFT_MISO -1        // Không dùng chân MISO

// ###### CẤU HÌNH MÀU SẮC (Tùy chọn cho ST7789) ######
#define TFT_RGB_ORDER TFT_RGB  // Hoặc TFT_BGR nếu màu bị ngược
#define TFT_INVERSION_ON       // Thường màn hình ST7789 cần bật cái này

// ###### CẤU HÌNH FONT CHỮ ######
#define LOAD_GLCD   // Font 1 chuẩn
#define LOAD_FONT2  // Font 2 nhỏ
#define LOAD_FONT4  // Font 4 trung bình
#define LOAD_FONT6  // Font 6 lớn (số)
#define LOAD_FONT7  // Font 7 rất lớn (số đồng hồ)
#define LOAD_FONT8  // Font 8 cực đại (số)
#define LOAD_GFXFF  // Hỗ trợ font FreeFonts
#define SMOOTH_FONT

// ###### TỐI ƯU TỐC ĐỘ 80MHz ######
#define SPI_FREQUENCY       80000000  // Tần số SPI tối đa cho ESP32-S3
#define SPI_READ_FREQUENCY  20000000  // Tần số khi đọc dữ liệu (giữ thấp cho ổn định)
#define SPI_TOUCH_FREQUENCY 2500000   // Tần số cảm ứng (nếu có)

// Sử dụng phần cứng SPI tối ưu của ESP32
#define USE_HSPI_PORT

// Dòng này cực kỳ quan trọng cho ESP32-S3 để sửa lỗi khởi động
#define TFT_SDA_READ 

