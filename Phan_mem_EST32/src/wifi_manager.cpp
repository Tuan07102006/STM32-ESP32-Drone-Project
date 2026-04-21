#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

const char* ssid = "TrungThuy_T2_NEW";   
const char* password = "thuy@1977";
unsigned long lastReconnectMillis = 0;
const unsigned long reconnectInterval = 4000;

void setupwifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  
  Serial.print("Dang ket noi WiFi: ");
  Serial.println(ssid);

  // --- THÊM ĐOẠN NÀY ĐỂ ĐỢI KẾT NỐI ---
  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) { // Đợi tối đa 10 giây

    Serial.print(".");
    timeout++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n--- KET NOI THANH CONG ---");
    Serial.print("IP cua ESP32: ");
    Serial.println(WiFi.localIP()); 
  } else {
    Serial.println("\n--- KET NOI THAT BAI (Sai pass hoac sai SSID) ---");
  }
}

void handlewifi() {
  if (WiFi.status() != WL_CONNECTED) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastReconnectMillis >= reconnectInterval) {
        lastReconnectMillis = currentMillis;
        WiFi.disconnect(); 
        WiFi.begin(ssid, password);
    }
  }
}