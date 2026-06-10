#include <Arduino.h>
#include "sensor_manager.h"
#include "config.h"
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_INA219.h>
#include <Adafruit_BMP280.h> 

extern Goi_du_lieu Du_lieu_gui_toi_ESP;

Adafruit_INA219 ina219;
Adafruit_BMP280 bmp(BMP_CBS, &SPI); 

// Biến Calib Độ cao
float do_cao_ban_dau = 0.0f;
bool da_calib_do_cao = false;
uint8_t so_mau_calib = 0;
float tong_do_cao = 0.0f;

// 1. THÊM BIẾN BÙ TRỪ NHIỆT ĐỘ (Tính bằng độ C)
const float OFFSET_NHIET_DO = 10.0f; 

void initSensors() {
    // (Giữ nguyên toàn bộ code khởi tạo cũ) 
    pinMode(BMP_CBS, OUTPUT); 
    digitalWrite(BMP_CBS, HIGH); 
    pinMode(IMU_NCS, OUTPUT); 
    digitalWrite(IMU_NCS, HIGH); 
    
    ina219.begin(); 
    bmp.begin();
    
    bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                    Adafruit_BMP280::SAMPLING_X2,  
                    Adafruit_BMP280::SAMPLING_X16, 
                    Adafruit_BMP280::FILTER_X16,    
                    Adafruit_BMP280::STANDBY_MS_1); 
}

void readSlowSensors() {
    Du_lieu_gui_toi_ESP.Dien_ap = ina219.getBusVoltage_V();
    Du_lieu_gui_toi_ESP.Dong_dien = ina219.getCurrent_mA() / 1000.0f;
    
    float ap_suat_hientai = bmp.readPressure();
    
    // 2. CỘNG THÊM GIÁ TRỊ BÙ TRỪ VÀO KẾT QUẢ ĐỌC
    float nhiet_do_hientai = bmp.readTemperature() + OFFSET_NHIET_DO; 
    
    float docao_tho = bmp.readAltitude(1013.25);

    // Thuật toán Calib ngầm 
    if (!da_calib_do_cao) {
        if (ap_suat_hientai > 80000.0f && docao_tho < 10000.0f) {
            tong_do_cao += docao_tho;
            so_mau_calib++;
            if (so_mau_calib >= 50) { 
                do_cao_ban_dau = tong_do_cao / 50.0f;
                da_calib_do_cao = true; 
            }
        }
        Du_lieu_gui_toi_ESP.Do_cao = 0.0f; 
    } else {
        Du_lieu_gui_toi_ESP.Do_cao = docao_tho - do_cao_ban_dau;
    }
    
    Du_lieu_gui_toi_ESP.Ap_xuat = ap_suat_hientai;
    Du_lieu_gui_toi_ESP.Nhiet_do = nhiet_do_hientai;
}