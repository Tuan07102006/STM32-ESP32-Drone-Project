#include "udp_manager.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WiFi.h>

WiFiUDP udp;

// Biến toàn cục lấy từ config.h
extern Lenh_Dieu_Khien Lenh_gui_di;
extern GPS_Data GPS_data; 
extern Goi_du_lieu Du_lieu_gui_toi_ESP;

// ĐỊA CHỈ MÁY TÍNH CỦA BẠN (Sửa lại đúng IP nhà bạn nhé)
const char* targetIP = "192.168.10.55"; 
const int targetPort = 12345;

// BIẾN CHO FAILSAFE
unsigned long thoi_gian_nhan_lenh_cuoi = 0; 
const unsigned long THOI_GIAN_MAT_SONG_TOI_DA = 2000; 

void setupUDP() {
  Serial.println("UDP Ready to send!");
}

void receiveCommandsUDP() {
  int packetSize = udp.parsePacket(); 
  if (packetSize) {
    String incomingData = udp.readString();
    
    // Đặt lại thời gian nhận lệnh
    thoi_gian_nhan_lenh_cuoi = millis(); 

    if (incomingData.startsWith("#") && incomingData.endsWith("*")) {
      Serial.print("Da nhan lenh tu App: ");
      Serial.println(incomingData);
    }
  }
}

void checkFailsafe() {
  if (thoi_gian_nhan_lenh_cuoi == 0) return; 

  if (millis() - thoi_gian_nhan_lenh_cuoi > THOI_GIAN_MAT_SONG_TOI_DA) {
    if (Lenh_gui_di.Trang_thai_Arm == 1) {
      Serial.println("\n[CẢNH BÁO] !!! MẤT KẾT NỐI APP !!! KÍCH HOẠT FAILSAFE !!!");
      Lenh_gui_di.Muc_Ga = 1000; 
      Lenh_gui_di.Diem_dat_Roll = 0.0;
      Lenh_gui_di.Diem_dat_Pitch = 0.0;
      Lenh_gui_di.Diem_dat_Yaw = 0.0;
      Lenh_gui_di.Trang_thai_Arm = 0; 
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
      telemetryData += String(Du_lieu_gui_toi_ESP.Roll, 2) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Pitch, 2) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Yaw, 2) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Dien_ap, 1) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Dong_dien, 1) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Do_cao, 1) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Ap_xuat, 1) + ",";
      telemetryData += String(Du_lieu_gui_toi_ESP.Nhiet_do, 1) + ",";
      
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
      telemetryData += String(Lenh_gui_di.Ki_roll_moi) + ",";
      telemetryData += String(Lenh_gui_di.Kd_roll_moi) + ",";
      telemetryData += String(Lenh_gui_di.Kp_pitch_moi) + ",";
      telemetryData += String(Lenh_gui_di.Ki_pitch_moi) + ",";
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