#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// --- PINOUT ---
#define Dong_co_1 PB5  
#define Dong_co_2 PA1
#define Dong_co_3 PA2
#define Dong_co_4 PA3
#define SCL_SCK PA5 
#define SDO_MISO PA6
#define SDA_MOSI PA7
#define IMU_NCS PA4
#define BMP_CBS PB0
#define I2C_SCL PB6  
#define I2C_SDA PB7
#define Buzzer PA8

#define Gioi_han_toc_do(val, min, max) constrain(val, min, max)

// --- STRUCTS ---
// Thùng hàng xuất khẩu: STM32 -> ESP32
struct __attribute__((packed)) Goi_du_lieu {
  float Roll, Pitch, Yaw;
  float Gia_toc_x, Gia_toc_y, Gia_toc_z;
  float Dien_ap, Dong_dien;
  float Do_cao, Ap_xuat, Nhiet_do;
  float goc_la_ban;
};

// Thùng hàng nhập khẩu: ESP32 -> STM32
struct __attribute__((packed)) Lenh_Dieu_Khien {
  float Muc_Ga;
  float Diem_dat_Roll;
  float Diem_dat_Pitch;
  float Diem_dat_Yaw;
  float Kp_roll_moi;
  float Ki_roll_moi;
  float Kd_roll_moi;
  float Kp_pitch_moi;
  float Ki_pitch_moi;
  float Kd_pitch_moi;
  float Kp_yaw_moi;
  float Ki_yaw_moi;
  uint8_t Trang_thai_Arm; 
};

struct Bo_dieu_khien_PID {
  float Kp, Ki, Kd;
  float Ti_le, Tich_phan, Dao_ham;
  float Loi_truoc_do;
  float Tich_phan_toi_da;
  float Dau_ra;
};

//  BIẾN TOÀN CỤC 
extern Goi_du_lieu Du_lieu_gui_toi_ESP;
extern Lenh_Dieu_Khien Lenh_tu_ESP;
extern Bo_dieu_khien_PID PID_roll, PID_pitch, PID_yaw;
extern float goc_roll_thuc_te, goc_pitch_thuc_te, goc_yaw_thuc_te;

#endif