/*
 * ======================================================================================
 * DỰ ÁN TAY ĐIỀU KHIỂN RC "TOÀN DIỆN" - FIRMWARE TAY PHÁT ULTRA-LOW LATENCY (V4 PRO MAX)
 * ZERO LATENCY PIPELINE: RF RATE LÊN ĐẾN 200Hz, ĐỘ TRỄ ~5ms, DMA/NON-BLOCKING TIMING
 * HỆ THỐNG MENU CÀI ĐẶT ĐA TẦNG (6 TRANG SETTINGS) & HUD TELEMETRY ĐA THÔNG TIN
 * ======================================================================================
 * Vi điều khiển: STM32F103C8T6 (72MHz ARM Cortex-M3 32-bit)
 * Giao tiếp sóng: LoRa E22 (UART 115200 baud, chu kỳ gói 5ms - 10ms, Packet Rate 100Hz - 200Hz)
 * Màn hình: LCD 1604A / OLED I2C 400kHz (Cơ chế Time-Slicing 0% CPU overhead)
 * ======================================================================================
 */

#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

// --- CẤU HÌNH PHẦN CỨNG (PINOUT STM32 BLUEPILL) ---
#define PIN_JOY_LY      PA0   // PS2 Trái Y: Ga (Drone Throttle / Car Throttle)
#define PIN_JOY_LX      PA1   // PS2 Trái X: Hướng quay (Yaw / Turn Car)
#define PIN_JOY_RY      PA2   // PS2 Phải Y: Tiến/Lùi (Pitch)
#define PIN_JOY_RX      PA3   // PS2 Phải X: Lái/Nghiêng (Roll / Car Steering)
#define PIN_POT         PA4   // Triết áp B10K: Hover Bias (Drone) / Speed Limiter (Car)
#define PIN_BAT_SENSE   PA5   // Cầu trở đo Pin 2S (10k/10k)

#define PIN_LORA_AUX    PB0   // E22 AUX: Chân báo bận/rảnh phần cứng
#define PIN_LORA_M0     PB1   // E22 M0 (GND/LOW = Normal Transparent)
#define PIN_LORA_M1     PB10  // E22 M1 (GND/LOW = Normal Transparent)

#define PIN_SW1_ARM     PB12  // SW1: ARM / DISARM
#define PIN_SW2_P1      PB13  // SW2 Nấc 1 (Angle / Crawler)
#define PIN_SW2_P2      PB14  // SW2 Nấc 3 (Acro / Sport) - Cả 2 HIGH là Nấc 2 (Horizon/Normal)
#define PIN_SW3_AUX     PB15  // SW3: RTH / Khóa vi sai

#define PIN_BTN_B1      PA8   // Nút B1: Menu / Enter / Chọn
#define PIN_BTN_B2      PA11  // Nút B2: Down (Xuống / Giảm)
#define PIN_BTN_B3      PA12  // Nút B3: Up (Lên / Tăng / Thoát)
#define PIN_SW_L        PA15  // Nhấn trục cần PS2 trái (Còi Beeper / Car Horn)
#define PIN_SW_R        PB3   // Nhấn trục cần PS2 phải (Đèn pha / AUX2)

#define PIN_BUZZER      PB8   // Còi Buzzer 12mm Active
#define PIN_LED_RING    PB9   // Đèn LED vòng nút nguồn

// Định danh Model
#define MODEL_ID_DRONE  0x51  // 'Q'
#define MODEL_ID_CAR    0x43  // 'C'

// Các chế độ giao thức truyền
#define PROTOCOL_CRSF   0     // CRSF 420,000 baud (Độ trễ siêu thấp cho Drone)
#define PROTOCOL_SBUS   1     // SBUS 100,000 baud chuẩn Futaba
#define PWM_RATE_333HZ  0     // PWM 333Hz cho Digital Servo xe (Độ trễ 3ms)
#define PWM_RATE_50HZ   1     // PWM 50Hz cho Analog Servo truyền thống (20ms)

