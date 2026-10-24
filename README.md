# DIY Dual-Mode RC Transmitter & Receiver (RC Car & FPV Drone)

Dự án tay điều khiển từ xa RC 2.4GHz / Sub-GHz (LoRa E22) đa năng, tích hợp điều khiển cả **Xe ô tô điều khiển (RC Car)** và **Máy bay Drone (FPV Quadcopter)** trên cùng một phần cứng và mạch thu duy nhất với cơ chế chống xung đột tín hiệu an toàn 100%.

---

## 🌟 Tính Năng Nổi Bật (Key Features)

- **Vỏ hộp công thái học in 3D (3D Printed Ergonomic Shell):**
  - Tích hợp sẵn ngàm gài không ốc (Snap-fit Cradles) cho pin Li-ion 18650, STM32 BluePill, sạc TP4056 Type-C, và còi Buzzer.
  - Hỗ trợ gờ che chống chạm mạch, gờ tỳ công tắc chống trôi nút nhấn.
  - Xem mô hình 3D trực tiếp trên GitHub bằng cách nhấn vào file: [`CAD/tay_cam_rc_gop_1_file.stl`](CAD/tay_cam_rc_gop_1_file.stl).
- **Hệ thống điều khiển chống xung đột (Zero-Conflict Safety Isolation):**
  - **Khi ở chế độ Drone:** Cổng PWM xe ô tô bị cô lập hoàn toàn (khóa chặt ga ESC ở mức 1500µs an toàn, bánh lái giữ thẳng).
  - **Khi ở chế độ Car:** Tín hiệu SBUS Drone bị ngắt lập tức sang Failsafe/Disarm (ngăn chặn quay cánh quạt ngoài ý muốn).
- **Xử lý Joystick PS2 (KY-023) chuyên sâu:**
  - Thuật toán hiệu chuẩn 3 điểm (Min 1150 -> Tâm 2048 -> Max 2950) chuyển đổi mượt mà sang dải 1000µs - 2000µs.
  - Lọc nhiễu trung bình trượt EMA ($\alpha = 0.25$) và Deadband khử rung tâm cần $\pm 25\mu s$.
- **Hỗ trợ cần ga tự về tâm (Altitude Hold & Speed Limiter):**
  - **Drone:** Núm xoay Potentiometer đóng vai trò Hover Throttle Bias (mặc định 35% ~ 1355µs), cho phép thả tay Joystick về tâm mà Drone vẫn giữ độ cao ổn định như flycam hiện đại.
  - **Car:** Núm xoay Potentiometer đóng vai trò Speed Limiter (giới hạn tốc độ tối đa từ 20% đến 100%).
- **Giao thức truyền LoRa E22 siêu xa:**
  - Tốc độ truyền UART 115200 baud, chu kỳ gói tin 20ms (50Hz refresh rate).
  - Kiểm tra toàn vẹn gói tin bằng mã hóa **CRC16-CCITT**.

---

## 📂 Cấu Trúc Dự Án (Repository Structure)

```text
├── CAD/
│   ├── tay_cam_rc_gop_1_file.stl   # Mô hình 3D hoàn chỉnh (Xem 3D trực tiếp trên web GitHub)
│   ├── tay_cam_rc_gop_1_file.step  # File CAD gốc định dạng STEP chuẩn công nghiệp
│   └── controller_box.scad         # Mã nguồn thiết kế tham số OpenSCAD
├── Firmware/
│   ├── TayCam_Transmitter_STM32_PRO/
│   │   └── TayCam_Transmitter_STM32_PRO.ino  # Code mạch phát STM32 BluePill
│   └── BoThu_Receiver_All_In_One_PRO/
│       └── BoThu_Receiver_All_In_One_PRO.ino # Code mạch thu All-in-One (SBUS + PWM)
├── Simulation/
│   └── mo_phong_tay_khien_rc_3d.html         # Trình giả lập 3D tương tác thời gian thực (60 FPS)
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

### Mạch Phát (Transmitter):
- `PA0 (ADC0)`: Joystick Trái - Y (Throttle / Pitch Car)
- `PA1 (ADC1)`: Joystick Trái - X (Yaw / Turn Car)
- `PA2 (ADC2)`: Joystick Phải - Y (Pitch Drone)
- `PA3 (ADC3)`: Joystick Phải - X (Roll Drone / Steering Car)
- `PA4 (ADC4)`: Núm chiết áp B10K (Hover Throttle Bias / Car Speed Limiter)
- `PB12`: Công tắc SW1 (ARM / DISARM)
- `PB13`: Công tắc SW2 (CHẾ ĐỘ: RC CAR / DRONE)
- `PB14`: Nút nhấn SW3 (AUX1 / Còi / Đèn)
- `PB15`: Nút nhấn SW4 (AUX2 / Chế độ bay)
- `PA9 (TX1) / PA10 (RX1)`: Giao tiếp LoRa E22 (115200 baud)
- `PB6 (SCL) / PB7 (SDA)`: Màn hình I2C OLED 0.96" hoặc LCD 1604A
- `PB0`: Còi Buzzer 5V Active

### Mạch Thu (Receiver All-in-One):
- `PA2 (TX2)`: Ngõ ra SBUS 100,000 baud inverted (cấp vào FC Drone)
- `PA0 (TIM2_CH1)`: Ngõ ra PWM Ch1 - Bẻ lái Servo xe ô tô (50Hz)
- `PA1 (TIM2_CH2)`: Ngõ ra PWM Ch2 - Ga ESC xe ô tô (50Hz)
- `PA3 / PA6 / PA7`: Ngõ ra PWM phụ trợ (Đèn, còi, tời cứu hộ)

---

## 🚀 Hướng Dẫn Cài Đặt & Nạp Code (Getting Started)

1. **Cài đặt môi trường:**
   - Cài đặt [Arduino IDE](https://www.arduino.cc/en/software).
   - Thêm bo mạch STM32 qua Board Manager: `STM32 Cores by STMicroelectronics`.
2. **Nạp Firmware Mạch Phát:**
   - Mở file `Firmware/TayCam_Transmitter_STM32_PRO/TayCam_Transmitter_STM32_PRO.ino`.
   - Chọn Board: `Generic STM32F1 series` -> Chip `STM32F103C8`.
   - Kết nối mạch qua mạch nạp ST-Link V2 và nhấn **Upload**.
3. **Nạp Firmware Mạch Thu:**
   - Mở file `Firmware/BoThu_Receiver_All_In_One_PRO/BoThu_Receiver_All_In_One_PRO.ino` và nạp tương tự.

---

## 📄 Bản Quyền (License)
Dự án được phát hành theo giấy phép **MIT License** - Hoàn toàn mã nguồn mở cho cộng đồng DIY Maker.
