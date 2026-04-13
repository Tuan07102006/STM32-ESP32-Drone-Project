#include <Arduino.h>
#include "calibrateESC.h"
#include "config.h"
#include "motor_manager.h" 

// Kéo 2 con trỏ Timer từ bộ nhớ của motor_manager.cpp sang đây để điều khiển
extern HardwareTimer *T2; 
extern HardwareTimer *T3;

void calibrateESC() {
  // Đảm bảo Timer đã chạy
  T2->resume(); 
  T3->resume();


  // XUẤT MAX GA (250us)
  T3->setCaptureCompare(2, 2000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, 2000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, 2000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, 2000, MICROSEC_COMPARE_FORMAT);

  
  // TỰ ĐỘNG CHỜ 3 GIÂY (Cho phép ESC kêu tít tít nhận Max Ga)
  delay(3000); 

  
  // XUẤT MIN GA (125us)
  T3->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(2, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(3, 1000, MICROSEC_COMPARE_FORMAT);
  T2->setCaptureCompare(4, 1000, MICROSEC_COMPARE_FORMAT);


  // Khóa cứng chương trình ở đây để không bay loạn xạ, bảo vệ an toàn
  while (true) {
    delay(1000);
  }
}