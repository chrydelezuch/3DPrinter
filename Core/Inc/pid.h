#ifndef PID_H
#define PID_H

#include <stdint.h>
#include "stm32f4xx_hal.h"


typedef struct {
    float Kp;
    float Ki;
    float Kd;
    float setpoint;
    float integral;
    float last_error;
    float output_min;
    float output_max;
} PID_Controller;

void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float output_min, float output_max);

float PID_Compute(PID_Controller *pid, float measurement, float dt);

uint32_t PID_to_PWM(float pid_value, TIM_HandleTypeDef *htim, uint32_t channel);
#endif
