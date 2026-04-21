#include <Arduino.h>
#include "sensor_manager.h"
#include "config.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <Adafruit_BMP280.h>

extern Goi_du_lieu Du_lieu_gui_toi_ESP;

Adafruit_INA219 ina219;
Adafruit_BMP280 bmp(BMP_CBS);

void initSensors() {
    pinMode(BMP_CBS, OUTPUT); digitalWrite(BMP_CBS, HIGH);
    
    ina219.begin(); 
    bmp.begin();
  
}

void readSlowSensors() {
    Du_lieu_gui_toi_ESP.Dien_ap = ina219.getBusVoltage_V();
    Du_lieu_gui_toi_ESP.Dong_dien = ina219.getCurrent_mA();
    Du_lieu_gui_toi_ESP.Do_cao = bmp.readAltitude(1013.25);
    Du_lieu_gui_toi_ESP.Ap_xuat = bmp.readPressure();
   
}