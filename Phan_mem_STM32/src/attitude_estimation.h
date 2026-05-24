#ifndef ATTITUDE_ESTIMATION_H
#define ATTITUDE_ESTIMATION_H

#include <Arduino.h>

typedef struct {
    float b0;     // Hệ số khuếch đại hệ thống
    float wo;     // Băng thông bộ quan sát

    float beta1;  // Lợi ích quan sát 1
    float beta2;  // Lợi ích quan sát 2
    float beta3;  // Lợi ích quan sát 3

    float z1;     // Góc ước lượng
    float z2;     // Vận tốc góc ước lượng
    float z3;     // Nhiễu tổng ước lượng (Quan trọng nhất)
} LESO_t;

void init_attitude_estimation(LESO_t *eso, float bo, float wo);
void update_attitude_estimation(LESO_t *eso, float y_measure, float u_control, float dt);

#endif