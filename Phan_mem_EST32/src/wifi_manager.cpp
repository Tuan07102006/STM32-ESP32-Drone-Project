#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

const char* ssid = "TrungThuy_T2_NEW";   
const char* password = "thuy@1977";

unsigned long lastReconnectMillis = 0;
const unsigned long reconnectInterval = 4000;

void SetupWiFi() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println(WiFi.localIP()); 
}
void handleWiFi() {
  if (WiFi.status() != WL_CONNECTED) {
        unsigned long currentMillis = millis();
        
        // Chỉ gọi WiFi.begin() sau mỗi khoảng thời gian nhất định (5s)
        if (currentMillis - lastReconnectMillis >= reconnectInterval) {
            lastReconnectMillis = currentMillis;
            WiFi.disconnect(); // Ngắt để làm sạch trước khi thử lại
            WiFi.begin(ssid, password);
        }
    }
}