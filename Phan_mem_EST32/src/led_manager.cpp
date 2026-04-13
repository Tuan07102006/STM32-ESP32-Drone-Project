#include "led_manager.h"
#include "config.h" 
#include <Adafruit_NeoPixel.h>
#include <WiFi.h>

Adafruit_NeoPixel led_thong_bao(SO_BONG_LED, CHAN_NOI_LED, NEO_GRB + NEO_KHZ800);  

unsigned long thoi_gian_nhay_led_truoc = 0;  
bool trang_thai_led = false;

void setupled() {
  led_thong_bao.begin();
  led_thong_bao.setBrightness(50);
  led_thong_bao.setPixelColor(0, led_thong_bao.Color(255, 255, 255)); // Trắng khi khởi động
  led_thong_bao.show();
}

void updateLEDStatus() { // Đã xóa tham số ở đây
  if (WiFi.status() != WL_CONNECTED) { // Tự kiểm tra WiFi trực tiếp
    if (millis() - thoi_gian_nhay_led_truoc >= 400) {
      thoi_gian_nhay_led_truoc = millis();
      trang_thai_led = !trang_thai_led;
      if (trang_thai_led) led_thong_bao.setPixelColor(0, led_thong_bao.Color(255, 0, 0)); // Đỏ nhấp nháy
      else led_thong_bao.setPixelColor(0, led_thong_bao.Color(0, 0, 0));
      led_thong_bao.show();
    }
  } else {
    led_thong_bao.setPixelColor(0, led_thong_bao.Color(0, 0, 255)); // Xanh dương đứng im
    led_thong_bao.show();
  }
}