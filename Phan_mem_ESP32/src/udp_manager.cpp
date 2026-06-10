#include "udp_manager.h"
#include "config.h"
#include <WiFiUdp.h>
#include <WiFi.h>
#include "memory_management.h"
#include <Preferences.h>

WiFiUDP udp;


extern Lenh_Dieu_Khien Lenh_gui_di;
extern GPS_Data GPS_data; 
extern Goi_du_lieu Du_lieu_gui_toi_ESP;
extern Preferences memory_management;

// ĐỊA CHỈ MÁY TÍNH 
const char* targetIP = "192.168.137.79"; 
const int targetPort = 34542;
const int localPort = 34542;   

// BIẾN CHO FAILSAFE
unsigned long thoi_gian_nhan_lenh_cuoi = 0; 
const unsigned long THOI_GIAN_MAT_SONG_TOI_DA = 2000; 

void setupUDP() {
  udp.begin(localPort); //  Mở cổng để bắt đầu nhận dữ liệu UDP
}

void receiveCommandsUDP() {
  int packetSize = udp.parsePacket(); 
  if (packetSize) {
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
      float r_p, r_d, p_p, p_d, y_p, y_i, dummy;
      // Yêu cầu quét chính xác 7 thông số (khớp với app Flutter)
      int parsed = sscanf(incomingData.c_str(), "@%f,%f,%f,%f,%f,%f,%f*", 
                          &r_p, &r_d, &p_p, &p_d, &y_p, &y_i, &dummy);
      
      if (parsed == 7) {
        
        
        Lenh_gui_di.Kp_roll_moi = r_p;
        Lenh_gui_di.Kd_roll_moi = r_d;
        Lenh_gui_di.Kp_pitch_moi = p_p;
        Lenh_gui_di.Kd_pitch_moi = p_d;
        Lenh_gui_di.Kp_yaw_moi = y_p;
        Lenh_gui_di.Ki_yaw_moi = y_i;
        
        memory_management.putFloat("Kp_roll", r_p);
        memory_management.putFloat("Kd_roll", r_d);
        memory_management.putFloat("Kp_pitch", p_p);
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

      // 3. Nếu ga đã xuống rất thấp, tự động DISARM sau một khoảng thời gian 
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
      char telemetryBuffer[400]; 
      

      snprintf(telemetryBuffer, sizeof(telemetryBuffer), 
        "$%.2f,%.2f,%.2f,%.2f,%.2f,%.1f,%.2f,%.2f,%.6f,%.6f,%d,%.1f,%.1f,%.1f,%.1f,%.0f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%.2f,%d*",
        Du_lieu_gui_toi_ESP.Roll, Du_lieu_gui_toi_ESP.Pitch, Du_lieu_gui_toi_ESP.Yaw,
        Du_lieu_gui_toi_ESP.Dien_ap, Du_lieu_gui_toi_ESP.Dong_dien, Du_lieu_gui_toi_ESP.Do_cao,
        Du_lieu_gui_toi_ESP.Ap_xuat, Du_lieu_gui_toi_ESP.Nhiet_do,
        GPS_data.gps_lat, GPS_data.gps_lng, GPS_data.gps_sat, GPS_data.gps_hdop,
        GPS_data.gps_speed, GPS_data.gps_alt, GPS_data.gps_course,
        Lenh_gui_di.Muc_Ga, Lenh_gui_di.Diem_dat_Roll, Lenh_gui_di.Diem_dat_Pitch, Lenh_gui_di.Diem_dat_Yaw,
        Lenh_gui_di.Kp_roll_moi, Lenh_gui_di.Kd_roll_moi,
        Lenh_gui_di.Kp_pitch_moi, Lenh_gui_di.Kd_pitch_moi,
        Lenh_gui_di.Kp_yaw_moi, Lenh_gui_di.Ki_yaw_moi,
        Lenh_gui_di.Trang_thai_Arm);

      udp.beginPacket(targetIP, targetPort);
      udp.print(telemetryBuffer);  
      udp.endPacket();
    }
  }
}