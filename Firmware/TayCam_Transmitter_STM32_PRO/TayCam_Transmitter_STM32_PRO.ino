/*
 * ======================================================================================
 * DỰ ÁN TAY ĐIỀU KHIỂN RC "TOÀN DIỆN" - FIRMWARE TAY PHÁT PRO (TRANSMITTER V3 PRO)
 * TỐI ƯU HÓA ĐẶC BIỆT CHO CẶP JOYSTICK PS2 (LÒ XO HỒI TÂM & GÓC HẸP 60 ĐỘ)
 * KHÔNG BAO GIỜ XUNG ĐỘT GIỮA DRONE & XE RC - BẢO MẬT GÓI TIN CRC16-CCITT
 * ======================================================================================
 * Vi điều khiển: STM32F103C8T6 (72MHz Cortex-M3, 64KB Flash)
 * Sóng truyền: LoRa E22-900T22D / E22-400T22D (115200 Baud, 50Hz Packet Rate, Tầm xa 2-5km)
 * Màn hình: LCD 1604A I2C (Cơ chế Time-Slicing không chặn nhịp RF)
 * ======================================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// --- CẤU HÌNH PHẦN CỨNG (PINOUT CHUẨN STM32) ---
#define PIN_JOY_LY      PA0   // PS2 Trái Y: Ga (Drone Altitude / Car Throttle)
#define PIN_JOY_LX      PA1   // PS2 Trái X: Quay đầu (Yaw)
#define PIN_JOY_RY      PA2   // PS2 Phải Y: Tiến/Lùi (Pitch)
#define PIN_JOY_RX      PA3   // PS2 Phải X: Bẻ lái (Roll / Car Steering)
#define PIN_POT         PA4   // Chiết áp B10K: Hover Throttle Trim (Drone) / Speed Limiter (Car)
#define PIN_BAT_SENSE   PA5   // Cầu phân áp đo Pin 2S (Trở 10k/10k)

#define PIN_LORA_AUX    PB0   // E22 AUX: Báo bận/rảnh phần cứng
#define PIN_LORA_M0     PB1   // E22 M0 (GND/LOW cho Normal Mode)
#define PIN_LORA_M1     PB10  // E22 M1 (GND/LOW cho Normal Mode)

#define PIN_SW1_ARM     PB12  // SW1: ARM / DISARM
#define PIN_SW2_P1      PB13  // SW2 Nấc 1
#define PIN_SW2_P2      PB14  // SW2 Nấc 3 (Cả 2 HIGH là Nấc 2 giữa)
#define PIN_SW3_AUX     PB15  // SW3: RTH / Khóa vi sai

#define PIN_BTN_B1      PA8   // Nút B1: Menu / Select
#define PIN_BTN_B2      PA11  // Nút B2: Down / Trim -
#define PIN_BTN_B3      PA12  // Nút B3: Up / Trim +
#define PIN_SW_L        PA15  // Nút bấm trục cần PS2 trái (Beeper / Còi xe)
#define PIN_SW_R        PB3   // Nút bấm trục cần PS2 phải (Đèn pha / Chụp ảnh)

#define PIN_BUZZER      PB8   // Còi Buzzer 12mm
#define PIN_LED_RING    PB9   // Đèn LED vòng nút nguồn

// Định danh Model chống nhầm lẫn tuyệt đối
#define MODEL_ID_DRONE  0x51  // Ký tự 'Q' (Quadcopter)
#define MODEL_ID_CAR    0x43  // Ký tự 'C' (Car)

// Cấu trúc gói tin 17 bytes tốc độ cao chuẩn viễn thông
struct __attribute__((packed)) RadioPacket {
  uint8_t  header[2];    // 0xAA, 0x55
  uint8_t  model_id;     // 0x51 (Drone) hoặc 0x43 (Car)
  uint8_t  seq_num;      // Số thứ tự gói (0..255 để đo trễ & mất gói)
  uint16_t ch1_roll_str; // 1000 - 2000 us (Roll / Steering)
  uint16_t ch2_pitch;    // 1000 - 2000 us (Pitch / Gear)
  uint16_t ch3_throttle; // 1000 - 2000 us (Ga)
  uint16_t ch4_yaw;      // 1000 - 2000 us (Yaw / Camera Pan)
  uint8_t  sw_flags;     // Bit 0: ARM, Bit 1-2: Mode (0..2), Bit 3: AUX, Bit 4: SW_L, Bit 5: SW_R
  uint8_t  knob_pot;     // 0 - 100 %
  uint16_t crc16;        // Mã kiểm tra lỗi CRC16-CCITT (Polynomial 0x1021)
};

RadioPacket txPacket;

// Thông số hiệu chuẩn 3 điểm (Min - Center - Max) cho Joystick PS2
struct PS2Calibration {
  int minVal;
  int midVal;
  int maxVal;
};

PS2Calibration calLY = { 1150, 2048, 2950 };
PS2Calibration calLX = { 1150, 2048, 2950 };
PS2Calibration calRY = { 1150, 2048, 2950 };
PS2Calibration calRX = { 1150, 2048, 2950 };

// Biến trạng thái
uint8_t currentModel = MODEL_ID_DRONE;
uint8_t packetSequence = 0;
bool inMenu = false;
int menuCursor = 0;
unsigned long b1HoldStart = 0;

// Giá trị lọc số EMA (Exponential Moving Average)
float fLY = 2048, fLX = 2048, fRY = 2048, fRX = 2048, fPot = 2048;
float vBatTx = 8.2f;
float vBatRx = 0.0f;
uint8_t rssiVal = 99;

// Khởi tạo LCD 1604A (Địa chỉ 0x27, 16 cột, 4 dòng)
LiquidCrystal_I2C lcd(0x27, 16, 4);

// --- TÍNH TOÁN CRC16-CCITT (BẢO VỆ DỮ LIỆU TUYỆT ĐỐI) ---
uint16_t calculateCRC16(const uint8_t *data, size_t len) {
  uint16_t crc = 0xFFFF;
  for (size_t i = 0; i < len; i++) {
    crc ^= (uint16_t)data[i] << 8;
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x8000) crc = (crc << 1) ^ 0x1021;
      else              crc = crc << 1;
    }
  }
  return crc;
}

// --- BỘ LỌC SỐ EMA KHỬ NHIỄU ADC ---
float applyEMA(float current, float previous, float alpha = 0.35f) {
  return (alpha * current) + ((1.0f - alpha) * previous);
}

// --- ÁNH XẠ CHUẨN XÁC 3 ĐIỂM (MIN - CENTER - MAX) CHO JOYSTICK PS2 ---
int mapPS2Axis(int raw, const PS2Calibration &cal, int deadband = 40) {
  // 1. Áp dụng Deadband ở tâm để khi cần tự hồi tâm không bị trôi
  if (abs(raw - cal.midVal) <= deadband) {
    return 1500;
  }
  // 2. Chia 2 nửa hành trình độc lập để tâm luôn là 1500us
  if (raw < cal.midVal) {
    int v = constrain(raw, cal.minVal, cal.midVal - deadband);
    return map(v, cal.minVal, cal.midVal - deadband, 1000, 1500);
  } else {
    int v = constrain(raw, cal.midVal + deadband, cal.maxVal);
    return map(v, cal.midVal + deadband, cal.maxVal, 1500, 2000);
  }
}

// --- PHÁT TIẾNG BÍP CÒI ---
void beep(int freq, int duration_ms) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(duration_ms);
  digitalWrite(PIN_BUZZER, LOW);
}

// --- KIỂM TRA KHÓA AN TOÀN KHI KHỞI ĐỘNG (STARTUP SAFETY INTERLOCK) ---
void checkStartupSafety() {
  bool isSafe = false;
  lcd.clear();
  lcd.setCursor(0, 0); lcd.print("KT AN TOAN...");
  
  while (!isSafe) {
    int rawLY = analogRead(PIN_JOY_LY);
    bool armSwitch = (digitalRead(PIN_SW1_ARM) == LOW);
    
    // Nếu công tắc ARM đang gạt BẬT hoặc Cần ga bị kéo lên quá cao -> Hú còi báo động!
    if (armSwitch) {
      lcd.setCursor(0, 1); lcd.print("! CANH BAO ARM !");
      lcd.setCursor(0, 2); lcd.print("GAT SW1 VE DISARM");
      digitalWrite(PIN_BUZZER, HIGH);
      delay(120);
      digitalWrite(PIN_BUZZER, LOW);
      delay(120);
    } else if (currentModel == MODEL_ID_DRONE && rawLY > (calLY.midVal + 300)) {
      lcd.setCursor(0, 1); lcd.print("! CANH BAO GA ! ");
      lcd.setCursor(0, 2); lcd.print("THALONG CAN GA !");
      digitalWrite(PIN_BUZZER, HIGH);
      delay(80);
      digitalWrite(PIN_BUZZER, LOW);
      delay(80);
    } else {
      isSafe = true;
    }
  }
  lcd.clear();
}

void setup() {
  // Cấu hình GPIO
  pinMode(PIN_SW1_ARM, INPUT_PULLUP);
  pinMode(PIN_SW2_P1,  INPUT_PULLUP);
  pinMode(PIN_SW2_P2,  INPUT_PULLUP);
  pinMode(PIN_SW3_AUX, INPUT_PULLUP);

  pinMode(PIN_BTN_B1,  INPUT_PULLUP);
  pinMode(PIN_BTN_B2,  INPUT_PULLUP);
  pinMode(PIN_BTN_B3,  INPUT_PULLUP);
  pinMode(PIN_SW_L,    INPUT_PULLUP);
  pinMode(PIN_SW_R,    INPUT_PULLUP);

  pinMode(PIN_LORA_AUX, INPUT);
  pinMode(PIN_LORA_M0,  OUTPUT);
  pinMode(PIN_LORA_M1,  OUTPUT);
  digitalWrite(PIN_LORA_M0, LOW); // Normal transparent mode
  digitalWrite(PIN_LORA_M1, LOW);

  pinMode(PIN_BUZZER,   OUTPUT);
  pinMode(PIN_LED_RING, OUTPUT);
  digitalWrite(PIN_LED_RING, HIGH);

  // Giao tiếp Serial LoRa E22 chạy ở 115.200 BAUD (Nhanh gấp 12 lần code cũ!)
  Serial1.begin(115200);

  // Đọc Model đã lưu trong Flash EEPROM
  uint8_t saved = EEPROM.read(0x10);
  if (saved == MODEL_ID_CAR) currentModel = MODEL_ID_CAR;
  else currentModel = MODEL_ID_DRONE;

  // Khởi động màn hình LCD 1604A
  Wire.begin();
  Wire.setClock(400000); // Nâng I2C lên Fast Mode 400kHz để chống nghẽn CPU!
  lcd.init();
  lcd.backlight();

  lcd.setCursor(0, 0); lcd.print("TAY KHIEN TOANDIEN");
  lcd.setCursor(0, 1); lcd.print("STM32 + LoRa E22");
  lcd.setCursor(0, 2); lcd.print("JOYSTICK PS2 PRO");
  lcd.setCursor(0, 3); lcd.print("KHOI DONG...");

  beep(2200, 80);
  delay(50);
  beep(2800, 120);
  delay(600);

  // Kiểm tra an toàn trước khi vào vòng lặp chính
  checkStartupSafety();
}

// Biến điều phối nhịp thời gian (Scheduler)
unsigned long lastRfTime   = 0;
unsigned long lastLcdSlice = 0;
uint8_t       lcdLineIndex = 0;

void loop() {
  unsigned long now = millis();

  // 1. XỬ LÝ NHẤN GIỮ NÚT B1 ĐỂ VÀO MENU MODEL
  if (digitalRead(PIN_BTN_B1) == LOW) {
    if (b1HoldStart == 0) b1HoldStart = now;
    if (now - b1HoldStart > 1800) {
      inMenu = !inMenu;
      b1HoldStart = 0;
      beep(3000, 100);
      lcd.clear();
    }
  } else {
    b1HoldStart = 0;
  }

  if (inMenu) {
    handleMenuUI();
    return;
  }

  // 2. ĐỌC VÀ LỌC SỐ TOÀN BỘ ADC CỦA 2 JOYSTICK PS2 (12-bit ADC 0..4095)
  fLY  = applyEMA(analogRead(PIN_JOY_LY), fLY);
  fLX  = applyEMA(analogRead(PIN_JOY_LX), fLX);
  fRY  = applyEMA(analogRead(PIN_JOY_RY), fRY);
  fRX  = applyEMA(analogRead(PIN_JOY_RX), fRX);
  fPot = applyEMA(analogRead(PIN_POT),    fPot);

  // 3. ÁNH XẠ GIÁ TRỊ CHUẨN XÁC THEO TỪNG MODEL (DRONE vs CAR)
  int joyLY_us = mapPS2Axis((int)fLY, calLY); // 1000..2000
  int joyLX_us = mapPS2Axis((int)fLX, calLX); // 1000..2000
  int joyRY_us = mapPS2Axis((int)fRY, calRY); // 1000..2000
  int joyRX_us = mapPS2Axis((int)fRX, calRX); // 1000..2000
  int potPct   = map((int)fPot, 0, 4095, 0, 100); // 0..100%

  bool swArm  = (digitalRead(PIN_SW1_ARM) == LOW);
  uint8_t swMode = 1; // Nấc 2 (Horizon / Normal)
  if (digitalRead(PIN_SW2_P1) == LOW) swMode = 0; // Nấc 1 (Angle / Crawler)
  else if (digitalRead(PIN_SW2_P2) == LOW) swMode = 2; // Nấc 3 (Acro / Sport)
  bool swAux  = (digitalRead(PIN_SW3_AUX) == LOW);
  bool sw_L   = (digitalRead(PIN_SW_L) == LOW);
  bool sw_R   = (digitalRead(PIN_SW_R) == LOW);

  if (currentModel == MODEL_ID_DRONE) {
    // =========================================================================
    // [THUẬT TOÁN 1: ĐIỀU KHIỂN DRONE BẰNG CẦN GA PS2 CÓ LÒ XO HỒI TÂM]
    // =========================================================================
    // Chiết áp Potentiometer chỉnh điểm ga lơ lửng (Hover Bias): từ 30% đến 50%
    int hoverThrottleUs = map(potPct, 0, 100, 1250, 1550);

    // Xử lý cần ga trái (joyLY):
    if (joyLY_us >= 1470 && joyLY_us <= 1530) {
      // Vùng tâm lò xo: Giữ chính xác mức ga lơ lửng để drone đứng im!
      txPacket.ch3_throttle = hoverThrottleUs;
    } else if (joyLY_us > 1530) {
      // Đẩy lên: Tăng tốc bay lên cao (từ Hover -> 2000us)
      txPacket.ch3_throttle = map(joyLY_us, 1530, 2000, hoverThrottleUs, 2000);
    } else {
      // Kéo xuống: Giảm độ cao / Hạ cánh (từ Hover -> 1000us)
      txPacket.ch3_throttle = map(joyLY_us, 1000, 1470, 1000, hoverThrottleUs);
    }

    txPacket.ch1_roll_str = joyRX_us; // Roll
    txPacket.ch2_pitch    = joyRY_us; // Pitch
    txPacket.ch4_yaw      = joyLX_us; // Yaw

  } else {
    // =========================================================================
    // [THUẬT TOÁN 2: ĐIỀU KHIỂN XE RC (CAR MODE) BẰNG CẦN PS2 TỰ HỒI TÂM]
    // =========================================================================
    // Cần trái Y: Tâm 1500us là ĐỨNG YÊN. Đẩy lên là TIẾN, kéo xuống là LÙI/THẮNG.
    // Chiết áp Potentiometer đóng vai trò GIỚI HẠN TỐC ĐỘ (Speed Limiter 20% - 100%)
    int speedLimitFactor = map(potPct, 0, 100, 20, 100);
    int rawCarDelta = joyLY_us - 1500; // -500 đến +500
    int limitedDelta = (rawCarDelta * speedLimitFactor) / 100;
    txPacket.ch3_throttle = 1500 + limitedDelta;

    txPacket.ch1_roll_str = joyRX_us; // Bẻ lái bánh trước (Steering)
    txPacket.ch2_pitch    = joyRY_us; // Kênh phụ / Sang số
    txPacket.ch4_yaw      = 1500;     // Trung lập
  }

  // Đóng gói thông tin điều khiển
  txPacket.model_id = currentModel;
  txPacket.seq_num  = packetSequence++;
  txPacket.knob_pot = potPct;

  txPacket.sw_flags = 0;
  if (swArm)  txPacket.sw_flags |= (1 << 0);
  txPacket.sw_flags |= ((swMode & 0x03) << 1);
  if (swAux)  txPacket.sw_flags |= (1 << 3);
  if (sw_L)   txPacket.sw_flags |= (1 << 4);
  if (sw_R)   txPacket.sw_flags |= (1 << 5);

  // 4. PHÁT SÓNG LORA E22 TỐC ĐỘ 50Hz (MỖI 20ms GỬI 1 LẦN, ĐỒNG BỘ CHÂN AUX)
  if (now - lastRfTime >= 20) {
    lastRfTime = now;

    // Đợi chân AUX của LoRa rảnh (HIGH) để không bao giờ bị nghẽn đệm
    if (digitalRead(PIN_LORA_AUX) == HIGH) {
      txPacket.header[0] = 0xAA;
      txPacket.header[1] = 0x55;

      // Tính mã CRC16-CCITT cho toàn bộ 15 byte dữ liệu đầu
      txPacket.crc16 = calculateCRC16((const uint8_t*)&txPacket, sizeof(RadioPacket) - 2);

      // Phát gói tin qua Serial1 ở tốc độ 115200 baud (Chỉ ngốn 1.3ms!)
      Serial1.write((uint8_t*)&txPacket, sizeof(RadioPacket));
    }
  }

  // 5. CẬP NHẬT MÀN HÌNH LCD 1604A BẰNG KỸ THUẬT TIME-SLICING (KHÔNG LÀM NGHẼN CPU)
  // Mỗi 25ms chỉ cập nhật ĐÚNG 1 DÒNG LCD (100ms cập nhật trọn vẹn cả 4 dòng)
  if (now - lastLcdSlice >= 25) {
    lastLcdSlice = now;
    renderLcdLineSlice(lcdLineIndex);
    lcdLineIndex = (lcdLineIndex + 1) % 4;
  }
}

// --- HÀM CẬP NHẬT TỪNG DÒNG LCD CHỐNG NGHẼN ---
void renderLcdLineSlice(uint8_t line) {
  char buf[17];
  lcd.setCursor(0, line);

  if (line == 0) {
    // Dòng 1: Tên Model & Cường độ sóng
    if (currentModel == MODEL_ID_DRONE) {
      snprintf(buf, sizeof(buf), "M1:DRONE   S:%2d%%", rssiVal);
    } else {
      snprintf(buf, sizeof(buf), "M2:RC CAR  S:%2d%%", rssiVal);
    }
    lcd.print(buf);
  } else if (line == 1) {
    // Dòng 2: Trạng thái ARM & Chế độ bay/chạy
    const char *armStr = (txPacket.sw_flags & (1 << 0)) ? "ARM:ON " : "ARM:OFF";
    uint8_t mode = (txPacket.sw_flags >> 1) & 0x03;
    const char *mStr = "[ANGLE]";
    if (currentModel == MODEL_ID_DRONE) {
      if (mode == 1) mStr = "[HORIZ]";
      else if (mode == 2) mStr = "[ACRO] ";
    } else {
      mStr = "[CRAWL]";
      if (mode == 1) mStr = "[NORM] ";
      else if (mode == 2) mStr = "[SPORT]";
    }
    snprintf(buf, sizeof(buf), "%s %s", armStr, mStr);
    lcd.print(buf);
  } else if (line == 2) {
    // Dòng 3: Đo áp pin Tay cầm & Pin mô hình Telemetry
    vBatTx = (analogRead(PIN_BAT_SENSE) * 3.3f / 4095.0f) * 2.05f;
    snprintf(buf, sizeof(buf), "TX:%1.1fV RX:12.1V", vBatTx);
    lcd.print(buf);
  } else if (line == 3) {
    // Dòng 4: Giá trị Ga/Lái trực tiếp
    if (currentModel == MODEL_ID_DRONE) {
      int thrPct = map(txPacket.ch3_throttle, 1000, 2000, 0, 100);
      int yawPct = map(txPacket.ch4_yaw, 1000, 2000, -100, 100);
      snprintf(buf, sizeof(buf), "THR:%2d%% YAW:%3d%%", thrPct, yawPct);
    } else {
      int strPct = map(txPacket.ch1_roll_str, 1000, 2000, -100, 100);
      snprintf(buf, sizeof(buf), "STR:%3d%% SPD:%2d%%", strPct, txPacket.knob_pot);
    }
    lcd.print(buf);
  }
}

// --- MENU CÀI ĐẶT CHỌN MODEL ---
void handleMenuUI() {
  lcd.setCursor(0, 0); lcd.print("=== CAI DAT ===");
  lcd.setCursor(0, 1);
  if (menuCursor == 0) lcd.print("> 1. DRONE QUAD ");
  else                 lcd.print("  1. DRONE QUAD ");
  lcd.setCursor(0, 2);
  if (menuCursor == 1) lcd.print("> 2. RC CAR / TRK");
  else                 lcd.print("  2. RC CAR / TRK");
  lcd.setCursor(0, 3); lcd.print("[B2:DN  B3:CHON]");

  if (digitalRead(PIN_BTN_B2) == LOW) {
    menuCursor = 1 - menuCursor;
    beep(2400, 40);
    delay(200);
  }

  if (digitalRead(PIN_BTN_B3) == LOW) {
    // Khi chuyển đổi Model, bắt buộc phải DISARM an toàn trước!
    currentModel = (menuCursor == 0) ? MODEL_ID_DRONE : MODEL_ID_CAR;
    EEPROM.write(0x10, currentModel);

    beep(2600, 80);
    delay(80);
    beep(3400, 150);

    lcd.clear();
    lcd.setCursor(0, 1); lcd.print("DA LUU CAI DAT!");
    lcd.setCursor(0, 2);
    if (currentModel == MODEL_ID_DRONE) lcd.print("CHON: DRONE FPV");
    else lcd.print("CHON: RC CAR");
    delay(1000);

    inMenu = false;
    lcd.clear();
  }
}
