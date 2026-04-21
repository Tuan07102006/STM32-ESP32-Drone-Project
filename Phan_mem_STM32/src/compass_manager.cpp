#include <Arduino.h>
#include "compass_manager.h"
#include "config.h"
#include <Wire.h>
#include <math.h>
#include <EEPROM.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

float goc_la_ban = 0.0f;
extern float goc_roll_thuc_te, goc_pitch_thuc_te;

// --- BIẾN HIỆU CHỈNH HARD-IRON (Dịch tâm điểm) ---
float X_offset = 0.0; 
float Y_offset = 0.0;
float Z_offset = 0.0;

// --- BIẾN HIỆU CHỈNH SOFT-IRON (Nắn tròn Elip) ---
float Scale_X = 1.0;
float Scale_Y = 1.0;
float Scale_Z = 1.0;

// Góc từ thiên (Magnetic Declination) - Chuyển từ Bắc Từ sang Bắc Thật
float declination_angle = -1.0; 

void saveCalibrationToEEPROM() {
    EEPROM.put(EEPROM_ADDR_OFFSET_X, X_offset);
    EEPROM.put(EEPROM_ADDR_OFFSET_Y, Y_offset);
    EEPROM.put(EEPROM_ADDR_OFFSET_Z, Z_offset);
    EEPROM.put(EEPROM_ADDR_SCALE_X, Scale_X);
    EEPROM.put(EEPROM_ADDR_SCALE_Y, Scale_Y);
    EEPROM.put(EEPROM_ADDR_SCALE_Z, Scale_Z);
    EEPROM.write(EEPROM_SIGNATURE, 0xAB); // đánh dấu đã calib
}

bool loadCalibrationFromEEPROM() {
    if (EEPROM.read(EEPROM_SIGNATURE) != 0xAB) return false;
    EEPROM.get(EEPROM_ADDR_OFFSET_X, X_offset);
    EEPROM.get(EEPROM_ADDR_OFFSET_Y, Y_offset);
    EEPROM.get(EEPROM_ADDR_OFFSET_Z, Z_offset);
    EEPROM.get(EEPROM_ADDR_SCALE_X, Scale_X);
    EEPROM.get(EEPROM_ADDR_SCALE_Y, Scale_Y);
    EEPROM.get(EEPROM_ADDR_SCALE_Z, Scale_Z);
    return true;
}

void initCompass() {
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x0A); Wire.write(0x1D); 
    Wire.endTransmission();

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x0B); Wire.write(0x01); 
    Wire.endTransmission();

    // Load calibration từ EEPROM nếu có
    if (!loadCalibrationFromEEPROM()) {
    }
}


// HÀM CALIBRATION TỔNG HỢP (Hard-iron + Soft-iron 3D)
// Gọi hàm này, cầm Drone xoay vòng tròn theo MỌI HƯỚNG (như khối Rubik) trong 20s
void calibrateCompass() {
  int16_t x_min = 32767, x_max = -32768;
  int16_t y_min = 32767, y_max = -32768;
  int16_t z_min = 32767, z_max = -32768;

  Serial.println(">>> BAT DAU CALIB: XOAY DRONE THEO CAC TRUC 3D TRONG 20 GIAY...");
  uint32_t start_time = millis();
  
  while (millis() - start_time < 20000) { // Quét trong 20 giây
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)QMC5883P_ADDR, (uint8_t)6);
    if (Wire.available() >= 6) {
      // Đọc Endian chuẩn của QMC5883 (LSB trước)
      int16_t raw_x = Wire.read() | (Wire.read() << 8);
      int16_t raw_y = Wire.read() | (Wire.read() << 8);
      int16_t raw_z = Wire.read() | (Wire.read() << 8);

      if (raw_x < x_min) x_min = raw_x;
      if (raw_x > x_max) x_max = raw_x;
      if (raw_y < y_min) y_min = raw_y;
      if (raw_y > y_max) y_max = raw_y;
      if (raw_z < z_min) z_min = raw_z;
      if (raw_z > z_max) z_max = raw_z;
    }
    delay(20); // Đọc nhanh 50Hz để lấy mẫu dày đặc
  }

  // 1. Tính toán Hard-Iron (Tâm của hình cầu từ trường)
  X_offset = (x_max + x_min) / 2.0;
  Y_offset = (y_max + y_min) / 2.0;
  Z_offset = (z_max + z_min) / 2.0;

  // 2. Tính toán Soft-Iron (Bán kính các trục để nắn Elip thành hình cầu)
  float chord_x = (x_max - x_min) / 2.0;
  float chord_y = (y_max - y_min) / 2.0;
  float chord_z = (z_max - z_min) / 2.0;

  float avg_chord = (chord_x + chord_y + chord_z) / 3.0;

  // Tránh lỗi chia cho 0 nếu cảm biến lỗi
  if (chord_x > 0 && chord_y > 0 && chord_z > 0) {
    Scale_X = avg_chord / chord_x;
    Scale_Y = avg_chord / chord_y;
    Scale_Z = avg_chord / chord_z;
  }
}

