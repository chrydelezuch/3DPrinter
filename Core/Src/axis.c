#include "axis.h"
#include "main.h"

stepper_motor_t motor_X;
stepper_motor_t motor_Y;
stepper_motor_t motor_Z;
stepper_motor_t motor_extruder;

cbuf_handle_t x_cbuf;
cbuf_handle_t y_cbuf;
cbuf_handle_t z_cbuf;
cbuf_handle_t e_cbuf;

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

        if (circ_buf_pop(axis_map[axis].buffer, (buffer_type*)&v) == 0)
        {
            set_motor_velocity_and_dir(m, &v);
        }
    }
}

void axis_init(void)
{


    circ_buf_init(x_cbuf, x_buffer_mem, AXIS_BUFFER_SIZE);
    circ_buf_init(y_cbuf, y_buffer_mem, AXIS_BUFFER_SIZE);
    circ_buf_init(z_cbuf, z_buffer_mem, AXIS_BUFFER_SIZE);
    circ_buf_init(e_cbuf, e_buffer_mem, AXIS_BUFFER_SIZE);


    stepper_motor_init(&motor_X, &axis_map[0].channel, TIM_CHANNEL_1,
                     DIR_X_GPIO_Port, DIR_X_Pin, 0, 1);

    stepper_motor_init(&motor_Y, &axis_map[0].channel, TIM_CHANNEL_2,
                     DIR_Y_GPIO_Port, DIR_Y_Pin, 0, 1);

    stepper_motor_init(&motor_Z, &axis_map[0].channel, TIM_CHANNEL_3,
                     DIR_Z_GPIO_Port, DIR_Z_Pin, 0, 1);

    stepper_motor_init(&motor_extruder, &axis_map[0].channel, TIM_CHANNEL_4,
                     DIR_EXTR_GPIO_Port, DIR_EXTR_Pin, 0, 1);
}

