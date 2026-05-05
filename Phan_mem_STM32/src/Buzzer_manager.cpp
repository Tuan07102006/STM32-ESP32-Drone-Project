#include "Buzzer_manager.h"
#include "config.h" 

void khoi_tao_buzzer() {
    pinMode(Buzzer, OUTPUT);
    digitalWrite(Buzzer, LOW); // Đảm bảo còi tắt khi mới khởi động
}

void bat_buzzer() {
    digitalWrite(Buzzer, HIGH);
}

void tat_buzzer() {
    digitalWrite(Buzzer, LOW);
}

// 1 tiếng bíp dài báo Max Ga
void buzzer_bao_max_ga() {
    bat_buzzer();
    delay(500);
    tat_buzzer();
}

// 2 tiếng bíp ngắn báo Min Ga (Hoàn tất ESC)
void buzzer_bao_min_ga() {
    bat_buzzer(); delay(150); tat_buzzer(); delay(150);
    bat_buzzer(); delay(150); tat_buzzer();
}

// 3 tiếng bíp ngắn báo hiệu bắt đầu Calib la bàn
void buzzer_bat_dau_calib() {
    for(int i = 0; i < 3; i++) {
        bat_buzzer(); delay(150); tat_buzzer(); delay(150);
    }
}

// 1 tiếng bíp cực dài báo hiệu Calib la bàn thành công
void buzzer_ket_thuc_calib() {
    bat_buzzer();
    delay(1000);
    tat_buzzer();
}