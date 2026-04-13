#ifndef GPS_MANAGER_H
#define GPS_MANAGER_H

#include <Arduino.h>
#include <TinyGPS++.h>

// --- CÁC BIẾN DỮ LIỆU GPS TỔNG HỢP ---
extern double gps_lat;     // Vĩ độ
extern double gps_lng;     // Kinh độ
extern int    gps_sat;     // Số lượng vệ tinh
extern double gps_hdop;    // Độ chính xác (Càng nhỏ càng tốt, < 2.0 là bay an toàn)
extern double gps_speed;   // Vận tốc (m/s)
extern double gps_alt;     // Độ cao GPS (mét)
extern double gps_course;  // Hướng di chuyển (Độ, giống la bàn)

void initGPS();
void readGPSRaw();

#endif