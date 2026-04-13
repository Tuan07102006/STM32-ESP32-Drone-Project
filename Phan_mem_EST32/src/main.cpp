#include <Arduino.h>
#include <Wire.h> 

// --- NHÚNG CÁC MODULE ĐÃ TÁCH ---
#include "config.h"
#include "led_manager.h"
#include "wifi_server.h"
#include "gps_manager.h"
#include "uart_stm32.h" 

// TẠO THỰC THỂ CHO CÁC BIẾN TOÀN CỤC (Đã hứa extern ở config.h)
Goi_du_lieu Du_lieu_gui_toi_ESP;
Lenh_Dieu_Khien Lenh_gui_di = {125.0, 0.0, 0.0, 0.0,   1.5, 0.05, 1.2,   1.5, 0.05, 1.2,   2.0, 0.1,   0};

void setup() {
  // 1. Khởi tạo cổng Serial để debug trên máy tính
  Serial.begin(115200);
  Serial.println("\n=== KHỞI ĐỘNG TRẠM MẶT ĐẤT ESP32-S3 (GCS) ===");

  // 2. Khởi tạo đường truyền I2C (Thay đổi chân SDA, SCL nếu cần)
  Wire.begin(8, 9); 

  // 3. Đánh thức từng module phần cứng và phần mềm
  initLED();             // Bật đèn LED
  setupWiFiAndWeb();     // Phát WiFi và bật WebServer
  initGPS();             // Mở cổng UART2 đọc NEO-8M
  initUART_STM32();      // Mở cổng UART1 nói chuyện với STM32

  Serial.println(">>> HỆ THỐNG ĐÃ SẴN SÀNG HOẠT ĐỘNG! <<<");
}

void loop() {
  updateLEDStatus(getConnectedDevices()); // Cập nhật nháy LED
  readGPSRaw();                           // Đọc tọa độ GPS
  readDataFromSTM32();                    // Nhận Telemetry từ STM32
  sendDataToSTM32();                      // Gửi lệnh điều khiển xuống STM32
  sendTelemetryToWeb();                   // Bơm dữ liệu lên Web (GCS)
  cleanupWebSocket();                     // Dọn dẹp RAM cho WebSocket
}