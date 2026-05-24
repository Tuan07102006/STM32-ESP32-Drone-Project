#include "attitude_estimation.h"

void init_attitude_estimation(LESO_t *eso, float bo, float wo) {
    eso->b0 = bo;
    eso->wo = wo;
    
    // Phân bổ điểm cực cho hệ bậc 3
    eso->beta1 = 3.0f * wo;
    eso->beta2 = 3.0f * wo * wo;
    eso->beta3 = wo * wo * wo;
    
    eso->z1 = 0.0f;
    eso->z2 = 0.0f;
    eso->z3 = 0.0f;
}

void update_attitude_estimation(LESO_t *eso, float y_measure, float u_control, float dt) {
    // Sai số giữa góc đo được và góc ước lượng
    float error = y_measure - eso->z1;
    
    // Cập nhật trạng thái bằng phương pháp Euler
    eso->z1 += (eso->z2 + eso->beta1 * error) * dt;
    eso->z2 += (eso->z3 + eso->beta2 * error + eso->b0 * u_control) * dt;
    eso->z3 += (eso->beta3 * error) * dt;
}