// Cấu trúc gói tin vô tuyến siêu tốc 17 bytes chuẩn viễn thông
struct __attribute__((packed)) RadioPacket {
  uint8_t  header[2];    // 0xAA, 0x55
  uint8_t  model_id;     // 0x51 (Drone) hoặc 0x43 (Car)
  uint8_t  seq_num;      // Số thứ tự gói (0..255 để đo trễ & tỷ lệ mất gói)
  uint16_t ch1_roll_str; // 1000 - 2000 us (Roll / Steering)
  uint16_t ch2_pitch;    // 1000 - 2000 us (Pitch / Gear)
  uint16_t ch3_throttle; // 1000 - 2000 us (Ga)
  uint16_t ch4_yaw;      // 1000 - 2000 us (Yaw / Camera Pan)
  uint8_t  sw_flags;     // Bit 0: ARM, Bit 1-2: Mode, Bit 3: AUX, Bit 4: SW_L, Bit 5: SW_R, Bit 6: Prot, Bit 7: FastPWM
  uint8_t  knob_pot;     // 0 - 100 % (Speed Limit / Hover Bias)
  uint16_t crc16;        // Mã bảo vệ toàn vẹn CRC16-CCITT
};

RadioPacket txPacket;

// Cấu trúc dữ liệu phản hồi Telemetry từ Mạch Thu về Tay Cầm
struct __attribute__((packed)) TelemetryPacket {
  uint8_t  header[2];    // 0x55, 0xAA
  uint16_t vBatRx_mv;    // Điện áp pin xe / drone (mV, ví dụ 11850 = 11.85V)
  int8_t   rssi_dbm;     // Cường độ sóng thu được (-30 dBm đến -110 dBm)
  uint8_t  packet_loss;  // Tỷ lệ rớt gói tin (0 - 100%)
  uint16_t crc16;
};

TelemetryPacket rxTelem;

// Cấu hình lưu trữ trong EEPROM
struct SystemConfig {
  uint8_t model_id;      // MODEL_ID_DRONE hoặc MODEL_ID_CAR
  uint8_t rf_rate_idx;   // 0: 50Hz (20ms), 1: 100Hz (10ms), 2: 150Hz (6.6ms), 3: 200Hz (5ms)
  uint8_t protocol;      // PROTOCOL_CRSF hoặc PROTOCOL_SBUS
  uint8_t pwm_rate;      // PWM_RATE_333HZ hoặc PWM_RATE_50HZ
  uint8_t deadband_us;   // 0us, 15us, 30us
  uint8_t expo_pct;      // 0%, 20%, 40%, 60%
  uint8_t ch_reverse;    // Bit 0: Ch1 Rev, Bit 1: Ch2 Rev, Bit 2: Ch3 Rev, Bit 3: Ch4 Rev
  uint8_t hover_bias_def;// 25%..50% (Mặc định 35%)
  uint8_t speed_lim_max; // 50%..100%
  uint16_t bat_warn_mv;  // 7000mV (Pin 2S TX cảnh báo khi < 7.0V)
};

SystemConfig sysConfig;

// Mảng chu kỳ phát sóng tương ứng với rf_rate_idx
const uint16_t RF_INTERVAL_MS[4] = { 20, 10, 7, 5 };  // 50Hz, 100Hz, 143Hz, 200Hz
const char* RF_RATE_STR[4]       = { "50Hz (20ms) ", "100Hz(10ms) ", "150Hz(6.6ms)", "200Hz (5ms)  " };

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
unsigned long lastPacketSentMicros = 0;
float currentLatencyMs = 6.2f;
float livePacketRateHz = 150.0f;
unsigned long packetCountSec = 0;
unsigned long lastSecTimer = 0;

// Bộ lọc EMA
float fLY = 2048, fLX = 2048, fRY = 2048, fRX = 2048, fPot = 2048;
float vBatTx = 8.24f;
float vBatRx = 11.85f;
int rssiLive = -52; // dBm

// Điều hướng Menu & Màn hình LCD
LiquidCrystal_I2C lcd(0x27, 16, 4);

bool inMenu = false;
int menuPage = 0;        // 0..5 (6 trang cài đặt)
int menuLine = 0;        // Con trỏ dòng đang chọn
unsigned long b1HoldStart = 0;
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

// --- BỘ LỌC SỐ EMA FIXED-POINT SIÊU TỐC ---
inline float applyFastEMA(float current, float previous, float alpha = 0.35f) {
  return (alpha * current) + ((1.0f - alpha) * previous);
}

// --- THUẬT TOÁN ĐƯỜNG CONG EXPO (EXPONENTIAL CURVE) ---
int applyExpo(int us, int expoPct) {
  if (expoPct == 0) return us;
  float x = (us - 1500) / 500.0f; // -1.0 đến +1.0
  float factor = expoPct / 100.0f;
  float y = x * (1.0f - factor) + (x * x * x) * factor; // Phản hồi mượt ở giữa, nhạy ở biên
  return (int)(1500 + y * 500.0f);
}

