#include "gps_manager.h"
#include "config.h" 
#include <Arduino.h>
#include <TinyGPS++.h>

TinyGPSPlus gps;
HardwareSerial gpsSerial(2); 

void setupGPS() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, CHAN_NOI_GPS_RX, CHAN_NOI_GPS_TX);
}

void readGPSRaw() {
  while (gpsSerial.available() > 0) {
    if (gps.encode(gpsSerial.read())) {
      
      // Lưu thẳng vào struct GPS_data
      if (gps.location.isValid()) {
        GPS_data.gps_lat = gps.location.lat();
        GPS_data.gps_lng = gps.location.lng();
      }
      if (gps.satellites.isValid()) {
        GPS_data.gps_sat = gps.satellites.value();
      }
      if (gps.hdop.isValid()) {
        GPS_data.gps_hdop = gps.hdop.hdop();
      }
      if (gps.speed.isValid()) {
        GPS_data.gps_speed = gps.speed.mps(); 
      }
      if (gps.altitude.isValid()) {
        GPS_data.gps_alt = gps.altitude.meters();
      }
      if (gps.course.isValid()) {
        GPS_data.gps_course = gps.course.deg();
      }
    }
  }
}