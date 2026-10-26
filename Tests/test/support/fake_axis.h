/* test/support/mock_axis.h
 * Podmienia prawdziwy axis.h w buildzie testowym.
 * Udostępnia axis_map + minimalny circ_buf, bez HAL/stepper.
 */
#ifndef MOCK_AXIS_H
#define MOCK_AXIS_H

#include <stdint.h>
#include "circ_buf.h"

#define AXIS_COUNT       4
#define AXIS_BUFFER_SIZE 1024

typedef enum {
    AXIS_X = 0,
    AXIS_Y,
    AXIS_Z,
    AXIS_E
} axis_t;

typedef struct {
    void           *motor;      /* nieużywane w testach */
    cbuf_handle_t   buffer;
    uint32_t        channel;
} Axis;

extern Axis axis_map[AXIS_COUNT];

/* Funkcje z axis.h, które usb_praser NIE wywołuje — puste stuby */
void process_axis(axis_t axis);
void axis_init(void *tim_handler);

#endif