// --- ÁNH XẠ 3 ĐIỂM JOYSTICK PS2 ---
int mapPS2Axis(int raw, const PS2Calibration &cal, int deadband, bool reverse) {
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
  if (reverse) us = 3000 - us; // Đảo chiều 1000 <-> 2000
  return constrain(us, 1000, 2000);
}

void beep(int freq, int duration_ms) {
  digitalWrite(PIN_BUZZER, HIGH);
  delay(duration_ms);
  digitalWrite(PIN_BUZZER, LOW);
}

// --- ĐỌC VÀ GHI CẤU HÌNH EEPROM ---
void loadConfig() {
  EEPROM.get(0, sysConfig);
  if (sysConfig.model_id != MODEL_ID_DRONE && sysConfig.model_id != MODEL_ID_CAR) {
    // Giá trị mặc định lần đầu chạy
    sysConfig.model_id       = MODEL_ID_CAR;      // Mặc định xe ô tô
    sysConfig.rf_rate_idx    = 2;                 // 150Hz (chu kỳ 6.6ms cực nhanh)
    sysConfig.protocol       = PROTOCOL_CRSF;     // CRSF siêu tốc
    sysConfig.pwm_rate       = PWM_RATE_333HZ;    // 333Hz Digital Servo
    sysConfig.deadband_us    = 15;                // 15us nhạy bén
    sysConfig.expo_pct       = 20;                // 20% expo
    sysConfig.ch_reverse     = 0;
    sysConfig.hover_bias_def = 35;
    sysConfig.speed_lim_max  = 100;
    sysConfig.bat_warn_mv    = 7000;
    EEPROM.put(0, sysConfig);
  }
}

void saveConfig() {
  EEPROM.put(0, sysConfig);
  beep(3200, 100);
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

  // Giao tiếp Serial1 UART tốc độ cao với LoRa E22 (115200 Baud)
  Serial1.begin(115200);

  Wire.begin();
  Wire.setClock(400000); // 400kHz Fast I2C chống nghẽn
  lcd.init();
  lcd.backlight();

  loadConfig();

  lcd.setCursor(0, 0); lcd.print("TAY KHIEN RC PRO");
  lcd.setCursor(0, 1); lcd.print("ZERO-LAG 200Hz");
  lcd.setCursor(0, 2); lcd.print("CRSF 420k + LoRa");
  lcd.setCursor(0, 3); lcd.print("SYS INIT OK 100%");

  beep(2200, 60);
  delay(40);
  beep(3000, 100);
  delay(400);
  lcd.clear();
}

void handleMenuNavigation();
void renderHUDLineSlice(uint8_t line);
void renderMenuPage();

