#include "emergency_stop.h"



void Outputs_Disable(void)
{
 // TO DO
}

void Stepper_Emergency_Stop()
{
    // stoping all stepper motors pwm chanels

    HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_4);

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);

}

void Heaters_Emergency_Stop()
{
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);

    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop(&htim4, TIM_CHANNEL_2);

    //PID_Reset(&pid_controller_bed);
    //PID_Reset(&pid_controller_print_head);
}


void Emergency_Stop_Activate()
{
	Stepper_Emergency_Stop();
	Heaters_Emergency_Stop();
	currentState = STATE_STOP;
}


void Stepper_Emergency_Stop_Reset()
{
    // Reset PWM compare values
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0);
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_4, 0);

    // Start PWM again
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_4);
}


void Heaters_Emergency_Stop_Reset()
{
    // Reset heater outputs
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(&htim4, TIM_CHANNEL_2, 0);

    // Start PWM again
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_2);

    // Optional:
    // PID_Reset(&pid_controller_bed);
    // PID_Reset(&pid_controller_print_head);
}


void Emergency_Stop_Reset()
{
    Stepper_Emergency_Stop_Reset();
    Heaters_Emergency_Stop_Reset();

}