// HÀM ĐỌC VÀ TÍNH TOÁN (Tốc độ 50Hz + Bù nghiêng 3D)
void readCompass() {
  static uint32_t lastCompassRead = 0;
  uint32_t now = millis();

  // Nâng tốc độ lên 50Hz (20ms) để theo kịp Drone
  if (now - lastCompassRead >= 20) { 
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)QMC5883P_ADDR, (uint8_t)6);
    
    if (Wire.available() >= 6) {
      int16_t raw_x = Wire.read() | (Wire.read() << 8);
      int16_t raw_y = Wire.read() | (Wire.read() << 8);
      int16_t raw_z = Wire.read() | (Wire.read() << 8);

      // BƯỚC 1: KHỬ NHIỄU (CALIBRATION KÉP)
      // Vừa trừ Offset (Hard), vừa nhân Scale (Soft)
      float x_cal = ((float)raw_x - X_offset) * Scale_X;
      float y_cal = ((float)raw_y - Y_offset) * Scale_Y;
      float z_cal = -((float)raw_z - Z_offset) * Scale_Z;

      //  BƯỚC 2: BÙ NGHIÊNG 3D (TILT COMPENSATION) 
      float roll_rad = goc_roll_thuc_te * PI / 180.0f;
      float pitch_rad = goc_pitch_thuc_te * PI / 180.0f;

      float cos_roll = cos(roll_rad);
      float sin_roll = sin(roll_rad);
      float cos_pitch = cos(pitch_rad);
      float sin_pitch = sin(pitch_rad);

      // Chiếu vector từ trường 3D lên mặt phẳng ngang tuyệt đối
      float X_horizontal = x_cal * cos_pitch + z_cal * sin_pitch;
      float Y_horizontal = x_cal * sin_roll * sin_pitch + y_cal * cos_roll - z_cal * sin_roll * cos_pitch;

      //  BƯỚC 3: TÍNH GÓC YAW THỰC TẾ
      float heading = atan2(Y_horizontal, X_horizontal) * 180.0f / PI;

      heading += declination_angle;

      if (heading < 0.0f) heading += 360.0f;
      if (heading >= 360.0f) heading -= 360.0f;

     static float heading_filtered = -1.0f;
      
      // Khởi tạo lần đọc đầu tiên
      if (heading_filtered < 0.0f) {
        heading_filtered = heading;
      } else {
        // Chú ý: Vì góc là 360 độ (0 và 359 nằm cạnh nhau), 
        // phải tính khoảng cách ngắn nhất trước khi lọc để Drone không bị xoay ngược
        float diff = heading - heading_filtered;
        if (diff > 180.0f) diff -= 360.0f;
        else if (diff < -180.0f) diff += 360.0f;
        
        // Công thức lọc: Lấy 70% số cũ + 30% số mới
        heading_filtered += diff * 0.3f; 
        
        // Chuẩn hóa lại 0 - 360
        if (heading_filtered >= 360.0f) heading_filtered -= 360.0f;
        if (heading_filtered < 0.0f) heading_filtered += 360.0f;
      }

      goc_la_ban = heading_filtered;
    lastCompassRead = now;
  }
}
}
void calibrateAndSave() {
    calibrateCompass();  
    saveCalibrationToEEPROM();
}