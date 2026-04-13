#include "uart_stm32.h"
#include "config.h" 
#include "wifi_manager.h" 
#include <Arduino.h>

extern Lenh_Dieu_Khien Lenh_gui_di;
extern Goi_du_lieu Du_lieu_gui_toi_ESP;

HardwareSerial SerialSTM(1);

void setupUART_STM32() {
  // Cấu hình Serial cho ESP32-S3 kết nối với STM32
  SerialSTM.begin(115200, SERIAL_8N1, STM32_RX, STM32_TX);
  SerialSTM.setTimeout(10); 
}

void sendDataToSTM32() {
  static unsigned long thoi_gian_gui_lenh = 0;
  if (millis() - thoi_gian_gui_lenh >= 20) {
    SerialSTM.write(0xAA);
    SerialSTM.write(0xBB);
    SerialSTM.write((uint8_t*)&Lenh_gui_di, sizeof(Lenh_Dieu_Khien));
    thoi_gian_gui_lenh = millis();
  }
}

void readDataFromSTM32() {
  // Bộ lọc quét trượt: Chỉ đọc khi thấy đúng 0xAA 0xBB
  while (SerialSTM.available() > 0) {
    if (SerialSTM.read() == 0xAA) {
      uint32_t t = millis();
      // Chờ đủ gói dữ liệu (sizeof Goi_du_lieu + 1 byte 0xBB)
      while (SerialSTM.available() < sizeof(Goi_du_lieu) + 1) {
        if (millis() - t > 5) return; 
      }
      
      if (SerialSTM.peek() == 0xBB) {
        SerialSTM.read();
        SerialSTM.readBytes((uint8_t*)&Du_lieu_gui_toi_ESP, sizeof(Goi_du_lieu));
      }
    }
  }
}