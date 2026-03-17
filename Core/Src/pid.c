#include "pid.h"

void PID_Init(PID_Controller *pid, float Kp, float Ki, float Kd, float output_min, float output_max) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->integral = 0.0f;
    pid->last_error = 0.0f;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->setpoint = 0.0f;
}

float PID_Compute(PID_Controller *pid, float measurement, float dt) {
    float error = pid->setpoint - measurement;
    pid->integral += error * dt;
    float derivative = (error - pid->last_error) / dt;
    float output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;


    if(output > pid->output_max) output = pid->output_max;
    if(output < pid->output_min) output = pid->output_min;

    pid->last_error = error;
    return output;
}

uint32_t PID_to_PWM(float pid_value, TIM_HandleTypeDef * htim, uint32_t channel) {

    // calcualte pwm in range 0-ARR
    uint32_t pwm_val = (uint32_t)((pid_value / 100.0f) * htim->Init.Period);

    // set pwm on timers
    __HAL_TIM_SET_COMPARE(htim, channel, pwm_val);

    return pwm_val;
}
