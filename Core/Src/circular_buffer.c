#include <string.h>
#include <stdint.h>
#include "circular_buffer.h"

static inline int circ_buf_free_space(cbuf_handle_t cb)
{
    return 	cb->max_len - ((cb->head - cb->tail + cb->max_len) % cb->max_len) - 1;
}

int circ_buf_init(cbuf_handle_t c,
                  buffer_type *buffer,
                  int max_len)
{
    if (!c || !buffer || max_len <= 0) return -1;

    c->buffer = buffer;
    c->max_len = max_len;
    c->head = 0;
    c->tail = 0;


    return 0;
}


int circ_buf_push(cbuf_handle_t c, buffer_type data)
{
    int next;

    next = c->head + 1;

    if (next >= c->max_len)
    	next = 0;

    if (next == c->tail)
    	return -1;  // pełny

    c->buffer[c->head] = data;
    c->head = next;

    return 0;
}

int circ_buf_push_many(cbuf_handle_t c, buffer_type *data, size_t len)
{
    int next;

    if (circ_buf_free_space(c) < (int)len) // zdarza się rzadko
        return -1;

    next = c->head + len;

    if (next >= c->max_len) { //zdarza się żadko
        size_t first_part = c->max_len - c->head;

        memcpy(&c->buffer[c->head],
               data,
               first_part * sizeof(buffer_type));

        memcpy(&c->buffer[0],
               data + first_part,
               (len - first_part) * sizeof(buffer_type));

        next -= c->max_len;
    } else {
        memcpy(&c->buffer[c->head],
               data,
               len * sizeof(buffer_type));
    }

    c->head = next;

    return 0;
}

int circ_buf_push_many_uint8(cbuf_handle_t c, uint8_t *data, size_t len)
{
    int next;

    len /= sizeof(buffer_type);

    if (circ_buf_free_space(c) < (int)len) // zdarza się rzadko
        return -1;

    next = c->head + len;

    if (next >= c->max_len) { // zdarza się rzadko
        size_t first_part = c->max_len - c->head;

        memcpy(&c->buffer[c->head],
               data,
               first_part * sizeof(buffer_type));

        memcpy(&c->buffer[0],
               data + first_part,
               (len - first_part) * sizeof(buffer_type));

        next -= c->max_len;
    } else {
        memcpy(&c->buffer[c->head],
               data,
               len * sizeof(buffer_type));
    }

    c->head = next;

    return 0;
}

int circ_buf_pop(cbuf_handle_t c, buffer_type *data)
{
    int next;

    if (c->head == c->tail) // zdarza się rzadko
        return -1;

    next = c->tail + 1;

    if (next >= c->max_len) // zdarza się rzadko
        next = 0;

    *data = c->buffer[c->tail];
    c->tail = next;

    return 0;
}

void circ_buf_reset(cbuf_handle_t c)
{
    if (!c)
        return;

    c->head = 0;
    c->tail = 0;
}

bool circ_buf_empty(cbuf_handle_t c)
{
    if (!c)
        return true;

    return circ_buf_free_space(c) == c->max_len - 1;
}

bool circ_buf_full(cbuf_handle_t c)
{
    if (!c)
        return false;

    return circ_buf_free_space(c) == 0;
}

size_t circ_buf_capacity(cbuf_handle_t c)
{
    if (!c)
        return 0;

    return (size_t)c->max_len-1;
}

size_t circ_buf_size(cbuf_handle_t c)
{
    if (!c)
        return 0;

    return (size_t)(c->max_len - circ_buf_free_space(c) - 1);
}

