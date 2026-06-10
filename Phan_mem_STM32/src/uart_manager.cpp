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
    static uint32_t lastByteTime = 0; 
    static uint8_t buffer[sizeof(Lenh_Dieu_Khien)];
    static uint8_t bufIndex = 0;
    static enum { WAIT_AA, WAIT_BB, RECEIVING } state = WAIT_AA;

    // Reset bộ đệm nếu quá 10ms không nhận được byte nào (chống kẹt khung)
    if (millis() - lastByteTime > 10 && state == RECEIVING) {
        state = WAIT_AA; 
    }

    while (Serial1.available()) {
        uint8_t c = Serial1.read();
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
                if (bufIndex >= sizeof(Lenh_Dieu_Khien)) {
                    memcpy(&Lenh_tu_ESP, buffer, sizeof(Lenh_Dieu_Khien));
                    lastRecvTime = millis();
                    state = WAIT_AA;
                }
                break;
        }
    }

    // 2. FAILSAFE: CẮT ĐỘNG CƠ NẾU MẤT SÓNG QUÁ 500ms
    if (millis() - lastRecvTime > 500) {
        Lenh_tu_ESP.Trang_thai_Arm = 0;
        Lenh_tu_ESP.Muc_Ga = 1000.0f;
    }

    // 3. GỬI TELEMETRY CHO ESP32 (Giữ nguyên)
    static uint32_t lastUART = 0;
    if (millis() - lastUART >= 20) { // Gửi với tốc độ 50Hz
        Serial1.write(0xAA);
        Serial1.write(0xBB);
        Serial1.write((uint8_t*)&Du_lieu_gui_toi_ESP, sizeof(Goi_du_lieu));
        lastUART = millis();
    }
}