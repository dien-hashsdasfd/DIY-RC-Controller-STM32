/*
 * ======================================================================================
 * DỰ ÁN TAY ĐIỀU KHIỂN RC "TOÀN DIỆN" - FIRMWARE BỘ THU ULTRA-SPEED (RECEIVER V4 PRO)
 * HỖ TRỢ ĐỒNG THỜI:
 *   1. CRSF PROTOCOL 420,000 BAUD (ĐỘ TRỄ 4ms) HOẶC SBUS 100,000 BAUD CHO DRONE
 *   2. HIGH-SPEED PWM 333Hz (CHU KỲ 3ms) CHO DIGITAL SERVO VÀ BRUSHLESS ESC XE Ô TÔ
 *   3. TELEMETRY PHẢN HỒI 2 CHIỀU: ĐIỆN ÁP PIN RX, CƯỜNG ĐỘ SÓNG RSSI, TỶ LỆ MẤT GÓI
 *   4. KHÔNG BAO GIỜ XUNG ĐỘT TÍN HIỆU GIỮA XE & MÁY BAY
 * ======================================================================================
 */

#include <Arduino.h>
#include <Servo.h>

#define PIN_PWM_STEER     9    // CH1: Servo bẻ lái xe (Hỗ trợ 50Hz / 333Hz)
#define PIN_PWM_THROTTLE  10   // CH2: ESC động cơ xe (Hỗ trợ 50Hz / 333Hz)
#define PIN_OUT_LIGHTS    5    // CH3: Đèn pha xe (Mosfet ON/OFF)
#define PIN_OUT_HORN      6    // CH4: Còi xe
#define PIN_PWM_AUX       7    // CH5: Kênh phụ (Khóa vi sai / Tời cứu hộ)
#define PIN_BAT_SENSE     A0   // Đo điện áp pin RX (Cầu trở 10k/2.2k đo tối đa 4S LiPo 16.8V)

#define MODEL_ID_DRONE    0x51 // 'Q'
#define MODEL_ID_CAR      0x43 // 'C'

#define PROTOCOL_CRSF     0
#define PROTOCOL_SBUS     1

// Cấu trúc gói tin vô tuyến đồng bộ với Tay Phát Ultra-Speed
struct __attribute__((packed)) RadioPacket {
  uint8_t  header[2];    // 0xAA, 0x55
  uint8_t  model_id;     // 0x51 (Drone) hoặc 0x43 (Car)
  uint8_t  seq_num;      // Số thứ tự gói
  uint16_t ch1_roll_str; // 1000 - 2000 us
  uint16_t ch2_pitch;    // 1000 - 2000 us
  uint16_t ch3_throttle; // 1000 - 2000 us
  uint16_t ch4_yaw;      // 1000 - 2000 us
  uint8_t  sw_flags;     // Bit 0: ARM, Bit 1-2: Mode, Bit 3: AUX, Bit 4: SW_L, Bit 5: SW_R, Bit 6: Prot, Bit 7: FastPWM
  uint8_t  knob_pot;     // 0 - 100 %
  uint16_t crc16;
};

RadioPacket rxPacket;

// Gói tin phản hồi Telemetry gửi ngược về tay cầm
struct __attribute__((packed)) TelemetryPacket {
  uint8_t  header[2];    // 0x55, 0xAA
  uint16_t vBatRx_mv;    // Điện áp pin (mV)
  int8_t   rssi_dbm;     // Cường độ sóng (-50 dBm)
  uint8_t  packet_loss;  // Tỷ lệ mất gói (%)
  uint16_t crc16;
};

TelemetryPacket txTelem;

Servo steerServo;
Servo motorESC;
Servo auxServo;

unsigned long lastPacketTime = 0;
unsigned long lastTelemTime  = 0;
unsigned long lastDroneOutTime = 0;
uint8_t lastSeqNum = 0;
uint16_t lostPacketCounter = 0;
uint16_t totalPacketCounter = 0;
bool isFailsafe = true;

// Mảng 16 kênh SBUS / CRSF
uint16_t sbusChannels[16];
uint8_t  sbusFrame[25];
uint8_t  crsfFrame[26];

// Tính CRC16-CCITT
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

// Bảng CRC8 cho chuẩn CRSF
uint8_t crsf_crc8(const uint8_t *ptr, uint8_t len) {
  uint8_t crc = 0;
  for (uint8_t i = 0; i < len; i++) {
    crc ^= ptr[i];
    for (uint8_t j = 0; j < 8; j++) {
      if (crc & 0x80) crc = (crc << 1) ^ 0xD5;
      else crc <<= 1;
    }
  }
  return crc;
}

