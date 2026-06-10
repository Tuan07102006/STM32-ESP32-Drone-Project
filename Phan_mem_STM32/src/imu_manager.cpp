#include <Arduino.h>          // Quan trọng: cấp uint8_t, pinMode, millis,...
#include "imu_manager.h"
#include "config.h"
#include <SPI.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef PI
#define PI 3.14159265358979323846
#endif

// --- CÁC BIẾN CHO BỘ LỌC MADGWICK ---
float beta = 0.1f; // Hệ số lọc (có thể chỉnh từ 0.05 đến 0.2)
float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f; // Quaternion


// Hàm căn bậc hai nghịch đảo cực nhanh (Fast Inverse Square Root)
float invSqrt(float x) {
    float halfx = 0.5f * x;
    float y = x;
    long i = *(long*)&y;
    i = 0x5f3759df - (i >> 1);
    y = *(float*)&i;
    y = y * (1.5f - (halfx * y * y));
    return y;
}

void MadgwickAHRSupdateIMU(float gx, float gy, float gz, float ax, float ay, float az, float dt) {
    float recipNorm;
    float s0, s1, s2, s3;
    float qDot1, qDot2, qDot3, qDot4;
    float _2q0, _2q1, _2q2, _2q3, _4q0, _4q1, _4q2 ,_8q1, _8q2, q0q0, q1q1, q2q2, q3q3;

    // Chỉ tính toán khi gia tốc kế có dữ liệu hợp lệ
    if(!((ax == 0.0f) && (ay == 0.0f) && (az == 0.0f))) {
        // Chuẩn hóa vector gia tốc
        recipNorm = invSqrt(ax * ax + ay * ay + az * az);
        ax *= recipNorm;
        ay *= recipNorm;
        az *= recipNorm;

        // Tối ưu hóa các phép tính lặp
        _2q0 = 2.0f * q0; _2q1 = 2.0f * q1; _2q2 = 2.0f * q2; _2q3 = 2.0f * q3;
        _4q0 = 4.0f * q0; _4q1 = 4.0f * q1; _4q2 = 4.0f * q2;
        _8q1 = 8.0f * q1; _8q2 = 8.0f * q2;
        q0q0 = q0 * q0; q1q1 = q1 * q1; q2q2 = q2 * q2; q3q3 = q3 * q3;

        // Gradient Descent (Thuật toán lõi)
        s0 = _4q0 * q2q2 + _2q2 * ax + _4q0 * q1q1 - _2q1 * ay;
        s1 = _4q1 * q3q3 - _2q3 * ax + 4.0f * q0q0 * q1 - _2q0 * ay - _4q1 + _8q1 * q1q1 + _8q1 * q2q2 + _4q1 * az;
        s2 = 4.0f * q0q0 * q2 + _2q0 * ax + _4q2 * q3q3 - _2q3 * ay - _4q2 + _8q2 * q1q1 + _8q2 * q2q2 + _4q2 * az;
        s3 = 4.0f * q1q1 * q3 - _2q1 * ax + 4.0f * q2q2 * q3 - _2q2 * ay;
        
        // Chuẩn hóa vector Gradient
        recipNorm = invSqrt(s0 * s0 + s1 * s1 + s2 * s2 + s3 * s3);
        s0 *= recipNorm; s1 *= recipNorm; s2 *= recipNorm; s3 *= recipNorm;

        // Tốc độ thay đổi của Quaternion (dựa trên Gyro và bù trừ sai số)
        qDot1 = 0.5f * (-q1 * gx - q2 * gy - q3 * gz) - beta * s0;
        qDot2 = 0.5f * (q0 * gx + q2 * gz - q3 * gy) - beta * s1;
        qDot3 = 0.5f * (q0 * gy - q1 * gz + q3 * gx) - beta * s2;
        qDot4 = 0.5f * (q0 * gz + q1 * gy - q2 * gx) - beta * s3;

        // Tích phân Euler cập nhật Quaternion
        q0 += qDot1 * dt; q1 += qDot2 * dt; q2 += qDot3 * dt; q3 += qDot4 * dt;

        // Chuẩn hóa lại Quaternion đầu ra
        recipNorm = invSqrt(q0 * q0 + q1 * q1 + q2 * q2 + q3 * q3);
        q0 *= recipNorm; q1 *= recipNorm; q2 *= recipNorm; q3 *= recipNorm;
    }
}


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
  writeReg(0x6B, 0x00); 
  writeReg(0x1A, 0x03);
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

  // Gia tốc kế
  float acc_x = -(ax / 16384.0f) * 9.81f;
  float acc_y = (ay / 16384.0f) * 9.81f;
  float acc_z = -(az / 16384.0f) * 9.81f;

  // Lọc thông thấp (Low-pass filter) cho Gyro thô (Chống nhiễu tần số cao)
  float gR_raw = -(gx / 131.0f);
  float gP_raw = gy / 131.0f;
  float gY_raw = -(gz / 131.0f);

  static float gR = 0.0f, gP = 0.0f, gY = 0.0f;
  gR = gR_raw;
  gP = gP_raw;
  gY = gY_raw;
  //Madgwick yêu cầu Gyro tính bằng Radians/giây (rad/s)
  float gyro_rad_x = gR * PI / 180.0f;
  float gyro_rad_y = gP * PI / 180.0f;
  float gyro_rad_z = gY * PI / 180.0f;

  // Gọi bộ lọc Madgwick
  MadgwickAHRSupdateIMU(gyro_rad_x, gyro_rad_y, gyro_rad_z, acc_x, acc_y, acc_z, dt);

  // 2. CHẶN GIỚI HẠN AN TOÀN CHO HÀM ASIN ĐỂ CHỐNG LỖI NaN
  goc_roll_thuc_te  = atan2(2.0f * (q0 * q1 + q2 * q3), 1.0f - 2.0f * (q1 * q1 + q2 * q2)) * 180.0f / PI;
  
  float sinp = 2.0f * (q0 * q2 - q3 * q1);
  if (sinp >= 1.0f) sinp = 1.0f;
  else if (sinp <= -1.0f) sinp = -1.0f;
  goc_pitch_thuc_te = asin(sinp) * 180.0f / PI;
  
  // Xử lý Yaw
  goc_yaw_thuc_te += gY * dt;
  if (goc_yaw_thuc_te >= 360.0f) goc_yaw_thuc_te -= 360.0f;
  if (goc_yaw_thuc_te < 0.0f) goc_yaw_thuc_te += 360.0f;

  // Bù trôi Yaw bằng La bàn
  float error_yaw = goc_la_ban - goc_yaw_thuc_te;
  if (error_yaw > 180.0f) error_yaw -= 360.0f;
  else if (error_yaw < -180.0f) error_yaw += 360.0f;

  goc_yaw_thuc_te += error_yaw * 0.1f;
  if (goc_yaw_thuc_te >= 360.0f) goc_yaw_thuc_te -= 360.0f;
  if (goc_yaw_thuc_te < 0.0f) goc_yaw_thuc_te += 360.0f;

  // Gói dữ liệu
  Du_lieu_gui_toi_ESP.Roll = goc_roll_thuc_te;
  Du_lieu_gui_toi_ESP.Pitch = goc_pitch_thuc_te;
  Du_lieu_gui_toi_ESP.Yaw = goc_yaw_thuc_te;

  Du_lieu_gui_toi_ESP.Gia_toc_x = acc_x;
  Du_lieu_gui_toi_ESP.Gia_toc_y = acc_y;
  Du_lieu_gui_toi_ESP.Gia_toc_z = gY; 
}