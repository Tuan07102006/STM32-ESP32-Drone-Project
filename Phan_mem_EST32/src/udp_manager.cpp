#include "udp_manager.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WiFi.h>
#include "memory_management.h"
#include <Preferences.h>

WiFiUDP udp;

// Biến toàn cục lấy từ config.h
extern Lenh_Dieu_Khien Lenh_gui_di;
extern GPS_Data GPS_data; 
extern Goi_du_lieu Du_lieu_gui_toi_ESP;
extern Preferences memory_management;

// ĐỊA CHỈ MÁY TÍNH 
const char* targetIP = "192.168.137.79"; 
const int targetPort = 34542;
const int localPort = 34542;   //cổng để ESP32 lắng nghe lệnh từ App

// BIẾN CHO FAILSAFE
unsigned long thoi_gian_nhan_lenh_cuoi = 0; 
const unsigned long THOI_GIAN_MAT_SONG_TOI_DA = 2000; 

void setupUDP() {
  udp.begin(localPort); //  Mở cổng để bắt đầu nhận dữ liệu UDP
}

void receiveCommandsUDP() {
  int packetSize = udp.parsePacket(); 
  if (packetSize) {
    // Sửa lỗi: Tăng kích thước mảng lên 256 để chứa ký tự '\0' một cách an toàn
    char packetBuffer[256];
    int len = udp.read(packetBuffer, 255);
    if (len > 0) packetBuffer[len] = '\0'; // Kết thúc chuỗi chuẩn C
    
    String incomingData = String(packetBuffer);
    thoi_gian_nhan_lenh_cuoi = millis(); 

    // LỆNH 1: ĐIỀU KHIỂN BAY TỪ JOYSTICK
    if (incomingData.startsWith("#") && incomingData.endsWith("*")) {
      float tempGa, tempRoll, tempPitch, tempYaw;
      int tempArm;
      int parsed = sscanf(incomingData.c_str(), "#%f,%f,%f,%f,%d*", &tempGa, &tempRoll, &tempPitch, &tempYaw, &tempArm);
      
      if (parsed == 5) {
        Lenh_gui_di.Muc_Ga = tempGa;
        Lenh_gui_di.Diem_dat_Roll = tempRoll;
        Lenh_gui_di.Diem_dat_Pitch = tempPitch;
        Lenh_gui_di.Diem_dat_Yaw = tempYaw;
        Lenh_gui_di.Trang_thai_Arm = (uint8_t)tempArm;
      }
    }

    // LỆNH 2: CẬP NHẬT PID
    else if (incomingData.startsWith("@") && incomingData.endsWith("*")) {
      float r_p, r_i, r_d, p_p, p_i, p_d, y_p, y_i, y_d;
      int parsed = sscanf(incomingData.c_str(), "@%f,%f,%f,%f,%f,%f,%f,%f,%f*", 
                          &r_p, &r_i, &r_d, &p_p, &p_i, &p_d, &y_p, &y_i, &y_d);
      
      if (parsed == 9) {
        Lenh_gui_di.Kp_roll_moi = r_p;
       // Lenh_gui_di.Ki_roll_moi = r_i;
        Lenh_gui_di.Kd_roll_moi = r_d;
        Lenh_gui_di.Kp_pitch_moi = p_p;
        //Lenh_gui_di.Ki_pitch_moi = p_i;
        Lenh_gui_di.Kd_pitch_moi = p_d;
        Lenh_gui_di.Kp_yaw_moi = y_p;
        Lenh_gui_di.Ki_yaw_moi = y_i;
        
        memory_management.putFloat("Kp_roll", r_p);
        //memory_management.putFloat("Ki_roll", r_i);
        memory_management.putFloat("Kd_roll", r_d);
        memory_management.putFloat("Kp_pitch", p_p);
       // memory_management.putFloat("Ki_pitch", p_i);
        memory_management.putFloat("Kd_pitch", p_d);
        memory_management.putFloat("Kp_yaw", y_p);
        memory_management.putFloat("Ki_yaw", y_i);
      }
    }
  }
}