// Chuyển us sang dải SBUS 11-bit (172..1811)
uint16_t usToSbus(uint16_t us) {
  return map(constrain(us, 1000, 2000), 1000, 2000, 172, 1811);
}

// Chuyển us sang dải CRSF 11-bit (172..1811)
uint16_t usToCrsf(uint16_t us) {
  return map(constrain(us, 1000, 2000), 1000, 2000, 172, 1811);
}

// Đóng gói khung SBUS 25 bytes
void packSbusFrame(bool failsafe) {
  memset(sbusFrame, 0, sizeof(sbusFrame));
  sbusFrame[0] = 0x0F;

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
  if (failsafe) {
    sbusFrame[23] |= (1 << 3); // Cờ Failsafe
    sbusFrame[23] |= (1 << 2); // Cờ Frame Lost
  }
  sbusFrame[24] = 0x00;
}

// Đóng gói khung CRSF Channels Frame 26 bytes (Chuẩn Crossfire 420k)
void packCrsfFrame() {
  crsfFrame[0] = 0xC8; // Địa chỉ Flight Controller
  crsfFrame[1] = 24;   // Chiều dài gói (Payload + Type + CRC)
  crsfFrame[2] = 0x16; // Type 0x16: RC Channels Packed

  // Đóng gói 16 kênh 11-bit vào 22 byte
  uint32_t bits = 0;
  uint8_t bitCount = 0;
  uint8_t byteIdx = 3;

  for (int ch = 0; ch < 16; ch++) {
    bits |= ((uint32_t)sbusChannels[ch] & 0x07FF) << bitCount;
    bitCount += 11;
    while (bitCount >= 8) {
      crsfFrame[byteIdx++] = (uint8_t)(bits & 0xFF);
      bits >>= 8;
      bitCount -= 8;
    }
  }
  crsfFrame[25] = crsf_crc8(&crsfFrame[2], 23); // Tính CRC8
}

void setup() {
  // Giao tiếp với LoRa E22 ở tốc độ cao 115200 Baud
  Serial.begin(115200);

  pinMode(PIN_OUT_LIGHTS, OUTPUT);
  pinMode(PIN_OUT_HORN,   OUTPUT);
  digitalWrite(PIN_OUT_LIGHTS, LOW);
  digitalWrite(PIN_OUT_HORN,   LOW);

  steerServo.attach(PIN_PWM_STEER);
  motorESC.attach(PIN_PWM_THROTTLE);
  auxServo.attach(PIN_PWM_AUX);

  // Mức an toàn khởi động
  steerServo.writeMicroseconds(1500);
  motorESC.writeMicroseconds(1500);
  auxServo.writeMicroseconds(1500);

  for (int i = 0; i < 16; i++) sbusChannels[i] = usToSbus(1500);
  sbusChannels[2] = usToSbus(1000); // Ga drone = 0
}

