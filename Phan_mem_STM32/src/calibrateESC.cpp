#include <Arduino.h>
#include "calibrateESC.h"
#include "config.h"
#include "motor_manager.h" 
#include "Buzzer_manager.h" // Nhớ gọi thư viện này

extern HardwareTimer *T2; 
extern HardwareTimer *T3;

void calibrateESC() {
  T2->resume(); 
  T3->resume();

  // XUẤT MAX GA (2000us)
  T3->setCaptureCompare(2, 2000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, 2000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, 2000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, 2000, MICROSEC_COMPARE_FORMAT);

  buzzer_bao_max_ga(); // Còi tự kêu 1 tiếng dài

  delay(2500); // Chờ thêm 2.5s (do còi đã chiếm 0.5s)

  // XUẤT MIN GA (1000us)
  T3->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, 1000, MICROSEC_COMPARE_FORMAT);

  delay(500);
  buzzer_bao_min_ga(); // Còi tự kêu 2 tiếng bíp

  while (true) {
    delay(1000);
  }
}