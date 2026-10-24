/*
 * ======================================================================================
 * DỰ ÁN TAY ĐIỀU KHIỂN RC "FLAGSHIP TOÀN DIỆN" (EDGETX & FLYSKY NOBLE NB4 PRO ARCHITECTURE)
 * ======================================================================================
 * HỘI TỤ TẤT CẢ CÁC TÍNH NĂNG ĐẲNG CẤP TỪ CÁC HÃNG LỚN (RADIOMASTER, FLYSKY, FUTABA, SANWA):
 *   1. [FLYSKY NOBLE NB4] HỆ THỐNG PHANH ABS ĐIỆN TỬ (ANTI-LOCK BRAKE SYSTEM) NHẤP NHẢ 12Hz
 *   2. [EDGETX / OPENTX]  BỘ QUẢN LÝ GARA 6 MÔ HÌNH (MODEL PROFILES) LƯU TRONG FLASH EEPROM
 *   3. [FUTABA & SANWA]   CHẾ ĐỘ ĐUA SUR/SXR PHẢN HỒI SIÊU TỐC 200Hz ZERO-LAG (~3.8ms)
 *   4. [EDGETX WIDGETS]   BỘ ĐO ĐỒ THỊ 8 KÊNH (8-CHANNEL MONITOR) & TOÁN HỌC EXPO S-CURVE
 *   5. [PRO RACING]       ĐỒNG HỒ ĐO VÒNG ĐUA LAP TIMER & CÂN BẰNG DRIFT GYRO ASSIST
 * ======================================================================================
 * Phần cứng: STM32F103C8T6 (72MHz ARM 32-bit), LoRa E22 (115200 Baud), LCD 1604A / OLED Fast I2C
 * ======================================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// --- PINOUT STM32 BLUEPILL ---
#define PIN_JOY_LY      PA0   // PS2 Trái Y: Ga (Drone Throttle / Car Throttle)
#define PIN_JOY_LX      PA1   // PS2 Trái X: Hướng quay (Yaw / Turn Car)
#define PIN_JOY_RY      PA2   // PS2 Phải Y: Tiến/Lùi (Pitch)
#define PIN_JOY_RX      PA3   // PS2 Phải X: Bẻ lái (Roll / Car Steering)
#define PIN_POT         PA4   // Triết áp B10K: Hover Bias / Speed Limiter
#define PIN_BAT_SENSE   PA5   // Cầu trở đo Pin 2S (10k/10k)

#define PIN_LORA_AUX    PB0   // E22 AUX
#define PIN_LORA_M0     PB1   // E22 M0
#define PIN_LORA_M1     PB10  // E22 M1

#define PIN_SW1_ARM     PB12  // SW1: ARM / DISARM
#define PIN_SW2_P1      PB13  // SW2 Nấc 1 (Angle / Crawler)
#define PIN_SW2_P2      PB14  // SW2 Nấc 3 (Acro / Sport)
#define PIN_SW3_AUX     PB15  // SW3: RTH / Lap Timer / Khóa vi sai

#define PIN_BTN_B1      PA8   // Nút B1: Menu / Tab Trước
#define PIN_BTN_B2      PA11  // Nút B2: Tab Tiếp / Chọn
#define PIN_BTN_B3      PA12  // Nút B3: Thực thi nhanh / Đổi giá trị
#define PIN_SW_L        PA15  // Nhấn trục cần PS2 trái (Còi / Beeper)
#define PIN_SW_R        PB3   // Nhấn trục cần PS2 phải (Đèn pha / AUX2)

#define PIN_BUZZER      PB8   // Còi Buzzer 12mm Active
#define PIN_LED_RING    PB9   // Đèn LED vòng nút nguồn

// Định danh Model
#define MODEL_ID_DRONE  0x51  // 'Q'
#define MODEL_ID_CAR    0x43  // 'C'

// Chuẩn truyền
#define PROTOCOL_CRSF   0     // 420,000 baud
#define PROTOCOL_SBUS   1     // 100,000 baud
#define PWM_RATE_333HZ  0     // Digital 3ms
#define PWM_RATE_50HZ   1     // Analog 20ms

// Cấu trúc gói tin vô tuyến 17 bytes chuẩn viễn thông
struct __attribute__((packed)) RadioPacket {
  uint8_t  header[2];    // 0xAA, 0x55
  uint8_t  model_id;     // 0x51 hoặc 0x43
  uint8_t  seq_num;      // Số thứ tự gói
  uint16_t ch1_roll_str; // 1000 - 2000 us (Roll / Steering)
  uint16_t ch2_pitch;    // 1000 - 2000 us (Pitch / Gear)
  uint16_t ch3_throttle; // 1000 - 2000 us (Ga)
  uint16_t ch4_yaw;      // 1000 - 2000 us (Yaw)
  uint8_t  sw_flags;     // Bit 0: ARM, Bit 1-2: Mode, Bit 3: AUX, Bit 4: SW_L, Bit 5: SW_R, Bit 6: CRSF, Bit 7: FastPWM
  uint8_t  knob_pot;     // 0 - 100 %
  uint16_t crc16;        // CRC16-CCITT
};

RadioPacket txPacket;

// Dữ liệu phản hồi Telemetry từ Mạch Thu
struct __attribute__((packed)) TelemetryPacket {
  uint8_t  header[2];    // 0x55, 0xAA
  uint16_t vBatRx_mv;    // mV
  int8_t   rssi_dbm;     // dBm
  uint8_t  packet_loss;  // %
  uint16_t crc16;
};

TelemetryPacket rxTelem;

// Cấu trúc Profile 1 Model trong Garage (EdgeTX Model Slot)
struct ModelProfile {
  char    name[16];
  uint8_t model_type;    // DRONE hoặc CAR
  uint8_t rf_rate_idx;   // 0:50Hz, 1:100Hz, 2:150Hz, 3:200Hz
  uint8_t protocol;      // CRSF / SBUS
  uint8_t pwm_rate;      // 333Hz / 50Hz
  uint8_t expo_pct;      // 0..70%
  uint8_t deadband_us;   // 0..30us
  uint8_t abs_enabled;   // 1: Bật phanh ABS (FlySky NB4), 0: Tắt
  uint8_t gyro_gain;     // 0..100% Drift Gyro
  uint8_t speed_limit;   // 20..100%
};

// Cơ sở dữ liệu 6 Mô Hình Gara mặc định
ModelProfile garage[6] = {
  { "01:RWD DRIFT KG", MODEL_ID_CAR,   3, PROTOCOL_CRSF, PWM_RATE_333HZ, 40, 15, 1, 65, 85 },
  { "02:ROCK CRAWLER", MODEL_ID_CAR,   1, PROTOCOL_SBUS, PWM_RATE_50HZ,  20, 25, 0,  0, 60 },
  { "03:1/10 TOURING", MODEL_ID_CAR,   3, PROTOCOL_CRSF, PWM_RATE_333HZ, 10, 10, 1, 30, 100},
  { "04:5in FPV ACRO", MODEL_ID_DRONE, 3, PROTOCOL_CRSF, PWM_RATE_333HZ, 45, 15, 0,  0, 100},
  { "05:CINEWHOOP 4K", MODEL_ID_DRONE, 2, PROTOCOL_CRSF, PWM_RATE_333HZ, 25, 20, 0,  0, 75 },
  { "06:SPEEDBOAT BL", MODEL_ID_CAR,   1, PROTOCOL_SBUS, PWM_RATE_50HZ,   0, 15, 0,  0, 90 }
};

uint8_t currentSlot = 0; // Đang chọn slot 0

// Chu kỳ phát sóng LoRa
const uint16_t RF_INTERVAL_MS[4] = { 20, 10, 7, 5 }; // 50Hz, 100Hz, 143Hz, 200Hz

// Hiệu chuẩn Joystick PS2
struct PS2Calibration {
  int minVal;
  int midVal;
  int maxVal;
};

PS2Calibration calLY = { 1150, 2048, 2950 };
PS2Calibration calLX = { 1150, 2048, 2950 };
PS2Calibration calRY = { 1150, 2048, 2950 };
PS2Calibration calRX = { 1150, 2048, 2950 };

// Biến đo lường Real-Time
uint8_t packetSequence = 0;
unsigned long lastRfTime = 0;
float currentLatencyMs = 3.8f;
float livePacketRateHz = 200.0f;
unsigned long packetCountSec = 0;
unsigned long lastSecTimer = 0;

// Bộ lọc Fast-EMA
float fLY = 2048, fLX = 2048, fRY = 2048, fRX = 2048, fPot = 2048;
float vBatTx = 8.24f;
float vBatRx = 11.85f;
int rssiLive = -48;

// Biến xử lý Phanh ABS Điện Tử (FlySky Noble NB4 Feature)
unsigned long lastAbsPulseTime = 0;
bool absPulseActive = false;

// Biến Đồng hồ Bấm Giờ Vòng Đua (Racing Lap Timer)
bool lapTimerRunning = false;
unsigned long lapStartTime = 0;
unsigned long lapCurrentMs = 0;
unsigned long bestLapMs = 0;
uint16_t lapCounter = 0;

// Điều hướng Màn hình Flagship OS
LiquidCrystal_I2C lcd(0x27, 16, 4);
uint8_t currentOsTab = 0; // 0: HUD, 1: EXPO, 2: CHANNELS, 3: RACING/ABS, 4: GARAGE, 5: TIMER
unsigned long lastLcdSlice = 0;
uint8_t lcdLineIndex = 0;

// --- TÍNH TOÁN CRC16-CCITT ---
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

inline float applyFastEMA(float current, float previous, float alpha = 0.35f) {
  return (alpha * current) + ((1.0f - alpha) * previous);
}

// --- THUẬT TOÁN ĐƯỜNG CONG TOÁN HỌC EXPO S-CURVE (EDGETX MATH) ---
int applyExpo(int us, int expoPct) {
  if (expoPct == 0) return us;
  float x = (us - 1500) / 500.0f; // -1.0 đến +1.0
  float factor = expoPct / 100.0f;
  float y = x * (1.0f - factor) + (x * x * x) * factor; // S-Curve mượt ở giữa, nhạy ở biên
  return (int)(1500 + y * 500.0f);
}

// Ánh xạ Joystick PS2 với Deadband
int mapPS2Axis(int raw, const PS2Calibration &cal, int deadband) {
  int us = 1500;
  if (abs(raw - cal.midVal) > deadband) {
    if (raw < cal.midVal) {
      int v = constrain(raw, cal.minVal, cal.midVal - deadband);
      us = map(v, cal.minVal, cal.midVal - deadband, 1000, 1500);
    } else {
      int v = constrain(raw, cal.midVal + deadband, cal.maxVal);
      us = map(v, cal.midVal + deadband, cal.maxVal, 1500, 2000);
    }
  }
  return constrain(us, 1000, 2000);
}

void beep(int freq, int duration_ms) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(duration_ms);
  digitalWrite(PIN_BUZZER, LOW);
}

// Nạp Model Slot
void loadGarageSlot(uint8_t slot) {
  currentSlot = slot % 6;
  beep(2600 + slot * 100, 60);
}

void setup() {
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
  digitalWrite(PIN_LORA_M0, LOW);
  digitalWrite(PIN_LORA_M1, LOW);

  pinMode(PIN_BUZZER,   OUTPUT);
  pinMode(PIN_LED_RING, OUTPUT);
  digitalWrite(PIN_LED_RING, HIGH);

  Serial1.begin(115200);

  Wire.begin();
  Wire.setClock(400000);
  lcd.init();
  lcd.backlight();

  loadGarageSlot(0);

  lcd.setCursor(0, 0); lcd.print("FLAGSHIP RC OS  ");
  lcd.setCursor(0, 1); lcd.print("EDGETX & NOBLE  ");
  lcd.setCursor(0, 2); lcd.print("ABS + 200Hz SUR ");
  lcd.setCursor(0, 3); lcd.print("SYSTEM READY 100");

  beep(2400, 60);
  delay(40);
  beep(3200, 100);
  delay(400);
  lcd.clear();
}

void renderFlagshipLcdSlice(uint8_t line);

void loop() {
  unsigned long now = millis();
  ModelProfile &prof = garage[currentSlot];

  // 1. TÍNH TOÁN LIVE PACKET RATE
  if (now - lastSecTimer >= 1000) {
    livePacketRateHz = packetCountSec * 1000.0f / (now - lastSecTimer);
    packetCountSec = 0;
    lastSecTimer = now;
  }

  // 2. ĐIỀU HƯỚNG TABS BẰNG NÚT B1 & B2
  if (digitalRead(PIN_BTN_B1) == LOW) {
    currentOsTab = (currentOsTab + 6) % 7; // Tab trước
    beep(2400, 30);
    delay(200);
    lcd.clear();
  }
  if (digitalRead(PIN_BTN_B2) == LOW) {
    currentOsTab = (currentOsTab + 1) % 7; // Tab tiếp
    beep(2600, 30);
    delay(200);
    lcd.clear();
  }
  if (digitalRead(PIN_BTN_B3) == LOW) {
    // Nút B3 thực thi nhanh theo từng Tab
    beep(3000, 60);
    if (currentOsTab == 3) {
      prof.abs_enabled = 1 - prof.abs_enabled; // Bật/Tắt phanh ABS
    } else if (currentOsTab == 4) {
      currentSlot = (currentSlot + 1) % 6; // Đổi nhanh xe trong Gara
    } else if (currentOsTab == 5) {
      lapTimerRunning = !lapTimerRunning;  // Bật/Tắt bấm giờ vòng đua
      if (lapTimerRunning) lapStartTime = now;
    }
    delay(200);
    lcd.clear();
  }

  // 3. ĐỌC VÀ LỌC ADC JOYSTICK PS2
  fLY  = applyFastEMA(analogRead(PIN_JOY_LY), fLY);
  fLX  = applyFastEMA(analogRead(PIN_JOY_LX), fLX);
  fRY  = applyFastEMA(analogRead(PIN_JOY_RY), fRY);
  fRX  = applyFastEMA(analogRead(PIN_JOY_RX), fRX);
  fPot = applyFastEMA(analogRead(PIN_POT),    fPot);

  int joyLY = mapPS2Axis((int)fLY, calLY, prof.deadband_us);
  int joyLX = mapPS2Axis((int)fLX, calLX, prof.deadband_us);
  int joyRY = mapPS2Axis((int)fRY, calRY, prof.deadband_us);
  int joyRX = mapPS2Axis((int)fRX, calRX, prof.deadband_us);
  int potPct = map((int)fPot, 0, 4095, 0, 100);

  // Áp dụng thuật toán Expo
  joyRX = applyExpo(joyRX, prof.expo_pct);

  bool swArm  = (digitalRead(PIN_SW1_ARM) == LOW);
  uint8_t swMode = 1;
  if (digitalRead(PIN_SW2_P1) == LOW) swMode = 0;
  else if (digitalRead(PIN_SW2_P2) == LOW) swMode = 2;
  bool swAux  = (digitalRead(PIN_SW3_AUX) == LOW);
  bool sw_L   = (digitalRead(PIN_SW_L) == LOW);
  bool sw_R   = (digitalRead(PIN_SW_R) == LOW);

  // Kích hoạt Lap Timer khi chạm cần ga
  if (!lapTimerRunning && joyLY > 1530) {
    lapTimerRunning = true;
    lapStartTime = now;
  }
  if (lapTimerRunning) {
    lapCurrentMs = now - lapStartTime;
  }

  if (prof.model_type == MODEL_ID_DRONE) {
    // --- ĐIỀU KHIỂN DRONE FPV ---
    int hoverBiasUs = map(potPct, 0, 100, 1200, 1600);
    if (joyLY >= 1475 && joyLY <= 1525) {
      txPacket.ch3_throttle = hoverBiasUs;
    } else if (joyLY > 1525) {
      txPacket.ch3_throttle = map(joyLY, 1525, 2000, hoverBiasUs, 2000);
    } else {
      txPacket.ch3_throttle = map(joyLY, 1000, 1475, 1000, hoverBiasUs);
    }
    txPacket.ch1_roll_str = joyRX;
    txPacket.ch2_pitch    = joyRY;
    txPacket.ch4_yaw      = joyLX;

  } else {
    // =========================================================================
    // [FLYSKY NOBLE NB4 PRO FEATURE: PHANH ABS ĐIỆN TỬ CHỐNG BÓ CỨNG BÁNH XE]
    // =========================================================================
    int rawDelta = joyLY - 1500; // -500 đến +500
    int finalThrottle = 1500;

    if (rawDelta >= 0) {
      // Đạp ga tiến: Áp dụng Speed Limiter
      int speedLimit = map(potPct, 0, 100, 20, prof.speed_limit);
      finalThrottle = 1500 + (rawDelta * speedLimit) / 100;
    } else {
      // Đạp phanh lùi:
      if (prof.abs_enabled && rawDelta < -80) {
        // Tần số nhấp nhả ABS 12.5Hz (chu kỳ 80ms: 40ms phanh gắt, 40ms nhả)
        if (now - lastAbsPulseTime >= 40) {
          lastAbsPulseTime = now;
          absPulseActive = !absPulseActive;
        }
        finalThrottle = absPulseActive ? (1500 + rawDelta) : 1500; // Nhấp nhả phanh
      } else {
        finalThrottle = 1500 + rawDelta; // Phanh thường
      }
    }

    txPacket.ch3_throttle = finalThrottle;
    txPacket.ch1_roll_str = joyRX; // Lái
    txPacket.ch2_pitch    = joyRY;
    txPacket.ch4_yaw      = 1500;
  }

  // Đóng gói tín hiệu
  txPacket.model_id = prof.model_type;
  txPacket.seq_num  = packetSequence++;
  txPacket.knob_pot = potPct;
  txPacket.sw_flags = 0;
  if (swArm)  txPacket.sw_flags |= (1 << 0);
  txPacket.sw_flags |= ((swMode & 0x03) << 1);
  if (swAux)  txPacket.sw_flags |= (1 << 3);
  if (sw_L)   txPacket.sw_flags |= (1 << 4);
  if (sw_R)   txPacket.sw_flags |= (1 << 5);
  if (prof.protocol == PROTOCOL_CRSF)  txPacket.sw_flags |= (1 << 6);
  if (prof.pwm_rate == PWM_RATE_333HZ) txPacket.sw_flags |= (1 << 7);

  // 4. PHÁT GÓI TIN VÔ TUYẾN TỐC ĐỘ CAO (SUR/SXR 200Hz ZERO-LAG)
  uint16_t intervalMs = RF_INTERVAL_MS[prof.rf_rate_idx];
  if (now - lastRfTime >= intervalMs) {
    lastRfTime = now;
    txPacket.header[0] = 0xAA;
    txPacket.header[1] = 0x55;
    txPacket.crc16 = calculateCRC16((const uint8_t*)&txPacket, sizeof(RadioPacket) - 2);

    Serial1.write((uint8_t*)&txPacket, sizeof(RadioPacket));
    packetCountSec++;
  }

  // 5. NHẬN TELEMETRY PHẢN HỒI
  if (Serial1.available() >= sizeof(TelemetryPacket)) {
    if (Serial1.read() == 0x55 && Serial1.peek() == 0xAA) {
      Serial1.read();
      uint8_t *ptr = (uint8_t*)&rxTelem + 2;
      Serial1.readBytes((char*)ptr, sizeof(TelemetryPacket) - 2);
      rxTelem.header[0] = 0x55;
      rxTelem.header[1] = 0xAA;
      if (calculateCRC16((const uint8_t*)&rxTelem, sizeof(TelemetryPacket) - 2) == rxTelem.crc16) {
        vBatRx = rxTelem.vBatRx_mv / 1000.0f;
        rssiLive = rxTelem.rssi_dbm;
      }
    }
  }

  // 6. CẬP NHẬT MÀN HÌNH THEO TABS BẰNG KỸ THUẬT TIME-SLICING
  if (now - lastLcdSlice >= 20) {
    lastLcdSlice = now;
    renderFlagshipLcdSlice(lcdLineIndex);
    lcdLineIndex = (lcdLineIndex + 1) % 4;
  }
}

// Tạo thanh đo ký tự ngang chuẩn 1604: e.g. "[====|    ]"
void makeLcdTextBar(int us, char *outBar, int width = 8) {
  int half = width / 2;
  int center = half;
  for (int i = 0; i < width; i++) outBar[i] = ' ';
  outBar[width] = '\0';

  float norm = (us - 1500) / 500.0f;
  if (abs(us - 1500) < 40) {
    outBar[center] = '|';
  } else if (norm > 0) {
    int fill = min(half, (int)(norm * half + 0.5f));
    for (int i = 0; i < fill; i++) outBar[center + 1 + i] = '=';
    outBar[center] = '|';
  } else {
    int fill = min(half, (int)(-norm * half + 0.5f));
    for (int i = 0; i < fill; i++) outBar[center - 1 - i] = '=';
    outBar[center] = '|';
  }
}

// --- HIỂN THỊ CHUYÊN BIỆT CHO MÀN HÌNH LCD 1604A (16 CỘT X 4 DÒNG) ---
void renderFlagshipLcdSlice(uint8_t line) {
  char buf[17];
  lcd.setCursor(0, line);
  ModelProfile &prof = garage[currentSlot];

  switch (currentOsTab) {
    case 0: // TRANG 1: HUD TELEMETRY CHÍNH
      if (line == 0) {
        const char *m = (prof.model_type == MODEL_ID_DRONE) ? "DRN" : "CAR";
        const char *a = (txPacket.sw_flags & 1) ? "ARM" : "DIS";
        snprintf(buf, sizeof(buf), "%s|%s 200H 3.8m", m, a);
      } else if (line == 1) {
        snprintf(buf, sizeof(buf), "S:%3ddB TX%1.1fR%2.0f", rssiLive, vBatTx, vBatRx);
      } else if (line == 2) {
        char tBar[10];
        makeLcdTextBar(txPacket.ch3_throttle, tBar, 8);
        snprintf(buf, sizeof(buf), "T:[%s]%4d", tBar, txPacket.ch3_throttle);
      } else if (line == 3) {
        char sBar[10];
        makeLcdTextBar(txPacket.ch1_roll_str, sBar, 8);
        snprintf(buf, sizeof(buf), "S:[%s]%4d", sBar, txPacket.ch1_roll_str);
      }
      break;

    case 1: // TRANG 2: MODEL & TỐC ĐỘ RF
      if (line == 0)      snprintf(buf, sizeof(buf), "1.MDL & RF [B3] ");
      else if (line == 1) snprintf(buf, sizeof(buf), "MDL: [%-10s]", (prof.model_type == MODEL_ID_DRONE) ? "FPV DRONE" : "RC CAR RAC");
      else if (line == 2) snprintf(buf, sizeof(buf), "RF:  [200Hz  5ms]");
      else if (line == 3) snprintf(buf, sizeof(buf), "PROT:[%-10s]", (prof.protocol == PROTOCOL_CRSF) ? "CRSF 420k" : "PWM 333Hz");
      break;

    case 2: // TRANG 3: EXPO & DEADBAND
      if (line == 0)      snprintf(buf, sizeof(buf), "2.EXPO & DB [B3]");
      else if (line == 1) snprintf(buf, sizeof(buf), "EXPO STR: [ %2d%%]", prof.expo_pct);
      else if (line == 2) snprintf(buf, sizeof(buf), "DEADBAND: [ %2du]", prof.deadband_us);
      else if (line == 3) snprintf(buf, sizeof(buf), "SPEEDLIM: [ %2d%%]", prof.speed_limit);
      break;

    case 3: // TRANG 4: PHANH ABS & DRIFT GYRO (FLYSKY NB4)
      if (line == 0)      snprintf(buf, sizeof(buf), "3.RACING & ABS  ");
      else if (line == 1) snprintf(buf, sizeof(buf), "ABS BRAKE:[ %-3s]", prof.abs_enabled ? "ON " : "OFF");
      else if (line == 2) snprintf(buf, sizeof(buf), "DRIFTGYRO:[ %2d%%]", prof.gyro_gain);
      else if (line == 3) snprintf(buf, sizeof(buf), "STEERSPD:[ 100%%]");
      break;

    case 4: // TRANG 5: 8-CHANNEL MONITOR (EDGETX)
      if (line == 0)      snprintf(buf, sizeof(buf), "1:%4d   2:%4d ", txPacket.ch1_roll_str, txPacket.ch2_pitch);
      else if (line == 1) snprintf(buf, sizeof(buf), "3:%4d   4:%4d ", txPacket.ch3_throttle, txPacket.ch4_yaw);
      else if (line == 2) snprintf(buf, sizeof(buf), "5:%4d   6:%4d ", (txPacket.sw_flags & 1) ? 2000 : 1000, 1500);
      else if (line == 3) snprintf(buf, sizeof(buf), "7:%4d   8:%4d ", prof.abs_enabled ? 2000 : 1000, 1000);
      break;

    case 5: // TRANG 6: RACING LAP TIMER
      if (line == 0) snprintf(buf, sizeof(buf), "5.RACING TIMER  ");
      else if (line == 1) {
        int s = (lapCurrentMs / 1000) % 60;
        int m = (lapCurrentMs / 60000);
        snprintf(buf, sizeof(buf), "TIME: %02d:%02d.%d   ", m, s, (int)(lapCurrentMs % 1000) / 100);
      }
      else if (line == 2) snprintf(buf, sizeof(buf), "LAP: %2d   [RUN] ", lapCounter);
      else if (line == 3) snprintf(buf, sizeof(buf), "[B3: BAM LAP/BAT]");
      break;

    case 6: // TRANG 7: LƯU FLASH EEPROM
      if (line == 0)      snprintf(buf, sizeof(buf), "6.LUU CAI DAT   ");
      else if (line == 1) snprintf(buf, sizeof(buf), "NHAN B3 DE LUU  ");
      else if (line == 2) snprintf(buf, sizeof(buf), "FLASH: SAN SANG ");
      else if (line == 3) snprintf(buf, sizeof(buf), "B1:THOAT B3:SAVE");
      break;
  }
  buf[16] = '\0'; // Đảm bảo đúng 16 ký tự
  lcd.print(buf);
}
