#ifndef T_VELOCITY_H
#define T_VELOCITY_H

#include "main.h"
typedef uint32_t t_velocity;


uint8_t vel_get_dir(t_velocity vel);
uint16_t vel_get_period(t_velocity vel);
uint16_t vel_get_step_number(t_velocity vel);

typedef enum
{
    DIR_STOP = 0,
    DIR_LEFT = 1,
    DIR_RIGHT = 2,
} velocity_dir_t;


#endif
