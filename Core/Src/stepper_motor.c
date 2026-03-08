#include "../Inc/stepper_motor.h"

void stepper_motor_init(
    stepper_motor_t *motor,
    TIM_HandleTypeDef *const tim_handler,
    const uint16_t tim_channel,
    GPIO_TypeDef *const dir_port,
    const uint16_t dir_pin,
    const int step_counter,
    const int step_period
) {
    motor->tim_handler = tim_handler;
    motor->tim_channel = tim_channel;
    motor->dir_port = dir_port;
    motor->dir_pin = dir_pin;
    motor->step_counter = step_counter;      // liczba kroków w jednej serii
    motor->tick_counter = 0;                 // licznik impulsów timera
    motor->step_period = step_period;        // liczba ticków między dwoma impulsami PWM
    motor->pulse = 0;
    motor->homing_step_period = 10;
    motor->is_homing = 0;
}

void set_motor_velocity_and_dir(stepper_motor_t *motor, const t_velocity *velocity) {
    motor->step_period = vel_get_period(*velocity);
    motor->step_counter = vel_get_step_number(*velocity);;
    motor->pulse = vel_get_period(*velocity) >> 1;

    if (vel_get_dir(velocity) == DIR_STOP ) {
        motor->pulse = 0; // zatrzymanie PWM jeśli kierunek = 0
    }
    else if (vel_get_dir(velocity) == DIR_LEFT) {
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    }
    else if (vel_get_dir(velocity) == DIR_RIGHT) {
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);
    }
}

void check_next_pulse(stepper_motor_t *motor) {
    if (motor->tick_counter == motor->step_period) {
        motor->tick_counter = 0;
        motor->step_counter--;
        __HAL_TIM_SET_COMPARE(motor->tim_handler, motor->tim_channel, motor->pulse);
    } else {
        __HAL_TIM_SET_COMPARE(motor->tim_handler, motor->tim_channel, 0);
        motor->tick_counter++;
    }
}

// Funkcja uruchamia ruch homing w kierunku endstopu
void start_homing(stepper_motor_t *motor, const uint8_t dir_to_endstop) {
    // ustaw kierunek w stronę endstopu
    if (dir_to_endstop > 0)
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(motor->dir_port, motor->dir_pin, GPIO_PIN_RESET);

    // ustaw sygnał PWM aby silnik ruszył
    motor->pulse = motor->homing_step_period >> 1;
    motor->tick_counter = 0;
    motor->step_counter = 0xFFFF; // praktycznie nieskończony ruch w stronę endstopu
    motor->is_homing = 1;
}

// Funkcja zatrzymuje silnik po wykryciu endstopu
void stop_homing(stepper_motor_t *motor) {
    // wyłącz PWM
    __HAL_TIM_SET_COMPARE(motor->tim_handler, motor->tim_channel, 0);
    motor->pulse = 0;
    motor->tick_counter = 0;
    motor->step_counter = 0;
    motor->is_homing = 0;

    // opcjonalnie ustaw aktualną pozycję jako 0
    // motor->current_position = 0; // jeśli masz pole current_position
}
