#include <Arduino.h>
#include <Wire.h> 

#include "config.h"
#include "led_manager.h"
#include "wifi_manager.h"
#include "gps_manager.h"
#include "uart_stm32.h" 

// TẠO THỰC THỂ CHO CÁC BIẾN TOÀN CỤC
Goi_du_lieu Du_lieu_gui_toi_ESP;
Lenh_Dieu_Khien Lenh_gui_di = {125.0, 0.0, 0.0, 0.0,   1.5, 0.05, 1.2,   1.5, 0.05, 1.2,   2.0, 0.1,   0};

GPS_Data GPS_data;           
Device_Status Device_status; 

void setup() {
  Serial.begin(115200);

  initLED();             
  SetupWiFi();          
  initGPS();             
  initUART_STM32();      

}

void loop() {
  updateLEDStatus(); 
  handleWiFi();      
  readGPSRaw();                           
  readDataFromSTM32();                    
  sendDataToSTM32();                      
}