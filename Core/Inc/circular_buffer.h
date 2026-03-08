#ifndef CIRC_BUF_H
#define CIRC_BUF_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "t_velocity.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t buffer_type;

typedef struct {
    buffer_type * buffer;  /* pointer to buffer memory */
    volatile int head;
    volatile int tail;
    int max_len;
} circ_buf_t;

/* Opaque handle */
typedef circ_buf_t * cbuf_handle_t;

/* Initialize circular buffer
 * Returns 0 on success, -1 on invalid arguments
 */
int circ_buf_init(cbuf_handle_t c, buffer_type *buffer, int max_len);


/* Push a single element into the circular buffer
 * Returns 0 on success, -1 if buffer is full
 */
int circ_buf_push(cbuf_handle_t c, buffer_type data);

/* Push multiple elements into the circular buffer
 * Returns 0 on success, -1 if not enough space
 */
int circ_buf_push_many(cbuf_handle_t c, buffer_type *data, size_t len);

/* Push raw uint8_t data (converted to buffer_type units)
 * Returns 0 on success, -1 if not enough space
 */
int circ_buf_push_many_uint8(cbuf_handle_t c, uint8_t *data, size_t len);

/* Pop a single element from the circular buffer
 * Returns 0 on success, -1 if buffer is empty
 */
int circ_buf_pop(cbuf_handle_t c, buffer_type *data);

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


#ifdef __cplusplus
}
#endif

#endif /* CIRC_BUF_H */
