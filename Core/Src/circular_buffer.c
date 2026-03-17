#include "circular_buffer.h"

static inline int circ_buf_free_space(cbuf_handle_t c)
{
    return c->max_len - ((c->head - c->tail + c->max_len) % c->max_len) - 1;
}

int circ_buf_init(cbuf_handle_t c, void *buffer, int max_len, size_t elem_size)
{
    if (!c || !buffer || max_len <= 0 || elem_size == 0)
        return -1;

    c->buffer = (uint8_t *)buffer;
    c->max_len = max_len;
    c->elem_size = elem_size;
    c->head = 0;
    c->tail = 0;
    c->leftover_size=0;

    return 0;
}

int circ_buf_push(cbuf_handle_t c, const void *data)
{
    int next = c->head + 1;

    if (next >= c->max_len)
        next = 0;

    if (next == c->tail)
        return -1; // pełny

    memcpy(&c->buffer[c->head * c->elem_size], data, c->elem_size);
    c->head = next;

    return 0;
}

int circ_buf_push_many(cbuf_handle_t c, const void *data, size_t len)
{
    if (circ_buf_free_space(c) < (int)len)
        return -1;

    int next = c->head + (int)len;

    if (next >= c->max_len) {
        size_t first_part = c->max_len - c->head;

        memcpy(&c->buffer[c->head * c->elem_size],
               data,
               first_part * c->elem_size);

        memcpy(&c->buffer[0],
               (uint8_t *)data + first_part * c->elem_size,
               (len - first_part) * c->elem_size);

        next -= c->max_len;
    } else {
        memcpy(&c->buffer[c->head * c->elem_size],
               data,
               len * c->elem_size);
    }

    c->head = next;

    return 0;
}

int circ_buf_push_many_uint8(cbuf_handle_t c, uint8_t *data, size_t len)
{

	int leftover = len;
	len /= c->elem_size;
	leftover -= len * c->elem_size;

	c->leftover_size += leftover;

	if(c->leftover_size >= c->elem_size){
		memcpy(c->leftover + c->leftover_size - leftover, data , leftover);
		circ_buf_push(c, c->leftover);
	    memcpy(c->leftover, c->leftover + c->elem_size, c->elem_size);
	    c->leftover_size -= c->elem_size;
	    data += c->leftover_size;
	    leftover = 0;
	}



    if (circ_buf_free_space(c) < (int)len)
        return -1;

    int next = c->head + (int)len;

    if (next >= c->max_len) {
        size_t first_part = c->max_len - c->head;

        memcpy(&c->buffer[c->head * c->elem_size],
               data,
               first_part * c->elem_size);

        memcpy(&c->buffer[0],
               (uint8_t *)data + first_part * c->elem_size,
               (len - first_part) * c->elem_size);

        next -= c->max_len;
    } else {
        memcpy(&c->buffer[c->head * c->elem_size],
               data,
               len * c->elem_size);
    }

    c->head = next;

    memcpy(c->leftover + c->leftover_size, data + len * c->elem_size, leftover);



    return 0;
}

int circ_buf_pop(cbuf_handle_t c, void *data)
{
    if (c->head == c->tail)
        return -1; // pusty

    int next = c->tail + 1;
    if (next >= c->max_len)
        next = 0;

    memcpy(data, &c->buffer[c->tail * c->elem_size], c->elem_size);
    c->tail = next;

    return 0;
}

void circ_buf_reset(cbuf_handle_t c)
{
    if (!c) return;

    c->head = 0;
    c->tail = 0;
}

bool circ_buf_empty(cbuf_handle_t c)
{
    if (!c) return true;
    return circ_buf_free_space(c) == c->max_len - 1;
}

bool circ_buf_full(cbuf_handle_t c)
{
    if (!c) return false;
    return circ_buf_free_space(c) == 0;
}

size_t circ_buf_capacity(cbuf_handle_t c)
{
    if (!c) return 0;
    return (size_t)c->max_len - 1;
}

size_t circ_buf_size(cbuf_handle_t c)
{
    if (!c) return 0;
    return (size_t)(c->max_len - circ_buf_free_space(c) - 1);
}

/*
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
*/
