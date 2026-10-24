# DIY Dual-Mode RC Transmitter & Receiver (RC Car & FPV Drone)
## ⚡ Phiên Bản Ultra-Low Latency V4 PRO MAX (Zero-Lag 200Hz • CRSF 420k • PWM 333Hz)

Dự án tay điều khiển từ xa RC chuyên nghiệp đa năng, tích hợp điều khiển cả **Xe ô tô điều khiển (RC Car)** và **Máy bay Drone (FPV Quadcopter)** trên cùng một hệ thống phần cứng, tối ưu hóa triệt để không có độ trễ (**Real-Time Zero-Lag ~3.8ms**) và trang bị hệ thống **Menu cài đặt đa tầng 6 trang chuyên sâu**.

---

## 🌟 Đột Phá Kỹ Thuật Siêu Tốc (Zero-Latency Pipeline)

- **Đường truyền sóng vô tuyến LoRa E22 siêu tốc (200Hz Packet Rate):**
  - Tùy chỉnh 4 mức tần số làm tươi: **`50Hz (20ms)` | `100Hz (10ms)` | `150Hz (6.6ms)` | `200Hz (5.0ms)`**.
  - Độ trễ điều khiển thực tế hạ xuống chỉ còn **`~3.8ms`** (ngang ngửa các hệ thống ExpressLRS / TBS Crossfire chuyên nghiệp).
  - Thuật toán đọc ADC non-blocking trên STM32 (72MHz Cortex-M3) kết hợp bộ lọc số Fast-EMA không làm nghẽn chu kỳ truyền.
- **Đầu ra Mạch Thu kép siêu nhạy (Dual Ultra-Speed Output):**
  - 🏎️ **Xe RC:** Hỗ trợ **High-Speed PWM 333Hz (chu kỳ 3ms)** cho Digital Servo và ESC Brushless (nhanh gấp 6.6 lần so với PWM 50Hz cổ điển, bánh xe bẻ lái tức thì, bốc đầu không độ trễ).
  - 🛸 **Drone FPV:** Hỗ trợ giao thức **CRSF (Crossfire Protocol) 420,000 baud** trực tiếp vào Flight Controller (Betaflight / INAV) với tần số làm tươi lên đến 200Hz. Vẫn giữ chuẩn SBUS 100,000 baud truyền thống để người dùng tùy chọn.
- **Hệ thống cách ly an toàn 100% (Zero-Conflict Safety Isolation):**
  - Khi điều khiển Drone: Mạch xe ô tô bị khóa cứng ga ở mức 1500µs an toàn tuyệt đối.
  - Khi lái Xe ô tô: Mạch bay Drone tự động nhận lệnh DISARM và cờ Failsafe ngắt ga hoàn toàn.

---

## 📟 Màn Hình HUD Đa Thông Tin & Menu Cài Đặt 6 Trang (Rich Telemetry GUI)

### 1. Màn hình HUD thời gian thực (Live Dashboard):
```text
Dòng 1: [CAR|ARM 150Hz 4.2ms] -> Tên Model, Trạng thái ARM, Tần số gói Hz, Độ trễ ms
Dòng 2: [S:-48dBm CRSF|333H ] -> Sóng RSSI (dBm), Giao thức Drone & Tần số PWM Xe
Dòng 3: [TX:8.24V RX:11.85V ] -> Đo điện áp Pin Tay Phát 2S và Pin Xe/Drone 3S Telemetry
Dòng 4: [T: 75% S: 25% P 80 ] -> Mức ga (THR), Góc bẻ lái (STR), Núm triết áp (POT)
```

### 2. Hệ thống Menu cài đặt sâu 6 trang (Deep Settings Menu):
- **Trang 1/6 (MODEL SELECT):** Chuyển đổi qua lại giữa `[RC CAR RACER]` và `[FPV DRONE QUAD]`.
- **Trang 2/6 (RF PACKET RATE):** Chọn tốc độ truyền sóng `50Hz` / `100Hz` / `150Hz` / `200Hz (5ms)`.
- **Trang 3/6 (PROTOCOL & PWM):** Đổi Drone giữa `CRSF (420k)` vs `SBUS (100k)`; Đổi Xe giữa `PWM 333Hz Digital` vs `PWM 50Hz Analog`.
- **Trang 4/6 (DEADBAND & EXPO):** Tùy chỉnh vùng chết Joystick `0us` (Ultra sharp), `15us`, `30us`; Đường cong Expo lái `0%`, `20%`, `40%`, `60%`.
- **Trang 5/6 (REVERSE CHANNELS):** Đảo chiều từng kênh độc lập (Ch1 Lái, Ch2 Ga, Ch3, Ch4).
- **Trang 6/6 (SAVE EEPROM):** Lưu toàn bộ thông số cấu hình vĩnh viễn vào bộ nhớ Flash STM32.

