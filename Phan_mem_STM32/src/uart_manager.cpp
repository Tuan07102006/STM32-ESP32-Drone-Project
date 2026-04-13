#include <Arduino.h>
#include "uart_manager.h"
#include "config.h"

extern Lenh_Dieu_Khien Lenh_tu_ESP;
extern Goi_du_lieu Du_lieu_gui_toi_ESP;

void initUART() {
  Serial1.begin(115200);
}

void handleCommunication() {
    static uint32_t lastRecvTime = 0;
    static uint8_t buffer[sizeof(Lenh_Dieu_Khien)];
    static uint8_t bufIndex = 0;
    static bool headerFound = false;

    // 1. Đọc từng byte từ Serial1
    while (Serial1.available()) {
        uint8_t c = Serial1.read();

        if (!headerFound) {
            if (c == 0xAA) headerFound = true;
        } else {
            if (c == 0xBB && bufIndex == 0) {
                // Đã có 0xAA 0xBB, bắt đầu nhận payload
                // Không làm gì thêm, chỉ reset buffer nếu cần
            } else {
                // Lưu byte vào buffer
                if (bufIndex < sizeof(Lenh_Dieu_Khien)) {
                    buffer[bufIndex++] = c;
                }

                // Khi đã nhận đủ payload
                if (bufIndex >= sizeof(Lenh_Dieu_Khien)) {
                    memcpy(&Lenh_tu_ESP, buffer, sizeof(Lenh_Dieu_Khien));
                    lastRecvTime = millis();
                    headerFound = false;
                    bufIndex = 0;
                }
            }
        }
    }

    // 2. Timeout failsafe
    if (millis() - lastRecvTime > 500) {
        Lenh_tu_ESP.Trang_thai_Arm = 0;
        Lenh_tu_ESP.Muc_Ga = 1000.0f;
    }

    // 3. Gửi telemetry (giữ nguyên)
    static uint32_t lastUART = 0;
    if (millis() - lastUART >= 20) {
        Serial1.write(0xAA);
        Serial1.write(0xBB);
        Serial1.write((uint8_t*)&Du_lieu_gui_toi_ESP, sizeof(Goi_du_lieu));
        lastUART = millis();
    }
}