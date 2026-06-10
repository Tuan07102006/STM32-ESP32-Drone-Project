#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// CẤU HÌNH CHÂN
#define CHAN_NOI_LED 48  
#define SO_BONG_LED 1
#define STM32_RX 18  
#define STM32_TX 17
#define CHAN_NOI_GPS_RX 16
#define CHAN_NOI_GPS_TX 15
#define GPS_BAUD 9600

//  CẤU TRÚC DỮ LIỆU 
// STM32 -> ESP32
struct __attribute__((packed)) Goi_du_lieu {
  float Roll, Pitch, Yaw;
  float Gia_toc_x, Gia_toc_y, Gia_toc_z;
  float Dien_ap, Dong_dien;
  float Do_cao, Ap_xuat, Nhiet_do;
};

// ESP32 -> STM32
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

struct __attribute__((packed)) GPS_Data {
  float gps_lat;    // Vĩ độ (4 bytes)
  float gps_lng;    // Kinh độ (4 bytes)
  float gps_alt;    // Độ cao GPS (4 bytes)
  float gps_speed;  // Vận tốc (4 bytes)
  float gps_course; // Hướng di chuyển (4 bytes)
  float gps_hdop;   // Độ chính xác (4 bytes)
  uint8_t gps_sat;  // Số lượng vệ tinh (1 byte) 
};


// BIẾN TOÀN CỤC 
extern Goi_du_lieu Du_lieu_gui_toi_ESP;
extern Lenh_Dieu_Khien Lenh_gui_di;
extern GPS_Data GPS_data;
//extern Device_Status Device_status;

#endif