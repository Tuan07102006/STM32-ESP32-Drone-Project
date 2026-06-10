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
    static uint32_t lastByteTime = 0;
    static uint8_t buffer[sizeof(Goi_du_lieu)];
    static uint8_t bufIndex = 0;
    static enum { WAIT_AA, WAIT_BB, RECEIVING } state = WAIT_AA;

    // Reset nếu kẹt khung quá 10ms
    if (millis() - lastByteTime > 10 && state == RECEIVING) {
        state = WAIT_AA; 
    }

    while (SerialSTM.available()) {
        uint8_t c = SerialSTM.read();
        lastByteTime = millis();

        switch (state) {
            case WAIT_AA:
                if (c == 0xAA) state = WAIT_BB;
                break;
            case WAIT_BB:
                if (c == 0xBB) {
                    state = RECEIVING;
                    bufIndex = 0;
                } else if (c != 0xAA) {
                    state = WAIT_AA;
                }
                break;
            case RECEIVING:
                buffer[bufIndex++] = c;
                if (bufIndex >= sizeof(Goi_du_lieu)) {
                  
                    memcpy(&Du_lieu_gui_toi_ESP, buffer, sizeof(Goi_du_lieu));
                    state = WAIT_AA; 
                }
                break;
        }
    }
}