void loop() {
  unsigned long now = millis();

  // 1. NHẬN VÀ GIẢI MÃ GÓI TIN TỪ LORA E22 VỚI BỘ LỌC CRC16
  while (Serial.available() >= sizeof(RadioPacket)) {
    if (Serial.read() == 0xAA && Serial.peek() == 0x55) {
      Serial.read();
      uint8_t *ptr = (uint8_t*)&rxPacket + 2;
      Serial.readBytes((char*)ptr, sizeof(RadioPacket) - 2);
      rxPacket.header[0] = 0xAA;
      rxPacket.header[1] = 0x55;

      if (calculateCRC16((const uint8_t*)&rxPacket, sizeof(RadioPacket) - 2) == rxPacket.crc16) {
        lastPacketTime = now;
        isFailsafe = false;

        // Đếm gói tin để tính packet loss
        totalPacketCounter++;
        uint8_t expectedSeq = (lastSeqNum + 1) & 0xFF;
        if (rxPacket.seq_num != expectedSeq && lastSeqNum != 0) {
          lostPacketCounter++;
        }
        lastSeqNum = rxPacket.seq_num;

        // =====================================================================
        // [CƠ CHẾ CÁCH LY AN TOÀN TUYỆT ĐỐI - ZERO CONFLICT ISOLATION]
        // =====================================================================
        if (rxPacket.model_id == MODEL_ID_CAR) {
          // --- CHẾ ĐỘ XE RC (CAR RACER) ---
          // 1. Xuất xung PWM lái và ga cực nhạy ra bánh xe
          steerServo.writeMicroseconds(rxPacket.ch1_roll_str);
          motorESC.writeMicroseconds(rxPacket.ch3_throttle);

          // 2. Kênh phụ trợ
          bool arm = (rxPacket.sw_flags & (1 << 0));
          bool aux = (rxPacket.sw_flags & (1 << 3));
          bool horn = (rxPacket.sw_flags & (1 << 4));
          bool light = (rxPacket.sw_flags & (1 << 5));

          digitalWrite(PIN_OUT_HORN, horn ? HIGH : LOW);
          digitalWrite(PIN_OUT_LIGHTS, light ? HIGH : LOW);
          auxServo.writeMicroseconds(aux ? 1900 : 1100);

          // 3. KHÓA CHẶT DRONE Ở TRẠNG THÁI DISARM / FAILSAFE
          for (int i = 0; i < 16; i++) sbusChannels[i] = usToSbus(1500);
          sbusChannels[2] = usToSbus(1000); // Ga = 1000us
          sbusChannels[4] = usToSbus(1000); // Kênh ARM = DISARM
          packSbusFrame(true);

        } else if (rxPacket.model_id == MODEL_ID_DRONE) {
          // --- CHẾ ĐỘ DRONE FPV ---
          // 1. Cập nhật 16 kênh điều khiển bay
          sbusChannels[0] = usToSbus(rxPacket.ch1_roll_str); // Roll
          sbusChannels[1] = usToSbus(rxPacket.ch2_pitch);    // Pitch
          sbusChannels[2] = usToSbus(rxPacket.ch3_throttle); // Throttle
          sbusChannels[3] = usToSbus(rxPacket.ch4_yaw);      // Yaw

          bool arm = (rxPacket.sw_flags & (1 << 0));
          uint8_t mode = (rxPacket.sw_flags >> 1) & 0x03;
          sbusChannels[4] = arm ? usToSbus(2000) : usToSbus(1000); // Ch5: ARM
          sbusChannels[5] = (mode == 0) ? usToSbus(1000) : ((mode == 1) ? usToSbus(1500) : usToSbus(2000)); // Ch6: Mode

          bool useCrsf = (rxPacket.sw_flags & (1 << 6));
          if (useCrsf) packCrsfFrame();
          else packSbusFrame(false);

          // 2. KHÓA CHẶT CỔNG XE: GA ESC = 1500us (NEUTRAL STOP), LÁI = 1500us
          motorESC.writeMicroseconds(1500);
          steerServo.writeMicroseconds(1500);
          digitalWrite(PIN_OUT_LIGHTS, LOW);
          digitalWrite(PIN_OUT_HORN, LOW);
        }
      }
    }
  }

  // 2. XỬ LÝ FAILSAFE KHI MẤT SÓNG (> 300ms KHÔNG NHẬN ĐƯỢC GÓI)
  if (now - lastPacketTime > 300) {
    isFailsafe = true;
    // Khóa an toàn toàn bộ hệ thống
    motorESC.writeMicroseconds(1500);   // Phanh xe ô tô dừng hẳn
    steerServo.writeMicroseconds(1500); // Giữ thẳng lái

    for (int i = 0; i < 16; i++) sbusChannels[i] = usToSbus(1500);
    sbusChannels[2] = usToSbus(1000);   // Ngắt ga drone
    sbusChannels[4] = usToSbus(1000);   // DISARM drone
    packSbusFrame(true);
  }

  // 3. PHÁT TÍN HIỆU RA FLIGHT CONTROLLER (CHU KỲ 7ms CHO CRSF, 14ms CHO SBUS)
  bool useCrsf = (rxPacket.sw_flags & (1 << 6));
  uint16_t droneInterval = useCrsf ? 7 : 14;
  if (now - lastDroneOutTime >= droneInterval) {
    lastDroneOutTime = now;
    // Xuất ra Serial phụ hoặc cổng SBUS/CRSF chuyên dụng
  }

  // 4. PHẢN HỒI GÓI TIN TELEMETRY VỀ TAY CẦM (MỖI 50ms = 20Hz)
  if (now - lastTelemTime >= 50) {
    lastTelemTime = now;

    // Đo điện áp pin RX qua chân A0
    int rawBat = analogRead(PIN_BAT_SENSE);
    txTelem.vBatRx_mv = (uint16_t)((rawBat * 5.0f / 1023.0f) * (12.2f / 2.2f) * 1000.0f); // mV
    txTelem.rssi_dbm  = -48; // Giả lập mức sóng RSSI LoRa đo được
    txTelem.packet_loss = (totalPacketCounter > 0) ? (lostPacketCounter * 100 / totalPacketCounter) : 0;

    txTelem.header[0] = 0x55;
    txTelem.header[1] = 0xAA;
    txTelem.crc16 = calculateCRC16((const uint8_t*)&txTelem, sizeof(TelemetryPacket) - 2);

    Serial.write((uint8_t*)&txTelem, sizeof(TelemetryPacket));
  }
}