void checkFailsafe() {
  if (thoi_gian_nhan_lenh_cuoi == 0) return;

  unsigned long hien_tai = millis();
  if (hien_tai - thoi_gian_nhan_lenh_cuoi > THOI_GIAN_MAT_SONG_TOI_DA) {
    if (Lenh_gui_di.Trang_thai_Arm == 1) {
      
     
      Lenh_gui_di.Diem_dat_Roll = 0.0;
      Lenh_gui_di.Diem_dat_Pitch = 0.0;
      Lenh_gui_di.Diem_dat_Yaw = 0.0;


      // mỗi 100ms ta giảm ga đi 5 đơn vị cho đến khi đạt mức hạ cánh an toàn
      static unsigned long thoi_gian_giam_ga = 0;
      if (hien_tai - thoi_gian_giam_ga > 100) { 
        if (Lenh_gui_di.Muc_Ga > 1250) { // Giới hạn mức ga tối thiểu để cánh quạt vẫn quay
          Lenh_gui_di.Muc_Ga -= 5; 
        }
        thoi_gian_giam_ga = hien_tai;
      }

      // 3. Nếu ga đã xuống rất thấp, tự động DISARM sau một khoảng thời gian (ví dụ 10 giây)
      if (hien_tai - thoi_gian_nhan_lenh_cuoi > 10000) {
        Lenh_gui_di.Trang_thai_Arm = 0;
        Lenh_gui_di.Muc_Ga = 1000;
      }
    }
  }
}

void sendTelemetryUDP() {
  static unsigned long lastSendTime = 0;
  if (millis() - lastSendTime >= 50) { 
    lastSendTime = millis();

    if (WiFi.status() == WL_CONNECTED) {
      String telemetryData = "";
      telemetryData.reserve(400); 

      telemetryData += "$";
      telemetryData += String(Du_lieu_gui_toi_ESP.Roll) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Pitch) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Yaw) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Dien_ap) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Dong_dien) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Do_cao, 1) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Ap_xuat) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Nhiet_do) + ",";
      
      telemetryData += String(GPS_data.gps_lat, 6) + ",";
      telemetryData += String(GPS_data.gps_lng, 6) + ",";
      telemetryData += String(GPS_data.gps_sat) + ",";
      telemetryData += String(GPS_data.gps_hdop, 1) + ",";
      telemetryData += String(GPS_data.gps_speed, 1) + ",";
      telemetryData += String(GPS_data.gps_alt, 1) + ",";
      telemetryData += String(GPS_data.gps_course, 1) + ",";
      
      telemetryData += String(Lenh_gui_di.Muc_Ga) + ","; 
      telemetryData += String(Lenh_gui_di.Diem_dat_Roll) + ",";
      telemetryData += String(Lenh_gui_di.Diem_dat_Pitch) + ",";
      telemetryData += String(Lenh_gui_di.Diem_dat_Yaw) + ",";
      telemetryData += String(Lenh_gui_di.Kp_roll_moi) + ",";

      //telemetryData += String(Lenh_gui_di.Ki_roll_moi) + ",";
      telemetryData += String(Lenh_gui_di.Kd_roll_moi) + ",";
      telemetryData += String(Lenh_gui_di.Kp_pitch_moi) + ",";
     //telemetryData += String(Lenh_gui_di.Ki_pitch_moi) + ",";
      telemetryData += String(Lenh_gui_di.Kd_pitch_moi) + ",";
      telemetryData += String(Lenh_gui_di.Kp_yaw_moi) + ",";
      telemetryData += String(Lenh_gui_di.Ki_yaw_moi) + ",";
      telemetryData += String(Lenh_gui_di.Trang_thai_Arm); 
      telemetryData += "*";

      udp.beginPacket(targetIP, targetPort);
      udp.print(telemetryData);  
      udp.endPacket();
    }
  }
}