

#include "../Inc/stepper_motor.h"

#ifdef UNIT_TEST
	#include "fake_hal.h"
#else
	#include "stm32f4xx_hal.h"
#endif

#define HOMING_STEP_PERIOD   10
#define HOMING_INFINITE_STEPS 0xFFFF

void stepper_motor_init(
    stepper_motor_t *motor,
    void *const tim_handler,
    const uint16_t tim_channel,
    void *const dir_port,
    const uint16_t dir_pin,
    const int step_counter,
    const int step_period
) {
    if (motor == NULL || tim_handler == NULL || dir_port == NULL) {
        return; /* invalid arguments */
    }

    motor->tim_handler = tim_handler;
    motor->tim_channel = tim_channel;
    motor->dir_port = dir_port;
    motor->dir_pin = dir_pin;
    motor->step_counter = step_counter;       /* number of steps in one move series */
    motor->tick_counter = 0;                  /* timer tick counter */
    motor->step_period = step_period;         /* ticks between two PWM pulses */
    motor->pulse = 0;
    motor->homing_step_period = HOMING_STEP_PERIOD;
    motor->is_homing = 0U;
}

void set_motor_velocity_and_dir(stepper_motor_t *motor, const t_velocity *velocity) {
    uint32_t period;
    uint8_t dir;

    if (motor == NULL || velocity == NULL) {
        return;
    }

    period = vel_get_period(*velocity);
    dir = vel_get_dir(*velocity);

    motor->step_period = (int)period;
    motor->step_counter = (int)vel_get_step_number(*velocity);
    motor->pulse = (int)(period >> 1U);

    if (dir == DIR_STOP) {
        motor->pulse = 0; /* stop PWM when direction is STOP */
    } else if (dir == DIR_LEFT) {
        HAL_GPIO_WritePin((GPIO_TypeDef *)motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    } else if (dir == DIR_RIGHT) {
        HAL_GPIO_WritePin((GPIO_TypeDef *)motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    }
}

void check_next_pulse(stepper_motor_t *motor) {
    if (motor == NULL) {
        return;
    }

    if (motor->tick_counter == motor->step_period) {
        motor->tick_counter = 0;
        motor->step_counter--;
        __HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->tim_handler, motor->tim_channel, (uint32_t)motor->pulse);
    } else {
    	__HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->tim_handler, motor->tim_channel, 0U);
        motor->tick_counter++;
    }
}

/* Start homing movement toward endstop */
void start_homing(stepper_motor_t *motor, const uint8_t dir_to_endstop) {
    if (motor == NULL) {
        return;
    }

    /* Set direction toward endstop */
    if (dir_to_endstop > 0U) {
        HAL_GPIO_WritePin((GPIO_TypeDef *)motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    } else {
        HAL_GPIO_WritePin((GPIO_TypeDef *)motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    }

    /* Set PWM signal to start motor */
    motor->pulse = HOMING_STEP_PERIOD >> 1;
    motor->tick_counter = 0;
    motor->step_counter = HOMING_INFINITE_STEPS; /* practically infinite movement toward endstop */
    motor->is_homing = 1U;
}

/* Stop motor after endstop detection */
void stop_homing(stepper_motor_t *motor) {
    if (motor == NULL) {
        return;
    }

    /* Disable PWM */
    __HAL_TIM_SET_COMPARE((TIM_HandleTypeDef *)motor->tim_handler, motor->tim_channel, 0U);
    motor->pulse = 0;
    motor->tick_counter = 0;
    motor->step_counter = 0;
    motor->is_homing = 0U;
}
