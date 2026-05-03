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

// --- BIẾN HIỆU CHỈNH HARD-IRON (Dịch tâm điểm) ---
float lech_x = 0.0; 
float lech_y = 0.0;
float lech_z = 0.0;

// --- BIẾN HIỆU CHỈNH SOFT-IRON (Nắn tròn Elip) ---
float ty_le_x = 1.0;
float ty_le_y = 1.0;
float ty_le_z = 1.0;

// Góc từ thiên (Chuyển từ Bắc Từ sang Bắc Thật)
float goc_tu_thien = -1.0; 

void saveCalibrationToEEPROM() {
    EEPROM.put(EEPROM_ADDR_OFFSET_X, lech_x);
    EEPROM.put(EEPROM_ADDR_OFFSET_Y, lech_y);
    EEPROM.put(EEPROM_ADDR_OFFSET_Z, lech_z);
    EEPROM.put(EEPROM_ADDR_SCALE_X, ty_le_x);
    EEPROM.put(EEPROM_ADDR_SCALE_Y, ty_le_y);
    EEPROM.put(EEPROM_ADDR_SCALE_Z, ty_le_z);
    EEPROM.write(EEPROM_SIGNATURE, 0xAB); 
}

bool loadCalibrationFromEEPROM() {
    if (EEPROM.read(EEPROM_SIGNATURE) != 0xAB) return false;
    EEPROM.get(EEPROM_ADDR_OFFSET_X, lech_x);
    EEPROM.get(EEPROM_ADDR_OFFSET_Y, lech_y);
    EEPROM.get(EEPROM_ADDR_OFFSET_Z, lech_z);
    EEPROM.get(EEPROM_ADDR_SCALE_X, ty_le_x);
    EEPROM.get(EEPROM_ADDR_SCALE_Y, ty_le_y);
    EEPROM.get(EEPROM_ADDR_SCALE_Z, ty_le_z);
    return true;
}

void initCompass() {
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_CTRL1); 
    Wire.write(QMC5883P_VAL_CTRL1); 
    Wire.endTransmission();

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_CTRL2); 
    Wire.write(QMC5883P_VAL_CTRL2); 
    Wire.endTransmission();

    if (!loadCalibrationFromEEPROM()) {
        // Không có dữ liệu calib lưu sẵn trong EEPROM - sử dụng giá trị mặc định
    }
}

void calibrateCompass() {
  int16_t x_min = 32767, x_max = -32768;
  int16_t y_min = 32767, y_max = -32768;
  int16_t z_min = 32767, z_max = -32768;

  Serial.println(">>> BAT DAU CALIB: XOAY DRONE THEO CAC TRUC 3D TRONG 20 GIAY...");
  uint32_t start_time = millis();
  
  // LƯU Ý: Vòng lặp này có chứa delay(20), CHỈ GỌI hàm này khi drone dưới mặt đất
  while (millis() - start_time < 20000) { 
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_DATA);
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)QMC5883P_ADDR, (uint8_t)6);
    if (Wire.available() >= 6) {
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
    delay(20); 
  }

  lech_x = (x_max + x_min) / 2.0;
  lech_y = (y_max + y_min) / 2.0;
  lech_z = (z_max + z_min) / 2.0;

  float day_cung_x = (x_max - x_min) / 2.0;
  float day_cung_y = (y_max - y_min) / 2.0;
  float day_cung_z = (z_max - z_min) / 2.0;

  float trung_binh_day_cung = (day_cung_x + day_cung_y + day_cung_z) / 3.0;

  if (day_cung_x > 0 && day_cung_y > 0 && day_cung_z > 0) {
    ty_le_x = trung_binh_day_cung / day_cung_x;
    ty_le_y = trung_binh_day_cung / day_cung_y;
    ty_le_z = trung_binh_day_cung / day_cung_z;
  }
}

void readCompass() {
  static uint32_t lastCompassRead = 0;
  uint32_t now = millis();

  if (now - lastCompassRead >= 20) { 
    lastCompassRead = now; 

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_DATA); 
    Wire.endTransmission();

    Wire.requestFrom((uint8_t)QMC5883P_ADDR, (uint8_t)6);
    
    if (Wire.available() >= 6) {
      int16_t raw_x = Wire.read() | (Wire.read() << 8);
      int16_t raw_y = Wire.read() | (Wire.read() << 8);
      int16_t raw_z = Wire.read() | (Wire.read() << 8);

      float x_cal = ((float)raw_x - lech_x) * ty_le_x;
      float y_cal = ((float)raw_y - lech_y) * ty_le_y;
      float z_cal = -((float)raw_z - lech_z) * ty_le_z;

      float roll_rad = goc_roll_thuc_te * PI / 180.0f;
      float pitch_rad = goc_pitch_thuc_te * PI / 180.0f;

      float cos_roll = cos(roll_rad);
      float sin_roll = sin(roll_rad);
      float cos_pitch = cos(pitch_rad);
      float sin_pitch = sin(pitch_rad);

      float x_horizontal = x_cal * cos_pitch + z_cal * sin_pitch;
      float y_horizontal = x_cal * sin_roll * sin_pitch + y_cal * cos_roll - z_cal * sin_roll * cos_pitch;

      float heading = atan2(y_horizontal, x_horizontal) * 180.0f / PI;

      heading += goc_tu_thien;

      if (heading < 0.0f) heading += 360.0f;
      if (heading >= 360.0f) heading -= 360.0f;

      static float heading_filtered = 0.0f;
      static bool heading_initialized = false;
      
      if (!heading_initialized) {
        heading_filtered = heading;
        heading_initialized = true;
      } else {
        float diff = heading - heading_filtered;
        if (diff > 180.0f) diff -= 360.0f;
        else if (diff < -180.0f) diff += 360.0f;
        
        heading_filtered += diff * 0.3f; 
        
        if (heading_filtered >= 360.0f) heading_filtered -= 360.0f;
        if (heading_filtered < 0.0f) heading_filtered += 360.0f;
      }

      goc_la_ban = heading_filtered;
    }
  }
}

void calibrateAndSave() {
    calibrateCompass();  
    saveCalibrationToEEPROM();
}