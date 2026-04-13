#include <Arduino.h>          // Quan trọng: cấp uint8_t, pinMode, millis,...
#include "imu_manager.h"
#include "config.h"
#include <SPI.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// Kéo biến góc la bàn từ file compass_manager
extern float goc_la_ban;
extern float goc_roll_thuc_te, goc_pitch_thuc_te, goc_yaw_thuc_te;
extern Goi_du_lieu Du_lieu_gui_toi_ESP;

void writeReg(uint8_t reg, uint8_t data) {
  digitalWrite(IMU_NCS, LOW);
  SPI.transfer(reg);
  SPI.transfer(data);
  digitalWrite(IMU_NCS, HIGH);
}

void readRegs(uint8_t reg, uint8_t count, uint8_t *dest) {
  digitalWrite(IMU_NCS, LOW);
  SPI.transfer(reg | 0x80);
  for (uint8_t i = 0; i < count; i++) dest[i] = SPI.transfer(0x00);
  digitalWrite(IMU_NCS, HIGH);
}

void initIMU() {
  pinMode(IMU_NCS, OUTPUT);
  digitalWrite(IMU_NCS, HIGH);
  SPI.setSCLK(SCL_SCK);
  SPI.setMISO(SDO_MISO);
  SPI.setMOSI(SDA_MOSI);
  SPI.begin();
  writeReg(0x6B, 0x00);  // Wake up MPU
}

void updateIMUCalculations(float dt) {
  uint8_t buffer[14];
  readRegs(0x3B, 14, buffer);

  int16_t ax = (buffer[0] << 8) | buffer[1];
  int16_t ay = (buffer[2] << 8) | buffer[3];
  int16_t az = (buffer[4] << 8) | buffer[5];
  int16_t gx = (buffer[8] << 8) | buffer[9];
  int16_t gy = (buffer[10] << 8) | buffer[11];
  int16_t gz = (buffer[12] << 8) | buffer[13];

  float acc_x = -(ax / 16384.0) * 9.81;
  float acc_y = (ay / 16384.0) * 9.81;
  float acc_z = -(az / 16384.0) * 9.81;

  float gR_raw = -(gx / 131.0);
  float gP_raw = gy / 131.0;
  float gY_raw = -(gz / 131.0);

  static float gR = 0.0f, gP = 0.0f, gY = 0.0f;
  gR = 0.7f * gR + 0.3f * gR_raw;
  gP = 0.7f * gP + 0.3f * gP_raw;
  gY = 0.7f * gY + 0.3f * gY_raw;

  float rollAcc = atan2(acc_y, acc_z) * 180.0 / PI;
  float pitchAcc = atan2(-acc_x, sqrt(pow(acc_y, 2) + pow(acc_z, 2))) * 180.0 / PI;

  // Complementary filter cho Roll & Pitch
  goc_roll_thuc_te = 0.98 * (goc_roll_thuc_te + gR * dt) + 0.02 * rollAcc;
  goc_pitch_thuc_te = 0.98 * (goc_pitch_thuc_te + gP * dt) + 0.02 * pitchAcc;

  // Yaw từ Gyro + la bàn
  goc_yaw_thuc_te += gY * dt;
  if (goc_yaw_thuc_te >= 360.0f) goc_yaw_thuc_te -= 360.0f;
  if (goc_yaw_thuc_te < 0.0f) goc_yaw_thuc_te += 360.0f;

  float error_yaw = goc_la_ban - goc_yaw_thuc_te;
  if (error_yaw > 180.0f) error_yaw -= 360.0f;
  else if (error_yaw < -180.0f) error_yaw += 360.0f;

  goc_yaw_thuc_te += error_yaw * 0.1f;
  if (goc_yaw_thuc_te >= 360.0f) goc_yaw_thuc_te -= 360.0f;
  if (goc_yaw_thuc_te < 0.0f) goc_yaw_thuc_te += 360.0f;

  // Đóng gói dữ liệu gửi lên ESP32
  Du_lieu_gui_toi_ESP.Roll = goc_roll_thuc_te;
  Du_lieu_gui_toi_ESP.Pitch = goc_pitch_thuc_te;
  Du_lieu_gui_toi_ESP.Yaw = goc_yaw_thuc_te;

  Du_lieu_gui_toi_ESP.Gia_toc_x = rollAcc;
  Du_lieu_gui_toi_ESP.Gia_toc_y = pitchAcc;
  Du_lieu_gui_toi_ESP.Gia_toc_z = gY;   // tốc độ góc yaw

  int16_t temp_raw = (buffer[6] << 8) | buffer[7];
  Du_lieu_gui_toi_ESP.Nhiet_do = (temp_raw / 333.87) + 21.0;
}