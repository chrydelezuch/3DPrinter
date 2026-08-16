#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include "t_velocity.h"
#include <stddef.h>  // Dla NULL
#include <stdint.h>  // Dla uint8_t, uint16_t, etc.




typedef struct {
    void *tim_handler;
    uint16_t tim_channel;
    void *dir_port;
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
    void *const tim_handler,
    const uint16_t tim_channel,
    void *const dir_port,
    const uint16_t dir_pin,
    const int step_counter,
    const int step_period
);

void set_motor_velocity_and_dir(stepper_motor_t *motor, const t_velocity *velocity);

void check_next_pulse(stepper_motor_t *motor);

void start_homing(stepper_motor_t *motor, const uint8_t dir_to_endstop);

void stop_homing(stepper_motor_t *motor);

#endif
