#include "gps_manager.h"
#include "config.h" 
#include <Arduino.h>

char nmeaBuffer[100];
int bufferIndex = 0;

extern GPS_Data GPS_data;

HardwareSerial gpsSerial(2); 

void setupGPS() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, CHAN_NOI_GPS_RX, CHAN_NOI_GPS_TX);
}

// Hàm kiểm tra tính toàn vẹn của dữ liệu NMEA (Checksum Validation)
bool validateChecksum(const char* sentence) {
  if (sentence[0] != '$') return false; // Câu NMEA luôn bắt đầu bằng '$'
  
  int checksum = 0;
  int i = 1;
  
  // XOR tất cả các ký tự giữa '$' và '*'
  while (sentence[i] != '*' && sentence[i] != '\0') {
    checksum ^= sentence[i];
    i++;
  }
  
  // Nếu tìm thấy dấu '*', so sánh checksum tính được với checksum trong chuỗi
  if (sentence[i] == '*') {
    int receivedChecksum = strtol(&sentence[i + 1], NULL, 16);
    return checksum == receivedChecksum;
  }
  return false;
}

// Hàm phụ trợ 1: Lấy chuỗi con nằm giữa các dấu phẩy
void getField(const char* str, int fieldIndex, char* output, int maxLen) {
  int currentField = 0;
  int outIdx = 0;
  output[0] = '\0'; // Xóa chuỗi output
  
  for (int i = 0; str[i] != '\0' && str[i] != '*'; i++) { // Bỏ qua phần checksum
    if (str[i] == ',') {
      currentField++;
      if (currentField > fieldIndex) break;
      continue;
    }
    if (currentField == fieldIndex && outIdx < maxLen - 1) {
      output[outIdx++] = str[i];
      output[outIdx] = '\0';
    }
  }
}

// Hàm phụ trợ 2: Chuyển đổi định dạng NMEA (DDMM.MMMM) sang độ thập phân
float nmeaToDecimal(const char* nmeaCoord, const char* dir) {
  if (strlen(nmeaCoord) < 4) return 0.0;
  
  // Tìm vị trí dấu chấm thập phân
  const char* dot = strchr(nmeaCoord, '.');
  if (!dot) return 0.0;
  
  int dotIdx = dot - nmeaCoord;
  int degreeDigits = dotIdx - 2; // Vĩ độ có 2 số chỉ độ, Kinh độ có 3 số
  
  char degStr[4] = {0};
  strncpy(degStr, nmeaCoord, degreeDigits);
  float degrees = atof(degStr); // Số độ
  
  float minutes = atof(nmeaCoord + degreeDigits); // Số phút
  float decimal = degrees + (minutes / 60.0f);
  
  // Nếu là hướng Nam (S) hoặc Tây (W) thì tọa độ mang giá trị âm
  if (dir[0] == 'S' || dir[0] == 'W') {
    decimal = -decimal;
  }
  return decimal;
}

// Hàm phân tích 1 dòng lệnh NMEA hoàn chỉnh
void parseNMEA(const char* sentence) {
  char header[7];
  getField(sentence, 0, header, sizeof(header));

  // Phân tích câu $GNRMC hoặc $GPRMC (Lấy Tọa độ, Vận tốc, Hướng)
  if (strncmp(header, "$GNRMC", 6) == 0 || strncmp(header, "$GPRMC", 6) == 0) {
    char status[2];
    getField(sentence, 2, status, sizeof(status));
    
    if (status[0] == 'A') { // 'A' = Active (Đã chốt được vị trí)
      char latStr[15], latDir[2], lonStr[15], lonDir[2], speedStr[10], courseStr[10];
      
      getField(sentence, 3, latStr, sizeof(latStr));
      getField(sentence, 4, latDir, sizeof(latDir));
      getField(sentence, 5, lonStr, sizeof(lonStr));
      getField(sentence, 6, lonDir, sizeof(lonDir));
      getField(sentence, 7, speedStr, sizeof(speedStr));
      getField(sentence, 8, courseStr, sizeof(courseStr));

      GPS_data.gps_lat = nmeaToDecimal(latStr, latDir);
      GPS_data.gps_lng = nmeaToDecimal(lonStr, lonDir);
      GPS_data.gps_speed = atof(speedStr) * 0.514444; // Đổi từ knots sang m/s
      GPS_data.gps_course = atof(courseStr);
    }
  }
  // Phân tích câu $GNGGA hoặc $GPGGA (Lấy Số vệ tinh, HDOP, Độ cao)
  else if (strncmp(header, "$GNGGA", 6) == 0 || strncmp(header, "$GPGGA", 6) == 0) {
    char quality[2], satStr[4], hdopStr[6], altStr[10];
    
    getField(sentence, 6, quality, sizeof(quality));
    
    if (quality[0] > '0') { // Nếu chất lượng > 0 (nghĩa là đã fix 3D/2D)
      getField(sentence, 7, satStr, sizeof(satStr));
      getField(sentence, 8, hdopStr, sizeof(hdopStr));
      getField(sentence, 9, altStr, sizeof(altStr));

      GPS_data.gps_sat = atoi(satStr);
      GPS_data.gps_hdop = atof(hdopStr);
      GPS_data.gps_alt = atof(altStr);
    } else {
      GPS_data.gps_sat = 0; // Đặt lại số vệ tinh về 0 nếu mất tín hiệu
    }
  }
}

// Hàm đọc và xử lý GPS (Nên trả về true nếu có dữ liệu mới để main loop tiện xử lý)
bool readGPSRaw() {
  bool newDataParsed = false;

  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    
    // Bắt đầu một câu NMEA mới
    if (c == '$') {
      bufferIndex = 0;
    }
    
    // Tránh tràn mảng
    if (bufferIndex < sizeof(nmeaBuffer) - 1) {
      nmeaBuffer[bufferIndex++] = c;
    }
    
    // Khi nhận được ký tự kết thúc dòng (Carriage Return hoặc Line Feed)
    if (c == '\r' || c == '\n') {
      if (bufferIndex > 5) { // Đảm bảo dòng có đủ dữ liệu cơ bản
        nmeaBuffer[bufferIndex] = '\0'; // Đóng chuỗi
        
        // CHỈ PHÂN TÍCH KHI CHECKSUM HỢP LỆ
        if (validateChecksum(nmeaBuffer)) {
          parseNMEA(nmeaBuffer);
          newDataParsed = true;
        }
      }
      bufferIndex = 0; // Reset index cho câu tiếp theo
    }
  }

  if (newDataParsed) {
    /* 
    Serial.print(GPS_data.gps_lat, 6); Serial.print(", ");
    Serial.print(GPS_data.gps_lng, 6); Serial.print(", ");
    Serial.print(GPS_data.gps_sat); Serial.print(", ");
    Serial.print(GPS_data.gps_hdop, 1); Serial.print(", ");
    Serial.println(GPS_data.gps_alt, 1);
    */
  }

  return newDataParsed;
}