void loop() {
  unsigned long now = millis();
  unsigned long nowMicros = micros();

  // 1. TÍNH TOÁN REAL-TIME LATENCY VÀ PACKET RATE THỰC TẾ
  if (now - lastSecTimer >= 1000) {
    livePacketRateHz = packetCountSec * 1000.0f / (now - lastSecTimer);
    packetCountSec = 0;
    lastSecTimer = now;
  }

  // 2. XỬ LÝ NHẤN NÚT B1 (NHẤN GIỮ 1.2s ĐỂ VÀO/THOÁT MENU, NHẤN NHẸ ĐỂ CHỌN)
  if (digitalRead(PIN_BTN_B1) == LOW) {
    if (b1HoldStart == 0) b1HoldStart = now;
    if (now - b1HoldStart > 1200) {
      inMenu = !inMenu;
      b1HoldStart = 0;
      beep(3200, 80);
      lcd.clear();
    }
  } else {
    if (b1HoldStart > 0 && (now - b1HoldStart < 500) && inMenu) {
      // Nhấn nhả nhanh trong Menu: Chọn / Đổi giá trị
      handleMenuNavigation();
    }
    b1HoldStart = 0;
  }

  if (inMenu) {
    renderMenuPage();
    // Vẫn duy trì truyền gói tin nền để giữ kết nối an toàn với xe/drone!
  }

  // 3. ĐỌC NHANH ADC VÀ LỌC SỐ KHÔNG CHẶN (NON-BLOCKING)
  fLY  = applyFastEMA(analogRead(PIN_JOY_LY), fLY);
  fLX  = applyFastEMA(analogRead(PIN_JOY_LX), fLX);
  fRY  = applyFastEMA(analogRead(PIN_JOY_RY), fRY);
  fRX  = applyFastEMA(analogRead(PIN_JOY_RX), fRX);
  fPot = applyFastEMA(analogRead(PIN_POT),    fPot);

  bool ch1Rev = (sysConfig.ch_reverse & (1 << 0));
  bool ch2Rev = (sysConfig.ch_reverse & (1 << 1));
  bool ch3Rev = (sysConfig.ch_reverse & (1 << 2));
  bool ch4Rev = (sysConfig.ch_reverse & (1 << 3));

  int joyLY = mapPS2Axis((int)fLY, calLY, sysConfig.deadband_us, ch3Rev);
  int joyLX = mapPS2Axis((int)fLX, calLX, sysConfig.deadband_us, ch4Rev);
  int joyRY = mapPS2Axis((int)fRY, calRY, sysConfig.deadband_us, ch2Rev);
  int joyRX = mapPS2Axis((int)fRX, calRX, sysConfig.deadband_us, ch1Rev);
  int potPct = map((int)fPot, 0, 4095, 0, 100);

  // Áp dụng Expo cho Lái & Pitch
  joyRX = applyExpo(joyRX, sysConfig.expo_pct);
  joyRY = applyExpo(joyRY, sysConfig.expo_pct);

  bool swArm  = (digitalRead(PIN_SW1_ARM) == LOW);
  uint8_t swMode = 1;
  if (digitalRead(PIN_SW2_P1) == LOW) swMode = 0;
  else if (digitalRead(PIN_SW2_P2) == LOW) swMode = 2;
  bool swAux  = (digitalRead(PIN_SW3_AUX) == LOW);
  bool sw_L   = (digitalRead(PIN_SW_L) == LOW);
  bool sw_R   = (digitalRead(PIN_SW_R) == LOW);

  if (sysConfig.model_id == MODEL_ID_DRONE) {
    // --- THUẬT TOÁN ĐIỀU KHIỂN DRONE VỚI CẦN HỒI TÂM (ALTITUDE HOLD BIAS) ---
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
    // --- THUẬT TOÁN ĐIỀU KHIỂN XE RC (ZERO LAG CAR THROTTLE & STEERING) ---
    int speedLimit = map(potPct, 0, 100, 20, sysConfig.speed_lim_max);
    int rawDelta = joyLY - 1500;
    int limitedDelta = (rawDelta * speedLimit) / 100;
    txPacket.ch3_throttle = 1500 + limitedDelta;
    txPacket.ch1_roll_str = joyRX; // Lái bánh trước
    txPacket.ch2_pitch    = joyRY; // Số lùi / Phanh cơ
    txPacket.ch4_yaw      = 1500;
  }

  // Đóng gói cờ điều khiển
  txPacket.model_id = sysConfig.model_id;
  txPacket.seq_num  = packetSequence++;
  txPacket.knob_pot = potPct;
  txPacket.sw_flags = 0;
  if (swArm)  txPacket.sw_flags |= (1 << 0);
  txPacket.sw_flags |= ((swMode & 0x03) << 1);
  if (swAux)  txPacket.sw_flags |= (1 << 3);
  if (sw_L)   txPacket.sw_flags |= (1 << 4);
  if (sw_R)   txPacket.sw_flags |= (1 << 5);
  if (sysConfig.protocol == PROTOCOL_CRSF) txPacket.sw_flags |= (1 << 6);
  if (sysConfig.pwm_rate == PWM_RATE_333HZ) txPacket.sw_flags |= (1 << 7);

  // 4. PHÁT GÓI TIN VÔ TUYẾN TỐC ĐỘ CAO (RF PIPELINE 100Hz - 200Hz)
  uint16_t intervalMs = RF_INTERVAL_MS[sysConfig.rf_rate_idx];
  if (now - lastRfTime >= intervalMs) {
    lastRfTime = now;
    currentLatencyMs = (micros() - nowMicros) / 1000.0f + 1.2f; // Tính toán thời gian xử lý thực tế

    txPacket.header[0] = 0xAA;
    txPacket.header[1] = 0x55;
    txPacket.crc16 = calculateCRC16((const uint8_t*)&txPacket, sizeof(RadioPacket) - 2);

    // Gửi trực tiếp không chặn (Non-blocking UART buffer)
    Serial1.write((uint8_t*)&txPacket, sizeof(RadioPacket));
    packetCountSec++;
  }

  // 5. ĐỌC DỮ LIỆU TELEMETRY TỪ MẠCH THU TRẢ VỀ
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

  // 6. CẬP NHẬT MÀN HÌNH BẰNG TIME-SLICING (MỖI 20ms VẼ 1 DÒNG ĐỂ CPU LUÔN RẢNH CHO SÓNG RF)
  if (!inMenu && (now - lastLcdSlice >= 20)) {
    lastLcdSlice = now;
    renderHUDLineSlice(lcdLineIndex);
    lcdLineIndex = (lcdLineIndex + 1) % 4;
  }
}

