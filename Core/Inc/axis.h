#ifndef AXIS_H
#define AXIS_H

#include "stepper_motor.h"
#include "circular_buffer.h"

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

void process_axis(axis_t axis);
void axis_init(void);

#define AXIS_BUFFER_SIZE 1024

/* ===== motors ===== */

extern stepper_motor_t motor_X;
extern stepper_motor_t motor_Y;
extern stepper_motor_t motor_Z;
extern stepper_motor_t motor_extruder;

/* ===== buffers ===== */

static buffer_type x_buffer_mem[AXIS_BUFFER_SIZE];
static buffer_type y_buffer_mem[AXIS_BUFFER_SIZE];
static buffer_type z_buffer_mem[AXIS_BUFFER_SIZE];
static buffer_type e_buffer_mem[AXIS_BUFFER_SIZE];

 static circ_buf_t x_buffer;
 static circ_buf_t y_buffer;
 static circ_buf_t z_buffer;
 static circ_buf_t e_buffer;

/* ===== buffer handles ===== */

extern cbuf_handle_t x_cbuf;
extern cbuf_handle_t y_cbuf;
extern cbuf_handle_t z_cbuf;
extern cbuf_handle_t e_cbuf;

#endif
