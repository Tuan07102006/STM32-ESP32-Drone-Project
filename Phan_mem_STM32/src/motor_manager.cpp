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

  // =========================================================
  // 6. ĐỒNG BỘ HÓA PHẦN CỨNG BẰNG THANH GHI (THAY VÌ DÙNG resume)
  // =========================================================
  
  // Bước 6.1: Tạm dừng cả 2 Timer (Clear bit CEN trong thanh ghi CR1)
  TIM2->CR1 &= ~TIM_CR1_CEN;
  TIM3->CR1 &= ~TIM_CR1_CEN;

  // Bước 6.2: Ép thông số đếm của TIM3 giống hệt TIM2 
  // (Đảm bảo tần số đếm bằng nhau tuyệt đối, không phụ thuộc thư viện tính toán)
  TIM3->PSC = TIM2->PSC;
  TIM3->ARR = TIM2->ARR;

  // Bước 6.3: Reset bộ đếm về 0 để xuất phát cùng vạch
  TIM2->CNT = 0;
  TIM3->CNT = 0;

  // Bước 6.4: Khóa ngắt toàn cục (Không cho CPU làm việc khác)
  noInterrupts(); 

  // Bước 6.5: Bật 2 timer liên tiếp (Chỉ lệch 1 chu kỳ máy - cực kỳ sát nhau)
  TIM2->CR1 |= TIM_CR1_CEN;
  TIM3->CR1 |= TIM_CR1_CEN;

  // Bước 6.6: Mở lại ngắt
  interrupts();
}