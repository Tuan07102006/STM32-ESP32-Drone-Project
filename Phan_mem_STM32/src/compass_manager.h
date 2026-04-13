#ifndef COMPASS_MANAGER_H
#define COMPASS_MANAGER_H

#include <Arduino.h>

// Địa chỉ I2C của chip QMC5883P thế hệ mới
#define QMC5883P_ADDR 0x2C

#define EEPROM_ADDR_OFFSET_X   0
#define EEPROM_ADDR_OFFSET_Y   4
#define EEPROM_ADDR_OFFSET_Z   8
#define EEPROM_ADDR_SCALE_X   12
#define EEPROM_ADDR_SCALE_Y   16
#define EEPROM_ADDR_SCALE_Z   20
#define EEPROM_SIGNATURE      24

// Khai báo các hàm sử dụng
void initCompass();
void readCompass();
void calibrateAndSave();

#endif