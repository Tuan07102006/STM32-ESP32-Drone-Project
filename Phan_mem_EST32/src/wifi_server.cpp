#include "wifi_server.h"
#include "config.h"
#include "web_page.h"
#include "gps_manager.h" 
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

extern Lenh_Dieu_Khien Lenh_gui_di;
extern Goi_du_lieu Du_lieu_gui_toi_ESP;
extern double gps_lat, gps_lng, gps_hdop, gps_speed, gps_alt, gps_course;
extern int gps_sat;

const char* ten_wifi_phat = "ESP32_S3";   
const char* mat_khau_phat = "12345678";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) {
  AwsFrameInfo *info = (AwsFrameInfo*)arg;
  
  // Chỉ xử lý khi nhận được đầy đủ 1 frame dữ liệu dạng TEXT
  if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
    
    DynamicJsonDocument doc(512);
    
    // AN TOÀN BỘ NHỚ: Truyền chính xác con trỏ 'data' và độ dài 'len' 
    // để ArduinoJson không đọc tràn ra ngoài vùng nhớ rác
    DeserializationError err = deserializeJson(doc, (const char*)data, len);
    
    if (!err) {
      // ÉP KIỂU CHÍNH XÁC (.as<type>) ĐỂ TRÁNH LỖI PHÂN TÍCH CỦA ESP32
      
      // 1. Phân tích lệnh bay (Flight Commands)
      if (doc.containsKey("arm")) {
        Lenh_gui_di.Trang_thai_Arm = doc["arm"].as<int>();
      }
      if (doc.containsKey("thr")) {
        // Khóa chặt ga ở mức an toàn của PWM (1000us - 2000us)
        Lenh_gui_di.Muc_Ga = constrain(doc["thr"].as<float>(), 1000.0f, 2000.0f);
      }
      if (doc.containsKey("rs")) {
        // Khóa biên độ Roll/Pitch không quá 45 độ để chống lật máy bay
        Lenh_gui_di.Diem_dat_Roll = constrain(doc["rs"].as<float>(), -45.0f, 45.0f);
      }
      if (doc.containsKey("ps")) {
        Lenh_gui_di.Diem_dat_Pitch = constrain(doc["ps"].as<float>(), -45.0f, 45.0f);
      }
      if (doc.containsKey("ys")) {
        Lenh_gui_di.Diem_dat_Yaw = doc["ys"].as<float>();
      }

      // 2. Phân tích lệnh cập nhật PID
      if (doc.containsKey("kp_roll")) Lenh_gui_di.Kp_roll_moi = doc["kp_roll"].as<float>();
      if (doc.containsKey("ki_roll")) Lenh_gui_di.Ki_roll_moi = doc["ki_roll"].as<float>();
      if (doc.containsKey("kd_roll")) Lenh_gui_di.Kd_roll_moi = doc["kd_roll"].as<float>();
      
      if (doc.containsKey("kp_pitch")) Lenh_gui_di.Kp_pitch_moi = doc["kp_pitch"].as<float>();
      if (doc.containsKey("ki_pitch")) Lenh_gui_di.Ki_pitch_moi = doc["ki_pitch"].as<float>();
      if (doc.containsKey("kd_pitch")) Lenh_gui_di.Kd_pitch_moi = doc["kd_pitch"].as<float>();
      
      if (doc.containsKey("kp_yaw")) Lenh_gui_di.Kp_yaw_moi = doc["kp_yaw"].as<float>();
      if (doc.containsKey("ki_yaw")) Lenh_gui_di.Ki_yaw_moi = doc["ki_yaw"].as<float>();
    }
  }
}

void setupWiFiAndWeb() {
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ten_wifi_phat, mat_khau_phat);
  
  ws.onEvent([](AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
    if (type == WS_EVT_DATA) {
      handleWebSocketMessage(arg, data, len);
    } 
    else if (type == WS_EVT_DISCONNECT) {
      // [TÍNH NĂNG SINH TỒN - FAILSAFE]
      // Nếu điện thoại mất sóng, tắt nguồn, văng app... sự kiện này sẽ kích hoạt ngay.
      if (server->count() == 0) { 
        Lenh_gui_di.Trang_thai_Arm = 0;   // Ngay lập tức khóa động cơ
        Lenh_gui_di.Muc_Ga = 1000.0f;     // Trả ga về mức 0
        Lenh_gui_di.Diem_dat_Roll = 0.0f; // Cân bằng lại máy bay
        Lenh_gui_di.Diem_dat_Pitch = 0.0f;
      }
    }
  });
  
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });
  server.begin(); 
}

void cleanupWebSocket() { 
  ws.cleanupClients(); 
}

int getConnectedDevices() { 
  return WiFi.softAPgetStationNum(); 
}

void sendTelemetryToWeb() {
  // BẢO VỆ CPU: Nếu không có thiết bị nào kết nối Web, bỏ qua việc đóng gói JSON
  if (ws.count() == 0) return; 

  static unsigned long thoi_gian_gui_web = 0;
  // Gửi với tốc độ 10Hz (100ms/lần). Tránh gửi quá nhanh làm sập hàng đợi của WiFi ESP32
  if (millis() - thoi_gian_gui_web >= 100) { 
    DynamicJsonDocument doc(1024);
    
    doc["rs"] = Lenh_gui_di.Diem_dat_Roll;
    doc["rf"] = Du_lieu_gui_toi_ESP.Roll;        
    doc["rr"] = Du_lieu_gui_toi_ESP.Gia_toc_x;   
    doc["ps"] = Lenh_gui_di.Diem_dat_Pitch;
    doc["pf"] = Du_lieu_gui_toi_ESP.Pitch;
    doc["pr"] = Du_lieu_gui_toi_ESP.Gia_toc_y;
    doc["ys"] = Lenh_gui_di.Diem_dat_Yaw;
    doc["yf"] = Du_lieu_gui_toi_ESP.Yaw; 
    doc["yr"] = Du_lieu_gui_toi_ESP.Gia_toc_z;
    
    doc["v"]  = Du_lieu_gui_toi_ESP.Dien_ap;
    doc["c"]  = Du_lieu_gui_toi_ESP.Dong_dien;
    doc["alt"]= Du_lieu_gui_toi_ESP.Do_cao;
    doc["tmp"]= Du_lieu_gui_toi_ESP.Nhiet_do;
    doc["prs"]= Du_lieu_gui_toi_ESP.Ap_xuat;
    doc["ax"] = Du_lieu_gui_toi_ESP.Gia_toc_x;
    doc["ay"] = Du_lieu_gui_toi_ESP.Gia_toc_y;
    doc["az"] = Du_lieu_gui_toi_ESP.Gia_toc_z;

    doc["lat"]  = gps_lat; 
    doc["lng"]  = gps_lng; 
    doc["sat"]  = gps_sat;
    doc["hdop"] = gps_hdop; 
    doc["spd"]  = gps_speed; 
    doc["gAlt"] = gps_alt; 
    doc["cog"]  = gps_course;

    String json_string;
    serializeJson(doc, json_string);
    ws.textAll(json_string); 
    
    thoi_gian_gui_web = millis();
  }
}