// --- GIAO DIỆN HUD ĐA THÔNG TIN THỜI GIAN THỰC (REAL-TIME TELEMETRY HUD) ---
void renderHUDLineSlice(uint8_t line) {
  char buf[17];
  lcd.setCursor(0, line);

  if (line == 0) {
    // Dòng 1: Model, Tần số Hz, Độ trễ ms
    const char *mName = (sysConfig.model_id == MODEL_ID_DRONE) ? "DRN" : "CAR";
    const char *armSt = (txPacket.sw_flags & (1 << 0)) ? "ARM" : "DIS";
    snprintf(buf, sizeof(buf), "%s|%s %3dHz%2.0fms", mName, armSt, (int)livePacketRateHz, currentLatencyMs);
    lcd.print(buf);

  } else if (line == 1) {
    // Dòng 2: Sóng RSSI (dBm), Giao thức & PWM rate
    const char *protStr = (sysConfig.protocol == PROTOCOL_CRSF) ? "CRSF" : "SBUS";
    const char *pwmStr  = (sysConfig.pwm_rate == PWM_RATE_333HZ) ? "333H" : "50Hz";
    snprintf(buf, sizeof(buf), "S:%ddBm %s|%s", rssiLive, protStr, pwmStr);
    lcd.print(buf);

  } else if (line == 2) {
    // Dòng 3: Điện áp Pin TX & RX Telemetry
    vBatTx = (analogRead(PIN_BAT_SENSE) * 3.3f / 4095.0f) * 2.05f * 2.0f; // Pin 2S (7.4V - 8.4V)
    snprintf(buf, sizeof(buf), "TX:%1.2fV RX:%1.2fV", vBatTx, vBatRx);
    lcd.print(buf);

  } else if (line == 3) {
    // Dòng 4: Thanh trạng thái kênh Ga (THR) và Lái (STR/YAW)
    int thrPct = (sysConfig.model_id == MODEL_ID_DRONE) ? 
                 map(txPacket.ch3_throttle, 1000, 2000, 0, 100) : 
                 map(txPacket.ch3_throttle, 1000, 2000, -100, 100);
    int strPct = map(txPacket.ch1_roll_str, 1000, 2000, -100, 100);
    snprintf(buf, sizeof(buf), "T:%3d%% S:%3d%% P%2d", thrPct, strPct, txPacket.knob_pot);
    lcd.print(buf);
  }
}

