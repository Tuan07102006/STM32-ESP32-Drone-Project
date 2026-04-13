#include <Arduino.h>
#include "config.h"
#include "motor_manager.h"
#include "imu_manager.h"
#include "pid_manager.h"
#include "sensor_manager.h"
#include "uart_manager.h"
#include "compass_manager.h"
#include "calibrateESC.h"

Goi_du_lieu Du_lieu_gui_toi_ESP;





// 1000(Ga), 0,0,0(Góc), 1.5,0.05,1.2(Roll), 1.5,0.05,1.2(Pitch), 2.0,0.1(Yaw), 0(Arm)
Lenh_Dieu_Khien Lenh_tu_ESP = {1000.0, 0.0, 0.0, 0.0,   1.5, 0.05, 1.2,   1.5, 0.05, 1.2,   2.0, 0.1,   0};
Bo_dieu_khien_PID PID_roll={1.5,0.05,1.2,0,0,0,0,400,0}, PID_pitch={1.5,0.05,1.2,0,0,0,0,400,0}, PID_yaw={2,0,0,0,0,0,0,400,0};
float goc_roll_thuc_te=0, goc_pitch_thuc_te=0, goc_yaw_thuc_te=0;
unsigned long time_prev = 0;

void setup() {
  initMotors();
  initCompass();
  //calibrateESC();
  //calibrateAndSave();
  initIMU();
  initSensors();
  initUART();
  
  time_prev = micros();
}

void loop() {
  unsigned long now = micros();
  // Khóa cứng chu kỳ vòng lặp ở mức 4000us (250Hz)
  if (now - time_prev < 4000) return; 
  
  float dt = (now - time_prev) / 1000000.0f; 
  
  if (dt <= 0 || dt > 0.02f) dt = 0.005f; 
  time_prev = now;

  readCompass(); 
  handleCommunication();   
  updateIMUCalculations(dt); 
  processFlightControl(dt);  

  static uint32_t slowTask = 0;
  if (millis() - slowTask > 500) {
    readSlowSensors(); 
    slowTask = millis();
  }
}