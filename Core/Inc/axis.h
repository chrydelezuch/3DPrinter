#ifndef AXIS_H
#define AXIS_H

#include "stepper_motor.h"
#include "circular_buffer.h"

#include <stdint.h>
#include "stm32f4xx_hal.h"

typedef enum
{
    AXIS_X = 0,
    AXIS_Y,
    AXIS_Z,
    AXIS_E,
    AXIS_COUNT
} axis_t;

typedef struct
{
    stepper_motor_t *motor;
    cbuf_handle_t buffer;
    uint32_t channel;
} Axis;

extern Axis axis_map[AXIS_COUNT];
extern axis_t channel_axis_map[AXIS_COUNT];

#define AXIS_BUFFER_SIZE 1024

void process_axis(axis_t axis);
void axis_init(TIM_HandleTypeDef *const tim_handler);



/* ===== motors ===== */

extern stepper_motor_t motor_X;
extern stepper_motor_t motor_Y;
extern stepper_motor_t motor_Z;
extern stepper_motor_t motor_extruder;



#endif
