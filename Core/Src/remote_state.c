#include "remote_state.h"

static RemoteState_t remote_state;

void remote_state_init(void)
{
    remote_state.system_state = 0;
    remote_state.error_flags = 0;
    remote_state.queue_free = 0;
}

void system_update_remote_state(uint8_t state)
{
    remote_state.system_state = state;
}

void system_update_remote_errors(uint8_t errors)
{
    remote_state.error_flags = errors;
}

void system_update_remote_queue(uint8_t queue_free)
{
    remote_state.queue_free = queue_free;
}

uint8_t remote_get_system_state(void)
{
    return remote_state.system_state;
}

uint8_t remote_get_error_flags(void)
{
    return remote_state.error_flags;
}

uint8_t remote_get_queue_free(void)
{
    return remote_state.queue_free;
}

RemoteState_t remote_get_state(void)
{
    return remote_state;
}
