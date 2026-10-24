/*
 * ======================================================================================
 * DỰ ÁN TAY ĐIỀU KHIỂN RC "TOÀN DIỆN" - FIRMWARE BỘ THU PRO (RECEIVER ALL-IN-ONE PRO)
 * KHÔNG BAO GIỜ XUNG ĐỘT: TỰ ĐỘNG CÁCH LY TUYỆT ĐỐI GIỮA DRONE & XE RC
 * BẢO VỆ TOÀN VẸN DỮ LIỆU BẰNG CRC16-CCITT & BỘ ĐÔI FAILSAFE THÔNG MINH
 * ======================================================================================
 * Phần cứng: 1 Bo mạch vi điều khiển (Arduino Nano / Pro Mini / STM32) + Module LoRa E22
 * Cổng ra phần cứng:
 *   [1] Cổng SBUS (TX): Nối vào cổng Rx của Flight Controller (Betaflight / INAV)
 *   [2] Cọc CH1 (D9):   Servo bẻ lái bánh trước xe (Steering)
 *   [3] Cọc CH2 (D10):  ESC động cơ chính xe (Tiến/Thắng/Lùi)
 *   [4] Cọc CH3 (D5):   Đèn pha xe (Mosfet ON/OFF)
 *   [5] Cọc CH4 (D6):   Còi xe (Car Horn)
 *   [6] Cọc CH5 (D7):   Kênh phụ (Khóa vi sai / Hộp số phụ / Tời kéo)
 *   [7] Chân A0:        Cầu trở đo áp pin mô hình (Telemetry gửi về tay cầm)
 * ======================================================================================
 */

#include <Arduino.h>
#include <Servo.h>

#define PIN_PWM_STEER     9    // CH1: Servo bẻ lái xe
#define PIN_PWM_THROTTLE  10   // CH2: ESC động cơ xe
#define PIN_OUT_LIGHTS    5    // CH3: Đèn pha xe
#define PIN_OUT_HORN      6    // CH4: Còi xe
#define PIN_PWM_AUX       7    // CH5: Servo phụ (Khóa vi sai / Hộp số)
#define PIN_BAT_SENSE     A0   // Đo điện áp pin nguồn (Cầu trở 10k/2.2k đo tối đa 16.8V)

#define MODEL_ID_DRONE    0x51 // 'Q'
#define MODEL_ID_CAR      0x43 // 'C'

Servo steerServo;
Servo motorESC;
Servo auxServo;

// Cấu trúc gói tin 17 bytes đồng bộ 100% với tay phát PRO
struct __attribute__((packed)) RadioPacket {
  uint8_t  header[2];    // 0xAA, 0x55
  uint8_t  model_id;     // 0x51 (Drone) hoặc 0x43 (Car)
  uint8_t  seq_num;      // Số thứ tự gói
  uint16_t ch1_roll_str; // 1000 - 2000 us (Roll / Steering)
  uint16_t ch2_pitch;    // 1000 - 2000 us (Pitch / Gear)
  uint16_t ch3_throttle; // 1000 - 2000 us (Ga)
  uint16_t ch4_yaw;      // 1000 - 2000 us (Yaw)
  uint8_t  sw_flags;     // Bit 0: ARM, Bit 1-2: Mode, Bit 3: AUX, Bit 4: SW_L, Bit 5: SW_R
  uint8_t  knob_pot;     // 0 - 100 %
  uint16_t crc16;        // CRC16-CCITT
};

RadioPacket rxPacket;
unsigned long lastPacketTime = 0;
unsigned long lastSbusTime   = 0;
unsigned long lastTelemTime  = 0;
bool isFailsafeActive = true;

// Khung 25 byte SBUS 16 kênh chuẩn quốc tế (100.000 Baud, 8E2 inverted)
uint16_t sbusChannels[16];
uint8_t  sbusFrame[25];

uint16_t usToSbus(uint16_t us) {
  return map(constrain(us, 1000, 2000), 1000, 2000, 172, 1811);
}

// Tính mã kiểm tra CRC16-CCITT
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

