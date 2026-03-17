#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "t_velocity.h"

typedef struct {
    TIM_HandleTypeDef *tim_handler;
    uint16_t tim_channel;
    GPIO_TypeDef *dir_port;
    uint16_t dir_pin;
    int step_counter;
    int tick_counter;
    int step_period;
    int pulse;
    int homing_step_period;
    uint8_t is_homing;
} stepper_motor_t;

void stepper_motor_init(
    stepper_motor_t *motor,
    TIM_HandleTypeDef *const tim_handler,
    const uint16_t tim_channel,
    GPIO_TypeDef *const dir_port,
    const uint16_t dir_pin,
    const int step_counter,
    const int step_period
);

void set_motor_velocity_and_dir(stepper_motor_t *motor, const t_velocity *velocity);

void check_next_pulse(stepper_motor_t *motor);

void start_homing(stepper_motor_t *motor, const uint8_t dir_to_endstop);

void stop_homing(stepper_motor_t *motor);

#endif
