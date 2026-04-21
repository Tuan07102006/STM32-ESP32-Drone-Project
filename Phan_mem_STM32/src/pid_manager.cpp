#include <Arduino.h>
#include "pid_manager.h"
#include "config.h"
#include "motor_manager.h"


extern Lenh_Dieu_Khien Lenh_tu_ESP;
extern Bo_dieu_khien_PID PID_roll, PID_pitch, PID_yaw;
extern float goc_roll_thuc_te, goc_pitch_thuc_te, goc_yaw_thuc_te;

// 1. HÀM PID CHO ROLL VÀ PITCH (Giữ nguyên gốc của bạn)

float Tinh_PID(Bo_dieu_khien_PID *P, float setpoint, float measured, float dt) {
  if (dt <= 0.0f) return 0.0f;
  float error = setpoint - measured;
  
  P->Ti_le = P->Kp * error;
  P->Tich_phan = constrain(P->Tich_phan + (P->Ki * error * dt), -P->Tich_phan_toi_da, P->Tich_phan_toi_da);
  P->Dao_ham = P->Kd * (error - P->Loi_truoc_do) / dt;
  P->Loi_truoc_do = error;
  
  return P->Ti_le + P->Tich_phan + P->Dao_ham;
}

// 2. HÀM PID THIẾT KẾ RIÊNG CHO LA BÀN (TRỤC YAW)

float Tinh_PID_Yaw(Bo_dieu_khien_PID *P, float setpoint, float measured, float dt) {
  if (dt <= 0.0f) return 0.0f;
  
  // A. Tính sai số thô và bù đường đi ngắn nhất
  float error = setpoint - measured;
  if (error > 180.0f) {
    error -= 360.0f;
  } else if (error < -180.0f) {
    error += 360.0f;
  }
  
  P->Ti_le = P->Kp * error;
  P->Tich_phan = constrain(P->Tich_phan + (P->Ki * error * dt), -P->Tich_phan_toi_da, P->Tich_phan_toi_da);
  
  // B. TÍNH ĐẠO HÀM (SỰ THAY ĐỔI GÓC)
  float delta_measured = measured - P->Loi_truoc_do;
  
  // FIX CHÍ MẠNG: Chống giật Khâu D khi góc nhảy mốc 359 -> 0
  if (delta_measured > 180.0f) {
    delta_measured -= 360.0f;
  } else if (delta_measured < -180.0f) {
    delta_measured += 360.0f;
  }
  
  P->Dao_ham = -P->Kd * (delta_measured / dt);
  
  // Cập nhật lại mốc cho vòng lặp sau
  P->Loi_truoc_do = measured;
  
  return P->Ti_le + P->Tich_phan + P->Dao_ham;
}

// 3. VÒNG LẶP ĐIỀU KHIỂN BAY CHÍNH
void processFlightControl(float dt) {
  
  PID_roll.Kp = Lenh_tu_ESP.Kp_roll_moi;
  PID_roll.Ki = Lenh_tu_ESP.Ki_roll_moi;
  PID_roll.Kd = Lenh_tu_ESP.Kd_roll_moi;

  PID_pitch.Kp = Lenh_tu_ESP.Kp_pitch_moi;
  PID_pitch.Ki = Lenh_tu_ESP.Ki_pitch_moi;
  PID_pitch.Kd = Lenh_tu_ESP.Kd_pitch_moi;

  //  BƯỚC 5 (Trong tài liệu): GIẢM ẢNH HƯỞNG PID YAW
  // Yaw quay bằng phản lực xoắn nên rất dễ rùng (oscillation). 
  // Phải triệt tiêu khâu D và giảm mạnh khâu P, I.
  PID_yaw.Kp = Lenh_tu_ESP.Kp_yaw_moi * 0.8f; 
  PID_yaw.Ki = Lenh_tu_ESP.Ki_yaw_moi * 0.5f;
  PID_yaw.Kd = 0.0f; // TRỤC YAW TUYỆT ĐỐI KHÔNG DÙNG D!

  // Tính PID Roll và Pitch
  float r_out = Tinh_PID(&PID_roll, Lenh_tu_ESP.Diem_dat_Roll, goc_roll_thuc_te, dt);
  float p_out = Tinh_PID(&PID_pitch, Lenh_tu_ESP.Diem_dat_Pitch, goc_pitch_thuc_te, dt);
  
  // PID DÙNG YAW_FUSED 
  // Đưa biến goc_yaw_thuc_te (đã được lai tạo ở imu_manager) vào tính toán
  float y_out = Tinh_PID_Yaw(&PID_yaw, Lenh_tu_ESP.Diem_dat_Yaw, goc_yaw_thuc_te, dt);

  r_out = constrain(r_out, -300.0f, 300.0f);
  p_out = constrain(p_out, -300.0f, 300.0f);
  y_out = constrain(y_out, -150.0f, 150.0f);
  // Bộ trộn tín hiệu động cơ (Motor Mixer)
  if (Lenh_tu_ESP.Trang_thai_Arm == 0) {
  // Khi disarm, reset toàn bộ tích phân
  PID_roll.Tich_phan = 0;
  PID_pitch.Tich_phan = 0;
  PID_yaw.Tich_phan = 0;
  Set_Motor_Speed(1000, 1000, 1000, 1000);
}
  else {
    // 2. NẾU ĐÃ ARM
    float thr = Lenh_tu_ESP.Muc_Ga;
    
    //  CHỐT CHẶN CHỐNG TỰ RÚ GA NẰM Ở ĐÂY 
    if (thr < 1500) {
       // KHI DRONE ĐANG NẰM TRÊN MẶT ĐẤT:
       // a. Xóa sạch khâu I để không bị tích phân sai số
       PID_roll.Tich_phan = 0;
       PID_pitch.Tich_phan = 0;
       PID_yaw.Tich_phan = 0;
       
       // b. KHÔNG CHO PHÉP P VÀ D TÁC ĐỘNG VÀO MOTOR!
       // Chỉ xuất duy nhất mức ga gốc ra 4 động cơ (quay đều nhau để chờ bay)
       Set_Motor_Speed(thr, thr, thr, thr);
    } 
    else {
       // KHI DRONE ĐÃ CẤT CÁNH:
       // Cho phép thuật toán PID nhúng tay vào để cân bằng máy bay
       Set_Motor_Speed(thr + r_out - p_out - y_out,  // Động cơ 1: Trái-Trước (CW)
                       thr + r_out + p_out + y_out,  // Động cơ 2: Trái-Sau   (CCW)
                       thr - r_out + p_out - y_out,  // Động cơ 3: Phải-Sau   (CW)
                       thr - r_out - p_out + y_out); // Động cơ 4: Phải-Trước (CCW)
    }
  }
}