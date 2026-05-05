#ifndef BUZZER_MANAGER_H
#define BUZZER_MANAGER_H

#include <Arduino.h>

void khoi_tao_buzzer();
void bat_buzzer();
void tat_buzzer();

// Thêm các hàm nhịp điệu tự động
void buzzer_bao_max_ga();
void buzzer_bao_min_ga();
void buzzer_bat_dau_calib();
void buzzer_ket_thuc_calib();

#endif