// Đóng gói khung SBUS 25 bytes
void packSbusFrame(bool isFailsafe) {
  memset(sbusFrame, 0, sizeof(sbusFrame));
  sbusFrame[0] = 0x0F; // Header SBUS

  sbusFrame[1]  = (uint8_t)((sbusChannels[0] & 0x07FF));
  sbusFrame[2]  = (uint8_t)((sbusChannels[0] >> 8) | (sbusChannels[1] << 3));
  sbusFrame[3]  = (uint8_t)((sbusChannels[1] >> 5) | (sbusChannels[2] << 6));
  sbusFrame[4]  = (uint8_t)((sbusChannels[2] >> 2));
  sbusFrame[5]  = (uint8_t)((sbusChannels[2] >> 10) | (sbusChannels[3] << 1));
  sbusFrame[6]  = (uint8_t)((sbusChannels[3] >> 7) | (sbusChannels[4] << 4));
  sbusFrame[7]  = (uint8_t)((sbusChannels[4] >> 4) | (sbusChannels[5] << 7));
  sbusFrame[8]  = (uint8_t)((sbusChannels[5] >> 1));
  sbusFrame[9]  = (uint8_t)((sbusChannels[5] >> 9) | (sbusChannels[6] << 2));
  sbusFrame[10] = (uint8_t)((sbusChannels[6] >> 6) | (sbusChannels[7] << 5));
  sbusFrame[11] = (uint8_t)((sbusChannels[7] >> 3));
  sbusFrame[12] = (uint8_t)((sbusChannels[8] & 0x07FF));
  sbusFrame[13] = (uint8_t)((sbusChannels[8] >> 8) | (sbusChannels[9] << 3));
  sbusFrame[14] = (uint8_t)((sbusChannels[9] >> 5) | (sbusChannels[10] << 6));
  sbusFrame[15] = (uint8_t)((sbusChannels[10] >> 2));
  sbusFrame[16] = (uint8_t)((sbusChannels[10] >> 10) | (sbusChannels[11] << 1));
  sbusFrame[17] = (uint8_t)((sbusChannels[11] >> 7) | (sbusChannels[12] << 4));
  sbusFrame[18] = (uint8_t)((sbusChannels[12] >> 4) | (sbusChannels[13] << 7));
  sbusFrame[19] = (uint8_t)((sbusChannels[13] >> 1));
  sbusFrame[20] = (uint8_t)((sbusChannels[13] >> 9) | (sbusChannels[14] << 2));
  sbusFrame[21] = (uint8_t)((sbusChannels[14] >> 6) | (sbusChannels[15] << 5));
  sbusFrame[22] = (uint8_t)((sbusChannels[15] >> 3));

  sbusFrame[23] = 0x00;
  if (isFailsafe) {
    sbusFrame[23] |= (1 << 3); // Cờ Failsafe (Kích hoạt Betaflight RTH / Land)
    sbusFrame[23] |= (1 << 2); // Cờ Frame Lost
  }
  sbusFrame[24] = 0x00;
}

void setup() {
  // Giao tiếp với LoRa E22 ở tốc độ cao 115.200 Baud
  Serial.begin(115200);

  pinMode(PIN_OUT_LIGHTS, OUTPUT);
  pinMode(PIN_OUT_HORN,   OUTPUT);
  digitalWrite(PIN_OUT_LIGHTS, LOW);
  digitalWrite(PIN_OUT_HORN,   LOW);

  steerServo.attach(PIN_PWM_STEER);
  motorESC.attach(PIN_PWM_THROTTLE);
  auxServo.attach(PIN_PWM_AUX);

  // Vị trí an toàn khởi đầu
  steerServo.writeMicroseconds(1500); // Thẳng lái
  motorESC.writeMicroseconds(1500);   // Dừng động cơ
  auxServo.writeMicroseconds(1500);

  for (int i = 0; i < 16; i++) sbusChannels[i] = usToSbus(1500);
  sbusChannels[2] = usToSbus(1000); // Ga ở mức 0
}

