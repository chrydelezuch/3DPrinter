#include "emergency_stop.h"
#include "debug.h"

static void stop_stepper_channel(uint32_t channel)
{
    if (HAL_TIM_PWM_Stop_IT(&htim3, channel) != HAL_OK) {
        debug_error("Stepper emergency stop failed on channel %lu\r\n", (unsigned long)channel);
    }

    __HAL_TIM_SET_COMPARE(&htim3, channel, 0);
}

static void reset_stepper_channel(uint32_t channel)
{
    __HAL_TIM_SET_COMPARE(&htim3, channel, 0);

    if (HAL_TIM_PWM_Start_IT(&htim3, channel) != HAL_OK) {
        debug_error("Stepper restart failed on channel %lu\r\n", (unsigned long)channel);
    }
}

static void stop_heater_channel(uint32_t channel)
{
    __HAL_TIM_SET_COMPARE(&htim4, channel, 0);

    if (HAL_TIM_PWM_Stop(&htim4, channel) != HAL_OK) {
        debug_error("Heater emergency stop failed on channel %lu\r\n", (unsigned long)channel);
    }
}

static void reset_heater_channel(uint32_t channel)
{
    __HAL_TIM_SET_COMPARE(&htim4, channel, 0);

    if (HAL_TIM_PWM_Start(&htim4, channel) != HAL_OK) {
        debug_error("Heater restart failed on channel %lu\r\n", (unsigned long)channel);
    }
}

void Outputs_Disable(void)
{
 // TO DO
}

void Stepper_Emergency_Stop()
{
    // stoping all stepper motors pwm chanels
    stop_stepper_channel(TIM_CHANNEL_1);
    stop_stepper_channel(TIM_CHANNEL_2);
    stop_stepper_channel(TIM_CHANNEL_3);
    stop_stepper_channel(TIM_CHANNEL_4);
}

void Heaters_Emergency_Stop()
{
    stop_heater_channel(TIM_CHANNEL_1);
    stop_heater_channel(TIM_CHANNEL_2);

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
    // Reset PWM compare values and start PWM again
    reset_stepper_channel(TIM_CHANNEL_1);
    reset_stepper_channel(TIM_CHANNEL_2);
    reset_stepper_channel(TIM_CHANNEL_3);
    reset_stepper_channel(TIM_CHANNEL_4);
}

void Heaters_Emergency_Stop_Reset()
{
    // Reset heater outputs and start PWM again
    reset_heater_channel(TIM_CHANNEL_1);
    reset_heater_channel(TIM_CHANNEL_2);

    // Optional:
    // PID_Reset(&pid_controller_bed);
    // PID_Reset(&pid_controller_print_head);
}

void Emergency_Stop_Reset()
{
    Stepper_Emergency_Stop_Reset();
    Heaters_Emergency_Stop_Reset();

}
