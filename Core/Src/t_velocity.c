#include "t_velocity.h"

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
