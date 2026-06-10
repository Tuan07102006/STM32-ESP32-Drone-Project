#include <Arduino.h>
#include "motor_manager.h"
#include "config.h"

extern Lenh_Dieu_Khien Lenh_tu_ESP;

HardwareTimer *T2;
HardwareTimer *T3;

void initMotors() {
  pinMode(Dong_co_1, OUTPUT); digitalWrite(Dong_co_1, LOW);
  pinMode(Dong_co_2, OUTPUT); digitalWrite(Dong_co_2, LOW);
  pinMode(Dong_co_3, OUTPUT); digitalWrite(Dong_co_3, LOW);
  pinMode(Dong_co_4, OUTPUT); digitalWrite(Dong_co_4, LOW);

  T2 = new HardwareTimer(TIM2);
  T3 = new HardwareTimer(TIM3);

  T2->setOverflow(400, HERTZ_FORMAT);
  T3->setOverflow(400, HERTZ_FORMAT);

  T3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_1);
  T2->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_2);
  T2->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_3);
  T2->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_4);
  
  T3->resume();
  T2->resume();
}

void Set_Motor_Speed(float m1, float m2, float m3, float m4) {
  if (Lenh_tu_ESP.Trang_thai_Arm == 0) {
    // KHI CHƯA ARM: Xuất 1000us để ngắt điện hoàn toàn
    T3->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
    T2->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
    T2->setCaptureCompare(3, 1000, MICROSEC_COMPARE_FORMAT);
    T2->setCaptureCompare(4, 1000, MICROSEC_COMPARE_FORMAT);
    return;
  }

  // KHI ĐÃ ARM (Ground Idle & Flying):
  // Ép giới hạn dưới về mức Idle (ví dụ: 1050) thay vì 1000
  const uint32_t MIN_THROTTLE = 1050; // Thay đổi theo mức quay chờ thực tế của motor
  const uint32_t MAX_THROTTLE = 2000;

  uint32_t safe_m1 = constrain((int)m1, MIN_THROTTLE, MAX_THROTTLE);
  uint32_t safe_m2 = constrain((int)m2, MIN_THROTTLE, MAX_THROTTLE);
  uint32_t safe_m3 = constrain((int)m3, MIN_THROTTLE, MAX_THROTTLE);
  uint32_t safe_m4 = constrain((int)m4, MIN_THROTTLE, MAX_THROTTLE);

  T3->setCaptureCompare(2, safe_m1, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, safe_m2, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, safe_m3, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, safe_m4, MICROSEC_COMPARE_FORMAT);
}