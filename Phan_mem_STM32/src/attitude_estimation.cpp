#include "config.h"


typedef struct {
    
    float b0;
    float wo;
    float Ts;

    float beta1;
    float beta2;

    float z1;
    float z2;

}LESO_t;

void init_attitude_estimation (LESO_t*eso, float bo, float wo, float Ts){

    eso->b0 = bo;
    eso->wo = wo;
    eso->Ts = Ts;
    eso->beta1 = 2.0f*wo;
    eso->beta2 = wo*wo;
    eso->z1 = 0.0f;
    eso->z2 = 0.0f;

}
