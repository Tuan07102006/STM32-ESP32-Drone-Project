#include <Arduino.h>
#include "compass_manager.h"
#include "config.h"
#include <Wire.h>
#include <math.h>
#include <EEPROM.h> 
#include "Buzzer_manager.h"

#ifndef PI
#define PI 3.14159265358979323846
#endif

extern Goi_du_lieu Du_lieu_gui_toi_ESP;

extern float goc_roll_thuc_te;
extern float goc_pitch_thuc_te;

float goc_la_ban = 0.0f;

float lech_x = 0.0; float lech_y = 0.0; float lech_z = 0.0;
float ty_le_x = 1.0; float ty_le_y = 1.0; float ty_le_z = 1.0;
float goc_tu_thien = -1.0; 

typedef struct {
  float off_x; float off_y; float off_z;
  float scale_x; float scale_y; float scale_z;
  uint8_t signature;
} CompassCalibData;

void saveCalibrationToEEPROM() {
  noInterrupts(); // Khóa ngắt bảo vệ vi điều khiển
  CompassCalibData data = {
    lech_x, lech_y, lech_z, 
    ty_le_x, ty_le_y, ty_le_z, 
    0xAB
  };
    EEPROM.put(0, data);
    interrupts(); // Mở ngắt
}

bool loadCalibrationFromEEPROM() {
    CompassCalibData data;
    EEPROM.get(0, data); 
    if (data.signature != 0xAB) return false; 
    
    lech_x = data.off_x; lech_y = data.off_y; lech_z = data.off_z;
    ty_le_x = data.scale_x; ty_le_y = data.scale_y; ty_le_z = data.scale_z;
    return true;
}

void initCompass() {
    Wire.begin();
    Wire.setClock(100000); // Ép chuẩn tốc độ I2C an toàn chống treo

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_CTRL1); 
    Wire.write(QMC5883P_VAL_CTRL1); 
    Wire.endTransmission();

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_CTRL2); 
    Wire.write(QMC5883P_VAL_CTRL2); 
    Wire.endTransmission();

    if (!loadCalibrationFromEEPROM()) {
        // Default
    }
}

void calibrateCompass() {
  int16_t x_min = 32767, x_max = -32768;
  int16_t y_min = 32767, y_max = -32768;
  int16_t z_min = 32767, z_max = -32768;
  
  Serial.println(">>> BAT DAU CALIB: XOAY DRONE TRONG 20 GIAY...");
  uint32_t start_time = millis();
  
  while (millis() - start_time < 20000) { 
    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_DATA);
    
    if (Wire.endTransmission() == 0) {
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
    } else {
        // Nếu phát hiện lỗi I2C, lập tức reset lại chân I2C để STM32 thoát kẹt
        Wire.begin();
        Wire.setClock(100000);
    }
    
    // Tăng từ 20ms lên 50ms (20Hz) để cảm biến kịp trả lời, chống treo
    delay(50); 
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

  // Khi bay thực tế, nếu bị nghẽn cũng giãn ra 50ms thay vì 20ms
  if (now - lastCompassRead >= 50) { 
    lastCompassRead = now; 

    Wire.beginTransmission(QMC5883P_ADDR);
    Wire.write(QMC5883P_REG_DATA); 
    if (Wire.endTransmission() != 0) {
        Wire.begin(); // Reset I2C chống rơi drone
        return; 
    }

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
      float cos_roll = cos(roll_rad); float sin_roll = sin(roll_rad);
      float cos_pitch = cos(pitch_rad); float sin_pitch = sin(pitch_rad);

      float x_horizontal = x_cal * cos_pitch + z_cal * sin_pitch;
      float y_horizontal = x_cal * sin_roll * sin_pitch + y_cal * cos_roll - z_cal * sin_roll * cos_pitch;

      float heading = atan2(y_horizontal, x_horizontal) * 180.0f / PI;
      heading += goc_tu_thien;

      if (heading < 0.0f) heading += 360.0f;
      if (heading >= 360.0f) heading -= 360.0f;

      static float heading_filtered = 0.0f;
      static bool heading_initialized = false;
      
      if (!heading_initialized) {
        heading_filtered = heading; heading_initialized = true;
      } else {
        float diff = heading - heading_filtered;
        if (diff > 180.0f) diff -= 360.0f;
        else if (diff < -180.0f) diff += 360.0f;
        
        heading_filtered += diff * 0.3f; 
        if (heading_filtered >= 360.0f) heading_filtered -= 360.0f;
        if (heading_filtered < 0.0f) heading_filtered += 360.0f;
      }
      Du_lieu_gui_toi_ESP.goc_la_ban = heading_filtered;
    }
  }
}

void calibrateAndSave() {
    buzzer_bat_dau_calib();

    // 1. Quét I2C trong 20 giây (đã fix tốc độ và chống treo)
    calibrateCompass();  
    
    // 2. Còi kết thúc
    tat_buzzer(); delay(50);
    bat_buzzer(); delay(150); tat_buzzer(); delay(150);
    bat_buzzer(); delay(150); tat_buzzer(); delay(150);
    buzzer_ket_thuc_calib(); 
    
    delay(500); 
    
    // 3. Ghi an toàn vào bộ nhớ
    saveCalibrationToEEPROM(); 
}