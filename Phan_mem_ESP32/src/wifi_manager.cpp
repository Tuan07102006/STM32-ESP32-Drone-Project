#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

const char* ssid = "LAN";   
const char* password = "12345678";
unsigned long lastReconnectMillis = 0;
const unsigned long reconnectInterval = 4000;

// THÊM: Ép IP Tĩnh khớp với Flutter (_targetIP = "192.168.137.116")
IPAddress local_IP(192, 168, 137, 116);  
IPAddress gateway(192, 168, 137, 1);
IPAddress subnet(255, 255, 255, 0);

void setupwifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  
  // Áp dụng IP tĩnh trước khi kết nối WiFi
  if (!WiFi.config(local_IP, gateway, subnet)) {
    Serial.println("Loi cau hinh IP Tinh!");
  }

  WiFi.begin(ssid, password);
  
  Serial.print("Dang ket noi WiFi: ");
  Serial.println(ssid);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    Serial.print(".");
    delay(500); // Thêm delay nhỏ để tránh treo watchdog
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n--- KET NOI THANH CONG ---");
    Serial.print("IP cua ESP32: ");
    Serial.println(WiFi.localIP()); // Lúc này chắc chắn phải là 192.168.137.116
  } else {
    Serial.println("\n--- KET NOI THAT BAI ---");
  }
}

void handlewifi() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastReconnectMillis >= reconnectInterval) {
        lastReconnectMillis = currentMillis;
        WiFi.disconnect(); 
        WiFi.config(local_IP, gateway, subnet); // Đảm bảo cấu hình lại khi đứt mạng
        WiFi.begin(ssid, password);
    }
  }
}