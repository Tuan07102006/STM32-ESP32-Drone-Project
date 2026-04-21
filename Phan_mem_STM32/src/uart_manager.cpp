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
    
    // Khai báo 3 trạng thái chờ dữ liệu
    static enum { WAIT_AA, WAIT_BB, RECEIVING } state = WAIT_AA;

    // 1. MÁY TRẠNG THÁI ĐỌC UART CHỐNG TRƯỢT KHUNG
    while (Serial1.available()) {
        uint8_t c = Serial1.read();

        switch (state) {
            case WAIT_AA:
                if (c == 0xAA) state = WAIT_BB; // Thấy đầu kéo, chờ toa số 1
                break;
                
            case WAIT_BB:
                if (c == 0xBB) {
                    state = RECEIVING; // Thấy toa số 1, bắt đầu nhận hàng
                    bufIndex = 0;
                } else if (c == 0xAA) {
                    state = WAIT_BB; // Xử lý lỗi trùng lặp byte
                } else {
                    state = WAIT_AA; // Lỗi, quay về chờ đầu kéo
                }
                break;
                
            case RECEIVING:
                buffer[bufIndex++] = c; // Đút dữ liệu vào mảng
                
                // Khi đã nhận đủ số byte của Struct
                if (bufIndex >= sizeof(Lenh_Dieu_Khien)) {
                    memcpy(&Lenh_tu_ESP, buffer, sizeof(Lenh_Dieu_Khien)); // Chép sang biến chính
                    lastRecvTime = millis(); // Đánh dấu thời gian sống!
                    state = WAIT_AA; // Reset để chờ gói tiếp theo
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