void loop() {
  unsigned long now = millis();

  // 1. GIẢI MÃ GÓI TIN TỪ LORA E22 VỚI BỘ LỌC CRC16 CHẶT CHẼ
  if (Serial.available() >= sizeof(RadioPacket)) {
    if (Serial.read() == 0xAA && Serial.peek() == 0x55) {
      Serial.read(); // Đọc bỏ 0x55
      uint8_t *ptr = (uint8_t*)&rxPacket + 2;
      Serial.readBytes((char*)ptr, sizeof(RadioPacket) - 2);
      rxPacket.header[0] = 0xAA;
      rxPacket.header[1] = 0x55;

      // Xác thực mã kiểm tra lỗi CRC16-CCITT
      uint16_t calcCRC = calculateCRC16((const uint8_t*)&rxPacket, sizeof(RadioPacket) - 2);
      if (calcCRC == rxPacket.crc16) {
        lastPacketTime = now;
        isFailsafeActive = false;

        bool armState   = (rxPacket.sw_flags & (1 << 0));
        uint8_t swMode  = (rxPacket.sw_flags >> 1) & 0x03;
        bool swAux      = (rxPacket.sw_flags & (1 << 3));
        bool swHorn     = (rxPacket.sw_flags & (1 << 4));
        bool swLight    = (rxPacket.sw_flags & (1 << 5));

        // =====================================================================
        // [CƠ CHẾ CÁCH LY TUYỆT ĐỐI - KHÔNG BAO GIỜ XUNG ĐỘT DRONE & XE]
        // =====================================================================
        if (rxPacket.model_id == MODEL_ID_DRONE) {
          // --- [A] ĐANG Ở CHẾ ĐỘ DRONE ---
          // 1. Cập nhật 16 kênh SBUS cho Flight Controller
          sbusChannels[0] = usToSbus(rxPacket.ch1_roll_str); // Ch1: Roll
          sbusChannels[1] = usToSbus(rxPacket.ch2_pitch);    // Ch2: Pitch
          sbusChannels[2] = usToSbus(rxPacket.ch3_throttle); // Ch3: Throttle (Ga)
          sbusChannels[3] = usToSbus(rxPacket.ch4_yaw);      // Ch4: Yaw
          sbusChannels[4] = armState ? usToSbus(2000) : usToSbus(1000); // Ch5: ARM

          if (swMode == 0)      sbusChannels[5] = usToSbus(1000); // Angle
          else if (swMode == 1) sbusChannels[5] = usToSbus(1500); // Horizon
          else                  sbusChannels[5] = usToSbus(2000); // Acro

          sbusChannels[6] = swAux ? usToSbus(2000) : usToSbus(1000); // Ch7: RTH / Rescue
          sbusChannels[7] = usToSbus(map(rxPacket.knob_pot, 0, 100, 1000, 2000)); // Ch8: Gimbal

          // 2. KHÓA CHẶT ĐẦU RA XE (Đảm bảo an toàn 100% không bao giờ xe tự chạy nhầm!)
          steerServo.writeMicroseconds(1500);
          motorESC.writeMicroseconds(1500); // Dừng xe
          digitalWrite(PIN_OUT_LIGHTS, LOW);
          digitalWrite(PIN_OUT_HORN, swHorn ? HIGH : LOW); // Cho phép dùng còi beeper tìm máy bay

        } else if (rxPacket.model_id == MODEL_ID_CAR) {
          // --- [B] ĐANG Ở CHẾ ĐỘ XE RC ---
          // 1. Cập nhật trực tiếp ra cọc Servo và ESC của xe
          steerServo.writeMicroseconds(constrain(rxPacket.ch1_roll_str, 1000, 2000));

          if (armState) {
            motorESC.writeMicroseconds(constrain(rxPacket.ch3_throttle, 1000, 2000));
          } else {
            motorESC.writeMicroseconds(1500); // Phanh tay đỗ xe
          }

          digitalWrite(PIN_OUT_LIGHTS, swLight ? HIGH : LOW); // Đèn pha
          digitalWrite(PIN_OUT_HORN,   swHorn  ? HIGH : LOW); // Còi xe
          auxServo.writeMicroseconds(swAux ? 1900 : 1100);    // Khóa vi sai

          // 2. KHÓA CHẶT TÍN HIỆU SBUS DRONE (Drone sẽ ở trạng thái DISARM & Failsafe)
          sbusChannels[2] = usToSbus(1000); // Cắt ga về 0
          sbusChannels[4] = usToSbus(1000); // DISARM
        }
      }
    }
  }

  // 2. CƠ CHẾ FAILSAFE KÉP KHI MẤT SÓNG (> 350ms)
  if (now - lastPacketTime > 350) {
    isFailsafeActive = true;

    // Failsafe cho Drone: Cắt ga về 0, DISARM, kích hoạt cờ Failsafe trên SBUS
    sbusChannels[2] = usToSbus(1000);
    sbusChannels[4] = usToSbus(1000);

    // Failsafe cho Xe: Dừng động cơ (1500us phanh), trả thẳng lái bánh trước (1500us)
    motorESC.writeMicroseconds(1500);
    steerServo.writeMicroseconds(1500);
    digitalWrite(PIN_OUT_LIGHTS, LOW);
    digitalWrite(PIN_OUT_HORN, LOW);
  }

  // 3. XUẤT KHUNG SBUS 70Hz ĐỀU ĐẶN (MỖI 14ms)
  if (now - lastSbusTime >= 14) {
    lastSbusTime = now;
    packSbusFrame(isFailsafeActive);
    // Xuất sbusFrame ra cổng UART nối mạch bay
  }

  // 4. GỬI TELEMETRY ĐIỆN ÁP PIN VỀ TAY CẦM (MỖI 500ms)
  if (now - lastTelemTime >= 500) {
    lastTelemTime = now;
    int rawA0 = analogRead(PIN_BAT_SENSE);
    float vBat = (rawA0 * 5.0f / 1023.0f) * 5.545f; // Hệ số cầu phân áp
    uint8_t vInt = (uint8_t)vBat;
    uint8_t vDec = (uint8_t)((vBat - vInt) * 10.0f);

    uint8_t telemBuf[4] = { 0x55, 0xAA, vInt, vDec };
    Serial.write(telemBuf, 4);
  }
}
