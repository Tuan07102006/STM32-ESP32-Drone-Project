#include "memory_management.h"
#include "config.h"
#include <Preferences.h>

Preferences memory_management;

extern Lenh_Dieu_Khien Lenh_gui_di;

void setupMemory() {
  memory_management.begin("cau_hinh", false); 
}

void readMemory() {

    Lenh_gui_di.Kp_pitch_moi = memory_management.getFloat("Kp_pitch", 1.2f);
    //Lenh_gui_di.Ki_pitch_moi = memory_management.getFloat("Ki_pitch", 1.5f);
    Lenh_gui_di.Kd_pitch_moi = memory_management.getFloat("Kd_pitch", 0.05f);
    
    Lenh_gui_di.Kp_roll_moi = memory_management.getFloat("Kp_roll", 1.2f);   
    //Lenh_gui_di.Ki_roll_moi = memory_management.getFloat("Ki_roll", 0.0f);
    Lenh_gui_di.Kd_roll_moi = memory_management.getFloat("Kd_roll", 0.05f);

    Lenh_gui_di.Kp_yaw_moi = memory_management.getFloat("Kp_yaw", 2.0f);
    Lenh_gui_di.Ki_yaw_moi = memory_management.getFloat("Ki_yaw", 2.0f);
}