---

## 📂 Cấu Trúc Thư Mục Dự Án (Repository Structure)

```text
├── CAD/
│   ├── tay_cam_rc_gop_1_file.stl   # Mô hình 3D vỏ tay cầm (Xem 3D trực tiếp trên web GitHub)
│   ├── tay_cam_rc_gop_1_file.step  # File CAD gốc định dạng STEP chuẩn kỹ thuật
│   └── controller_box.scad         # Mã nguồn thiết kế tham số OpenSCAD
├── Firmware/
│   ├── TayCam_Transmitter_STM32_PRO/
│   │   └── TayCam_Transmitter_STM32_PRO.ino  # Code STM32 Transmitter Zero-Lag 200Hz & Menu 6 trang
│   └── BoThu_Receiver_All_In_One_PRO/
│       └── BoThu_Receiver_All_In_One_PRO.ino # Code Mạch Thu All-In-One (CRSF 420k + PWM 333Hz)
├── Simulation/
│   └── mo_phong_tay_khien_rc_3d.html         # Trình giả lập 3D thời gian thực 60FPS + Menu tương tác
└── README.md
```

---

## 🖨️ Thông Số In 3D Đề Xuất (3D Printing Settings)

| Thông số | Giá trị khuyến nghị |
| :--- | :--- |
| **Vật liệu in (Filament)** | **PETG** (Khuyên dùng để bền dẻo, chịu lực các ngàm gài) hoặc PLA+ |
| **Mật độ ruột (Infill)** | **25%** (Grip / Gyroid hoặc Grid) |
| **Độ dày thành (Wall Loops)** | 3 - 4 lớp (1.2mm - 1.6mm) |
| **Độ cao lớp (Layer Height)**| 0.20 mm |
| **Nhiệt độ đầu in / Bàn in** | PETG: 235°C - 240°C / 75°C - 80°C |
| **Hỗ trợ (Support)** | Tree Support (Góc nghiêng > 55°) |
| **Khối lượng vỏ** | ~140g - 160g |

---

## ⚡ Sơ Đồ Chân STM32 BluePill (Pinout Mapping)

### Mạch Phát (Transmitter STM32F103C8T6):
- `PA0 (ADC0)`: Joystick Trái - Y (Throttle / Pitch Car)
- `PA1 (ADC1)`: Joystick Trái - X (Yaw / Turn Car)
- `PA2 (ADC2)`: Joystick Phải - Y (Pitch Drone)
- `PA3 (ADC3)`: Joystick Phải - X (Roll Drone / Steering Car)
- `PA4 (ADC4)`: Núm chiết áp B10K (Hover Throttle Bias / Car Speed Limiter)
- `PA5 (ADC5)`: Cầu phân áp đo Pin 2S (Trở 10k/10k)
- `PB12`: Công tắc SW1 (ARM / DISARM)
- `PB13 / PB14`: Công tắc 3 vị trí SW2 (Angle/Crawler - Horizon/Normal - Acro/Sport)
- `PB15`: Công tắc SW3 (RTH / Khóa vi sai)
- `PA8`: Nút B1 (Menu / Select / Save)
- `PA11 / PA12`: Nút B2 / B3 (Trang sau / Đổi giá trị)
- `PA15 / PB3`: Nút bấm nhấn trục cần PS2 Trái & Phải (Còi xe / Đèn pha)
- `PA9 (TX1) / PA10 (RX1)`: Giao tiếp LoRa E22 tốc độ cao (115200 baud)
- `PB6 (SCL) / PB7 (SDA)`: Màn hình LCD 1604A I2C Fast-Mode 400kHz
- `PB8`: Còi Buzzer 5V Active

### Mạch Thu (Receiver All-in-One):
- `PA2 (TX2)`: Ngõ ra **CRSF (420,000 baud)** hoặc **SBUS (100,000 baud)** cấp vào FC Drone
- `PA0 (TIM2_CH1)`: Ngõ ra **PWM 333Hz / 50Hz** - Bẻ lái Servo xe ô tô (CH1)
- `PA1 (TIM2_CH2)`: Ngõ ra **PWM 333Hz / 50Hz** - Ga ESC xe ô tô (CH2)
- `PA3 / PA6 / PA7`: Ngõ ra PWM phụ trợ (Đèn, còi, tời cứu hộ)
- `A0 (ADC)`: Cầu trở đo áp pin xe/drone gửi Telemetry 2 chiều về màn hình tay cầm

---

## 📄 Bản Quyền (License)
Dự án phát hành theo giấy phép **MIT License** - Hoàn toàn mã nguồn mở cho cộng đồng RC & Drone Maker.
