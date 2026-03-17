#ifndef CIRC_BUF_H
#define CIRC_BUF_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "t_velocity.h"
#include "stm32f4xx_hal.h"

#include <string.h>






typedef struct {
    uint8_t * buffer;  /* pointer to buffer memory */
    volatile int head;
    volatile int tail;
    int max_len;
    size_t elem_size;
    uint8_t leftover[64];
    size_t leftover_size;
} circ_buf_t;

/* Opaque handle */
typedef circ_buf_t * cbuf_handle_t;

/* Initialize circular buffer
 * Returns 0 on success, -1 on invalid arguments
 */
int circ_buf_init(cbuf_handle_t c, void *buffer, int max_len, size_t elem_size);


/* Push a single element into the circular buffer
 * Returns 0 on success, -1 if buffer is full
 */
int circ_buf_push(cbuf_handle_t c, const void *data);

/* Push multiple elements into the circular buffer
 * Returns 0 on success, -1 if not enough space
 */
int circ_buf_push_many(cbuf_handle_t c, const void *data, size_t len);

/* Push raw uint8_t data (converted to buffer_type units)
 * Returns 0 on success, -1 if not enough space
 */
//int circ_buf_push_many_uint8(cbuf_handle_t c, uint8_t *data, size_t len);

int circ_buf_push_many_uint8(cbuf_handle_t c, uint8_t *data, size_t len);

/* Pop a single element from the circular buffer
 * Returns 0 on success, -1 if buffer is empty
 */
int circ_buf_pop(cbuf_handle_t c, void *data);

/// Reset the circular buffer to empty, head == tail
void circ_buf_reset(cbuf_handle_t c);

/// Returns true if the buffer is empty
bool circ_buf_empty(cbuf_handle_t c);

/// Returns true if the buffer is full
bool circ_buf_full(cbuf_handle_t c);

/// Returns the maximum capacity of the buffer
size_t circ_buf_capacity(cbuf_handle_t c);

/// Returns the current number of elements in the buffer
size_t circ_buf_size(cbuf_handle_t c);




#endif /* CIRC_BUF_H */
