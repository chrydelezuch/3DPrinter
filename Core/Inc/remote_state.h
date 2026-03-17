#ifndef REMOTE_STATE_H
#define REMOTE_STATE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// ###
// Remote system state structure
// ###

typedef struct
{
    uint8_t system_state;
    uint8_t error_flags;
    uint8_t queue_free;
} RemoteState_t;


// ###
// Initialization
// ###

void remote_state_init(void);


// ###
// Field update functions
// ###

void system_update_remote_state(uint8_t state);
void system_update_remote_errors(uint8_t errors);
void system_update_remote_queue(uint8_t queue_free);


// ###
// Getter functions
// ###
uint8_t remote_get_system_state(void);
uint8_t remote_get_error_flags(void);
uint8_t remote_get_queue_free(void);


// ###
// Get complete remote state
// ###

RemoteState_t remote_get_state(void);


#ifdef __cplusplus
}
#endif

#endif
