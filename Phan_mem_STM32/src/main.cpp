#include <Arduino.h>
#include <Wire.h>
#include "config.h"
#include "motor_manager.h"
#include "imu_manager.h"
#include "pid_manager.h"
#include "sensor_manager.h"
#include "uart_manager.h"
#include "compass_manager.h"
#include "calibrateESC.h"
#include "Buzzer_manager.h"

// --- KHỞI TẠO BIẾN TOÀN CỤC ---
Goi_du_lieu Du_lieu_gui_toi_ESP;

// 1000(Ga), 0,0,0(Góc), 1.5,0.05,1.2(Roll), 1.5,0.05,1.2(Pitch), 2.0,0.1(Yaw), 0(Arm)
Lenh_Dieu_Khien Lenh_tu_ESP = {1000.0, 0.0, 0.0, 0.0, 1.5, 0.05, 1.2, 1.5, 0.05, 1.2, 2.0, 0.1, 0};

Bo_dieu_khien_PID PID_roll  = {1.5, 0.05, 1.2, 0, 0, 0, 0, 400, 0};
Bo_dieu_khien_PID PID_pitch = {1.5, 0.05, 1.2, 0, 0, 0, 0, 400, 0};
Bo_dieu_khien_PID PID_yaw   = {2.0, 0.00, 0.0, 0, 0, 0, 0, 400, 0}; 

float goc_roll_thuc_te = 0.0f;
float goc_pitch_thuc_te = 0.0f; 
float goc_yaw_thuc_te = 0.0f;

unsigned long time_prev = 0;

void setup() {
  khoi_tao_buzzer();
  
  // Thiết lập I2C cho các chân được remap trên STM32
  Wire.setSCL(I2C_SCL); 
  Wire.setSDA(I2C_SDA); 
  Wire.begin();
  
  initMotors();
  initCompass();
  // calibrateESC();       // Bỏ comment khi cần Calib ESC
  //calibrateAndSave();   // Bỏ comment khi cần Calib La bàn
  initIMU();
  initSensors();
  initUART();
  
  time_prev = micros();
}

void loop() {
  unsigned long now = micros();
  
  // Khóa cứng chu kỳ vòng lặp ở mức 4000us (250Hz)
  if (now - time_prev < 4000) return; 
  
  // Tính toán thời gian delta (dt) ra đơn vị giây
  float dt = (now - time_prev) / 1000000.0f; 
  
  if (dt <= 0.0f || dt > 0.02f) {
    dt = 0.004f; 
  }
  time_prev = now;

  // --- VÒNG LẶP ĐIỀU KHIỂN CHÍNH (250Hz) ---
  readCompass(); 
  handleCommunication();   
  updateIMUCalculations(dt); 
  processFlightControl(dt);  

  // --- VÒNG LẶP ĐỌC CẢM BIẾN CHẬM (2Hz) ---
  static uint32_t slowTask = 0;
  if (millis() - slowTask > 500) {
    readSlowSensors(); 
    slowTask = millis();
  }
}