#include <Arduino.h>
#include <Wire.h> 

#include "config.h"
#include "led_manager.h"
#include "wifi_manager.h"
#include "gps_manager.h"
#include "uart_stm32.h" 
#include "udp_manager.h"
#include "memory_management.h"

// TẠO THỰC THỂ CHO CÁC BIẾN TOÀN CỤC
Goi_du_lieu Du_lieu_gui_toi_ESP;
Lenh_Dieu_Khien Lenh_gui_di = {1000.0, 0.0, 0.0, 0.0,   1.5, 0.05, 1.2,   1.5, 0.05, 1.2,   2.0, 0.1,   0};
         
//Device_Status Device_status; 

uint32_t loop_timer; // Biến lưu mốc thời gian của vòng lặp

void setup() {
  Serial.begin(115200);
  setupMemory();
  setupled();             
  setupwifi();          
  setupGPS();            
  setupUART_STM32();    
  setupUDP();   

  // Khởi tạo mốc thời gian ngay trước khi vào loop
  loop_timer = micros(); 
}

void loop() {
  updateLEDStatus(); 
  readMemory();   
  handlewifi();   
  
  receiveCommandsUDP(); 
  checkFailsafe();    
  
  readGPSRaw();                           
  readDataFromSTM32();                    
  
  sendTelemetryUDP();   
  sendDataToSTM32(); 

  while (micros() - loop_timer < 4000) {
  }
  
  loop_timer = micros(); 
}