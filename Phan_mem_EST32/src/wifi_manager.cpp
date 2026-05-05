#include "wifi_manager.h"
#include "config.h"
#include <WiFi.h>

const char* ssid = "LAN";   
const char* password = "12345678";
unsigned long lastReconnectMillis = 0;
const unsigned long reconnectInterval = 4000;

// Khai báo cấu hình IP Tĩnh
//IPAddress local_IP(10, 76, 12, 54);  // Cố định IP cho ESP32-S3
//IPAddress gateway(10, 76, 12, 1);
//IPAddress subnet(255, 255, 255, 0);

void setupwifi() {
  WiFi.mode(WIFI_STA);
  WiFi.setSleep(false);
  

  WiFi.begin(ssid, password);
  
  Serial.print("Dang ket noi WiFi: ");
  Serial.println(ssid);

  int timeout = 0;
  while (WiFi.status() != WL_CONNECTED && timeout < 20) {
    Serial.print(".");
    timeout++;
    delay(500);
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