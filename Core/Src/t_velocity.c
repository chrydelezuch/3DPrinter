#include "t_velocity.h"

// bits 0..1 are direction
// bits 2..15 are step number
// bits 16..31 are period

uint8_t vel_get_dir(t_velocity vel)
{
    return (uint8_t)(vel & 0x00000003U);
}


uint32_t vel_get_period(t_velocity vel)
{
    return (vel & 0xFFFF0000U) >> 16U;
}

uint16_t vel_get_step_number(t_velocity vel)
{
    return (uint16_t)((vel & 0x0000FFFCU) >> 2U);
}
