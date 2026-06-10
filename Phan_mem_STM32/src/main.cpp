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

// KHỞI TẠO BIẾN TOÀN CỤC 
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
  
  // Thiết lập I2C cho các chân trên STM32
  Wire.setSCL(I2C_SCL); 
  Wire.setSDA(I2C_SDA); 
  Wire.begin();
  
  initMotors();
  initCompass();
  // calibrateESC();       
  //calibrateAndSave();  
  initIMU();
  initSensors();
  initUART();
  
  time_prev = micros();
}

void loop() {
    uint32_t now_micros = micros();
    uint32_t now_millis = millis();

    static uint32_t last_loop_400Hz = 0;
    static uint32_t last_loop_50Hz = 0;
    static uint32_t last_loop_10Hz = 0;

    
    if (now_micros - last_loop_400Hz >= 2500) {
        
        float dt = (now_micros - last_loop_400Hz) / 1000000.0f;
        last_loop_400Hz = now_micros;

        updateIMUCalculations(dt); // Lấy dữ liệu Gyro/Accel (SPI) và tính Madgwick
        processFlightControl(dt);  // Tính toán LESO, PID và xuất lệnh ra Động cơ
    }

    
    if (now_millis - last_loop_50Hz >= 20) {
        last_loop_50Hz = now_millis;

        handleCommunication();     // Giao tiếp UART qua lại với ESP32
        readCompass();             // Đọc la bàn GY-271 (I2C)

    }

    
    if (now_millis - last_loop_10Hz >= 100) {
        last_loop_10Hz = now_millis;

        readSlowSensors();         // Đọc Baro BMP280 và INA219 (Dòng/Áp)
           
    }
}