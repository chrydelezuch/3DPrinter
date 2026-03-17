#include "axis.h"


stepper_motor_t motor_X;
stepper_motor_t motor_Y;
stepper_motor_t motor_Z;
stepper_motor_t motor_extruder;

static uint8_t x_buffer_mem[AXIS_BUFFER_SIZE];
static uint8_t y_buffer_mem[AXIS_BUFFER_SIZE];
static uint8_t z_buffer_mem[AXIS_BUFFER_SIZE];
static uint8_t e_buffer_mem[AXIS_BUFFER_SIZE];

static circ_buf_t x_buffer;
static circ_buf_t y_buffer;
static circ_buf_t z_buffer;
static circ_buf_t e_buffer;



Axis axis_map[AXIS_COUNT] =
{
    { &motor_X,        &x_buffer, TIM_CHANNEL_1 },
    { &motor_Y,        &y_buffer, TIM_CHANNEL_2 },
    { &motor_Z,        &z_buffer, TIM_CHANNEL_3 },
    { &motor_extruder, &e_buffer, TIM_CHANNEL_4 }
};

axis_t channel_axis_map[AXIS_COUNT] =
{
    AXIS_X,
    AXIS_Y,
    AXIS_Z,
    AXIS_E
};

inline void process_axis(axis_t axis)
{
     stepper_motor_t *m = axis_map[axis].motor;

    check_next_pulse(m);

    if (m->step_counter <= 0)
    {
        t_velocity v;

        if (circ_buf_pop(axis_map[axis].buffer, (uint8_t*)&v) == 0)
        {
            set_motor_velocity_and_dir(m, &v);
        }
    }
}

void axis_init(TIM_HandleTypeDef *const tim_handler)
{


    circ_buf_init(&x_buffer, x_buffer_mem, AXIS_BUFFER_SIZE, 4);
    circ_buf_init(&y_buffer, y_buffer_mem, AXIS_BUFFER_SIZE, 4);
    circ_buf_init(&z_buffer, z_buffer_mem, AXIS_BUFFER_SIZE, 4);
    circ_buf_init(&e_buffer, e_buffer_mem, AXIS_BUFFER_SIZE, 4);


    stepper_motor_init(&motor_X, tim_handler, TIM_CHANNEL_1,
                     DIR_X_GPIO_Port, DIR_X_Pin, 0, 1);

    stepper_motor_init(&motor_Y, tim_handler, TIM_CHANNEL_2,
                     DIR_Y_GPIO_Port, DIR_Y_Pin, 0, 1);

    stepper_motor_init(&motor_Z, tim_handler, TIM_CHANNEL_3,
                     DIR_Z_GPIO_Port, DIR_Z_Pin, 0, 1);

    stepper_motor_init(&motor_extruder, tim_handler, TIM_CHANNEL_4,
                     DIR_EXTR_GPIO_Port, DIR_EXTR_Pin, 0, 1);
}

