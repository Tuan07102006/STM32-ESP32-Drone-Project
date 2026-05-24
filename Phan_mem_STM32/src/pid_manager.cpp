#include <Arduino.h>
#include "pid_manager.h"
#include "config.h"
#include "motor_manager.h"
#include "attitude_estimation.h"

LESO_t leso_roll;
LESO_t leso_pitch;
bool is_leso_init = false;

extern Lenh_Dieu_Khien Lenh_tu_ESP;
extern Bo_dieu_khien_PID PID_roll, PID_pitch, PID_yaw;
extern float goc_roll_thuc_te, goc_pitch_thuc_te, goc_yaw_thuc_te;

enum TrangThaiBay {
    STATE_DISARMED,
    STATE_GROUND_IDLE,
    STATE_FLYING
};


// 1. HÀM PID CHO ROLL VÀ PITCH 
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
  
  // Chống giật Khâu D khi góc nhảy mốc 359 -> 0
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
  
  float thr = Lenh_tu_ESP.Muc_Ga;

  // 1. TÍNH TOÁN TPA (THROTTLE PID ATTENUATION)
  const float TPA_BREAKPOINT = 1450.0f; // Điểm ga bắt đầu giảm PID (Tùy chỉnh theo điểm Hover)
  const float TPA_FACTOR = 0.30f;       // Giảm tối đa 30% tại mức ga cao nhất
  
  float tpa_multiplier = 1.0f;          // Mặc định giữ nguyên 100% sức mạnh PID
  
  if (thr > TPA_BREAKPOINT) {
      // Tính tỷ lệ % ga vượt ngưỡng (từ 0.0 đến 1.0)
      float over_throttle_ratio = (thr - TPA_BREAKPOINT) / (2000.0f - TPA_BREAKPOINT);
      over_throttle_ratio = constrain(over_throttle_ratio, 0.0f, 1.0f);
      
      // Suy giảm dần multiplier
      tpa_multiplier = 1.0f - (over_throttle_ratio * TPA_FACTOR);
  }

  // 2. CẬP NHẬT THÔNG SỐ BỘ ĐIỀU KHIỂN
  // Áp dụng TPA làm "mềm" khâu P và D của Roll, Pitch khi ga cao
  PID_roll.Kp = Lenh_tu_ESP.Kp_roll_moi * tpa_multiplier;
  PID_roll.Kd = Lenh_tu_ESP.Kd_roll_moi * tpa_multiplier;

  PID_pitch.Kp = Lenh_tu_ESP.Kp_pitch_moi * tpa_multiplier;
  PID_pitch.Kd = Lenh_tu_ESP.Kd_pitch_moi * tpa_multiplier;

  // Trục Yaw không cần TPA, cấu hình fix cứng để chống xoay vòng
  PID_yaw.Kp = Lenh_tu_ESP.Kp_yaw_moi * 0.8f; 
  PID_yaw.Ki = Lenh_tu_ESP.Ki_yaw_moi * 0.5f;
  PID_yaw.Kd = 0.0f; // TUYỆT ĐỐI KHÔNG DÙNG D CHO YAW

  // 3. TÍNH TOÁN BỘ QUAN SÁT VÀ LUẬT ĐIỀU KHIỂN (ADRC)
  if (!is_leso_init) {
      init_attitude_estimation(&leso_roll, 20.0f, 40.0f);
      init_attitude_estimation(&leso_pitch, 20.0f, 40.0f);
      is_leso_init = true;
  }

  static float last_r_out = 0.0f;
  static float last_p_out = 0.0f;

  // Cập nhật LESO
  update_attitude_estimation(&leso_roll, goc_roll_thuc_te, last_r_out, dt);
  update_attitude_estimation(&leso_pitch, goc_pitch_thuc_te, last_p_out, dt);

  // Tính PD bù nhiễu
  float error_roll = Lenh_tu_ESP.Diem_dat_Roll - leso_roll.z1;
  float u0_roll = (PID_roll.Kp * error_roll) - (PID_roll.Kd * leso_roll.z2); 
  float r_out = (u0_roll - leso_roll.z3) / leso_roll.b0;

  float error_pitch = Lenh_tu_ESP.Diem_dat_Pitch - leso_pitch.z1;
  float u0_pitch = (PID_pitch.Kp * error_pitch) - (PID_pitch.Kd * leso_pitch.z2);
  float p_out = (u0_pitch - leso_pitch.z3) / leso_pitch.b0;

  float y_out = Tinh_PID_Yaw(&PID_yaw, Lenh_tu_ESP.Diem_dat_Yaw, goc_yaw_thuc_te, dt); 

  // Giới hạn an toàn và lưu lại cho chu kỳ LESO sau
  r_out = constrain(r_out, -300.0f, 300.0f);
  p_out = constrain(p_out, -300.0f, 300.0f);
  y_out = constrain(y_out, -150.0f, 150.0f);

  last_r_out = r_out;
  last_p_out = p_out;

  // 4. MÁY TRẠNG THÁI (FLIGHT STATE MACHINE)
  static TrangThaiBay trang_thai_hien_tai = STATE_DISARMED;

  // Nhận diện trạng thái
  if (Lenh_tu_ESP.Trang_thai_Arm == 0) {
      trang_thai_hien_tai = STATE_DISARMED;
  } else {
      if (trang_thai_hien_tai == STATE_DISARMED) {
          trang_thai_hien_tai = STATE_GROUND_IDLE; 
      }
      if (trang_thai_hien_tai == STATE_GROUND_IDLE && thr > 1250) {
          trang_thai_hien_tai = STATE_FLYING;
      }
      if (trang_thai_hien_tai == STATE_FLYING && thr < 1100) {
          trang_thai_hien_tai = STATE_GROUND_IDLE;
      }
  }

  // Thực thi theo trạng thái
  switch (trang_thai_hien_tai) {
      case STATE_DISARMED:
          PID_yaw.Tich_phan = 0;
          leso_roll.z1 = leso_roll.z2 = leso_roll.z3 = 0.0f;
          leso_pitch.z1 = leso_pitch.z2 = leso_pitch.z3 = 0.0f;
          last_r_out = last_p_out = 0.0f;
          
          Set_Motor_Speed(1000, 1000, 1000, 1000);
          break;

      case STATE_GROUND_IDLE:
          // Xả lỗi tích lũy để chống lật (Ground Spool-up)
          PID_yaw.Tich_phan = 0;
          leso_roll.z3 = 0.0f;  
          leso_pitch.z3 = 0.0f;
          last_r_out = last_p_out = 0.0f; 

          Set_Motor_Speed(1050, 1050, 1050, 1050); 
          break;

      case STATE_FLYING:
          // Trộn tín hiệu ra 4 động cơ (X Copter Form)
          Set_Motor_Speed(thr + r_out - p_out - y_out,  
                          thr + r_out + p_out + y_out,  
                          thr - r_out + p_out - y_out,  
                          thr - r_out - p_out + y_out); 
          break;
  }
}