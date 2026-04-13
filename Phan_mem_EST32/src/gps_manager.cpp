#include "gps_manager.h"
#include "config.h" 
#include <Arduino.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); 

// Khởi tạo biến
double gps_lat = 0.0;
double gps_lng = 0.0;
int    gps_sat = 0;
double gps_hdop = 99.9; // Mặc định 99.9 là sai số cực lớn (Chưa có sóng)
double gps_speed = 0.0;
double gps_alt = 0.0;
double gps_course = 0.0;

void initGPS() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, CHAN_NOI_GPS_RX, CHAN_NOI_GPS_TX);
}

void readGPSRaw() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      
      // 1. Tọa độ (Vĩ độ, Kinh độ)
      if (gps.location.isValid()) {
        gps_lat = gps.location.lat();
        gps_lng = gps.location.lng();
      }
      
      // 2. Số lượng vệ tinh đang kết nối
      if (gps.satellites.isValid()) {
        gps_sat = gps.satellites.value();
      }

      // 3. HDOP (Horizontal Dilution of Precision - Sai số mặt phẳng)
      if (gps.hdop.isValid()) {
        gps_hdop = gps.hdop.hdop();
      }

      // 4. Vận tốc bay thực tế (Chuyển sang m/s cho chuẩn hệ mét)
      if (gps.speed.isValid()) {
        gps_speed = gps.speed.mps(); 
      }

      // 5. Độ cao so với mực nước biển 
      if (gps.altitude.isValid()) {
        gps_alt = gps.altitude.meters();
      }

      // 6. Hướng di chuyển (Course over ground)
      if (gps.course.isValid()) {
        gps_course = gps.course.deg();
      }
      
    }
  }
}