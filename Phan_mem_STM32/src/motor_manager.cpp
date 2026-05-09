#include <Arduino.h>
#include "motor_manager.h"
#include "config.h"

extern Lenh_Dieu_Khien Lenh_tu_ESP;

HardwareTimer *T2;
HardwareTimer *T3;

void initMotors() {
  // 1. Đảm bảo chân PWM ở mức LOW trước khi cấu hình timer
  pinMode(Dong_co_1, OUTPUT); digitalWrite(Dong_co_1, LOW);
  pinMode(Dong_co_2, OUTPUT); digitalWrite(Dong_co_2, LOW);
  pinMode(Dong_co_3, OUTPUT); digitalWrite(Dong_co_3, LOW);
  pinMode(Dong_co_4, OUTPUT); digitalWrite(Dong_co_4, LOW);

  // 2. Khởi tạo timer
  T2 = new HardwareTimer(TIM2);
  T3 = new HardwareTimer(TIM3);

  // 3. Cấu hình tần số
  T2->setOverflow(400, HERTZ_FORMAT);
  T3->setOverflow(400, HERTZ_FORMAT);

  // 4. Gán kênh PWM cho chân
  T3->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_1);
  T2->setMode(2, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_2);
  T2->setMode(3, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_3);
  T2->setMode(4, TIMER_OUTPUT_COMPARE_PWM1, Dong_co_4);

  // 5. Khởi tạo mức ga (throttle) thấp nhất
  T3->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, 1000, MICROSEC_COMPARE_FORMAT);

  // 6. ĐỒNG BỘ HÓA PHẦN CỨNG BẰNG THANH GHI
  TIM2->CR1 &= ~TIM_CR1_CEN;
  TIM3->CR1 &= ~TIM_CR1_CEN;

  TIM3->PSC = TIM2->PSC;
  TIM3->ARR = TIM2->ARR;

  TIM2->CNT = 0;
  TIM3->CNT = 0;

  noInterrupts(); 

  TIM2->CR1 |= TIM_CR1_CEN;
  TIM3->CR1 |= TIM_CR1_CEN;

  interrupts();
}

void Set_Motor_Speed(float m1, float m2, float m3, float m4) {
  if (Lenh_tu_ESP.Trang_thai_Arm == 0) {
    // Khi chưa arm, giữ min
    T3->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
    T2->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
    T2->setCaptureCompare(3, 1000, MICROSEC_COMPARE_FORMAT);
    T2->setCaptureCompare(4, 1000, MICROSEC_COMPARE_FORMAT);
    return;
  }

  uint32_t safe_m1 = constrain((int)m1, 1000, 2000);
  uint32_t safe_m2 = constrain((int)m2, 1000, 2000);
  uint32_t safe_m3 = constrain((int)m3, 1000, 2000);
  uint32_t safe_m4 = constrain((int)m4, 1000, 2000);

  T3->setCaptureCompare(2, safe_m1, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, safe_m2, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, safe_m3, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, safe_m4, MICROSEC_COMPARE_FORMAT);
}