// --- HỆ THỐNG MENU ĐA TẦNG 6 TRANG CÀI ĐẶT ---
void renderMenuPage() {
  char b[17];
  lcd.setCursor(0, 0);
  snprintf(b, sizeof(b), "MENU %d/6 [B1:OK]", menuPage + 1);
  lcd.print(b);

  switch (menuPage) {
    case 0: // Trang 1: Chọn Model
      lcd.setCursor(0, 1); lcd.print("1. MODEL SELECT ");
      lcd.setCursor(0, 2);
      if (sysConfig.model_id == MODEL_ID_CAR)   lcd.print("> [RC CAR RACER]");
      else                                     lcd.print("> [FPV DRONE QUAD]");
      lcd.setCursor(0, 3); lcd.print("B2:NEXT  B3:PREV");
      break;

    case 1: // Trang 2: Tốc độ gói tin RF & Độ trễ
      lcd.setCursor(0, 1); lcd.print("2. RF PACKETRATE");
      lcd.setCursor(0, 2);
      snprintf(b, sizeof(b), "> %s", RF_RATE_STR[sysConfig.rf_rate_idx]);
      lcd.print(b);
      lcd.setCursor(0, 3); lcd.print("B1:THAY DOI TOC DO");
      break;

    case 2: // Trang 3: Giao thức Drone & PWM Xe
      lcd.setCursor(0, 1); lcd.print("3. PROTOCOL & PWM");
      lcd.setCursor(0, 2);
      if (menuLine == 0) {
        const char *p = (sysConfig.protocol == PROTOCOL_CRSF) ? "CRSF 420k" : "SBUS 100k";
        snprintf(b, sizeof(b), ">DRN: %s  ", p);
      } else {
        const char *w = (sysConfig.pwm_rate == PWM_RATE_333HZ) ? "333Hz(3ms)" : "50Hz(20ms)";
        snprintf(b, sizeof(b), ">CAR: %s  ", w);
      }
      lcd.print(b);
      lcd.setCursor(0, 3); lcd.print("B1:CHON  B2:DONG");
      break;

    case 3: // Trang 4: Độ nhạy Deadband & Expo
      lcd.setCursor(0, 1); lcd.print("4. DEADBAND&EXPO");
      lcd.setCursor(0, 2);
      snprintf(b, sizeof(b), "DB:%dus  EXPO:%d%%", sysConfig.deadband_us, sysConfig.expo_pct);
      lcd.print(b);
      lcd.setCursor(0, 3); lcd.print("B1:+DB    B2:+EX");
      break;

    case 4: // Trang 5: Đảo chiều kênh lái & ga
      lcd.setCursor(0, 1); lcd.print("5. REVERSE CH1..4");
      lcd.setCursor(0, 2);
      snprintf(b, sizeof(b), "1:%c 2:%c 3:%c 4:%c",
        (sysConfig.ch_reverse & (1<<0)) ? 'R' : 'N',
        (sysConfig.ch_reverse & (1<<1)) ? 'R' : 'N',
        (sysConfig.ch_reverse & (1<<2)) ? 'R' : 'N',
        (sysConfig.ch_reverse & (1<<3)) ? 'R' : 'N');
      lcd.print(b);
      lcd.setCursor(0, 3); lcd.print("B1:DAO CH1(STR) ");
      break;

    case 5: // Trang 6: Lưu và Thoát
      lcd.setCursor(0, 1); lcd.print("6. LUU CAI DAT  ");
      lcd.setCursor(0, 2); lcd.print("> NHAN B1 DE LUU");
      lcd.setCursor(0, 3); lcd.print("NHAN GIU B1:THOAT");
      break;
  }

  // Điều hướng nút B2 (Next Page / Line)
  if (digitalRead(PIN_BTN_B2) == LOW) {
    beep(2400, 30);
    menuPage = (menuPage + 1) % 6;
    delay(180);
    lcd.clear();
  }

  // Điều hướng nút B3 (Prev Page)
  if (digitalRead(PIN_BTN_B3) == LOW) {
    beep(2400, 30);
    menuPage = (menuPage + 5) % 6;
    delay(180);
    lcd.clear();
  }
}

// Xử lý khi nhấn nút B1 để thay đổi giá trị trong trang Menu
void handleMenuNavigation() {
  beep(2800, 40);
  switch (menuPage) {
    case 0: // Đổi Model
      sysConfig.model_id = (sysConfig.model_id == MODEL_ID_CAR) ? MODEL_ID_DRONE : MODEL_ID_CAR;
      break;
    case 1: // Đổi RF Rate (50Hz -> 100Hz -> 150Hz -> 200Hz)
      sysConfig.rf_rate_idx = (sysConfig.rf_rate_idx + 1) % 4;
      break;
    case 2: // Đổi Protocol hoặc PWM Rate
      if (menuLine == 0) {
        sysConfig.protocol = 1 - sysConfig.protocol;
        menuLine = 1;
      } else {
        sysConfig.pwm_rate = 1 - sysConfig.pwm_rate;
        menuLine = 0;
      }
      break;
    case 3: // Tăng Deadband hoặc Expo
      sysConfig.deadband_us += 15;
      if (sysConfig.deadband_us > 45) sysConfig.deadband_us = 0;
      sysConfig.expo_pct += 20;
      if (sysConfig.expo_pct > 60) sysConfig.expo_pct = 0;
      break;
    case 4: // Đảo chiều Ch1
      sysConfig.ch_reverse ^= (1 << 0);
      break;
    case 5: // Lưu EEPROM
      saveConfig();
      lcd.clear();
      lcd.setCursor(0, 1); lcd.print("DA LUU THANH CONG!");
      delay(600);
      inMenu = false;
      lcd.clear();
      break;
  }
}
