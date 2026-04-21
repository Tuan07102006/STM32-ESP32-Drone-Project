#include "gps_manager.h"
#include "config.h" 
#include <Arduino.h>
#include <MicroNMEA.h>

char nmeaBuffer[85];
MicroNMEA nmea(nmeaBuffer, sizeof(nmeaBuffer));

HardwareSerial gpsSerial(2); 

void setupGPS() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, CHAN_NOI_GPS_RX, CHAN_NOI_GPS_TX);
}

void readGPSRaw() {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    
    // MẸO GỠ LỖI 1: Bỏ dấu // ở dòng dưới đây nếu bạn muốn xem mạch có đang nhận được chữ nào từ GPS không
    // Serial.print(c); 

    if (nmea.process(c)) {
      // Đã giải mã xong 1 cục dữ liệu NMEA
      

      if (nmea.isValid()) {
        GPS_data.gps_lat = nmea.getLatitude() / 1000000.0;
        GPS_data.gps_lng = nmea.getLongitude() / 1000000.0;
      } 
      
      GPS_data.gps_sat = nmea.getNumSatellites();
      GPS_data.gps_hdop = nmea.getHDOP() / 10.0;
      GPS_data.gps_speed = (nmea.getSpeed() / 1000.0) * 0.514444;
      
      long alt;
      if (nmea.getAltitude(alt)) {
        GPS_data.gps_alt = alt / 1000.0; 
      }
      GPS_data.gps_course = nmea.getCourse() / 1000.0;
    }
  }
}