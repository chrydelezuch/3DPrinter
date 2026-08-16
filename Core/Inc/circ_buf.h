#ifndef CIRC_BUF_H
#define CIRC_BUF_H

/**
 * @brief Circular buffer descriptor.
 *
 * The buffer uses a power-of-two capacity and a bit mask for
 * efficient circular addressing.
 */
typedef struct {
    unsigned char *data;          /* Pointer to the buffer storage. */
    volatile unsigned int in;     /* Write position in bytes. */
    volatile unsigned int out;    /* Read position in bytes. */
    volatile unsigned int esize;  /* Item size in bytes. */
    unsigned int mask;            /* Buffer capacity - 1. */
} circ_buf_t;

/* Circular buffer handle. */
typedef circ_buf_t *cbuf_handle_t;


/**
 * @brief Initialize a circular buffer.
 *
 * The effective buffer capacity is rounded down to the nearest
 * power of two.
 *
 * @param self       Circular buffer handle.
 * @param buf        Buffer storage.
 * @param bufsize    Buffer size in bytes.
 * @param item_size  Size of one item in bytes.
 */
void circ_buf_init(circ_buf_t *self,
                   void *buf,
                   unsigned int bufsize,
                   unsigned int item_size);


/**
 * @brief Return the number of bytes currently stored in the buffer.
 *
 * @param self Circular buffer handle.
 *
 * @return Number of bytes currently stored.
 */
unsigned int circ_buf_size_uint8(circ_buf_t *self);


/**
 * @brief Return the buffer capacity in bytes.
 *
 * @param self Circular buffer handle.
 *
 * @return Maximum number of bytes the buffer can hold.
 */
unsigned int circ_buf_capacity_uint8(circ_buf_t *self);


/**
 * @brief Return the available space in bytes.
 *
 * @param self Circular buffer handle.
 *
 * @return Number of free bytes available.
 */
unsigned int circ_buf_avail_uint8(circ_buf_t *self);


/**
 * @brief Return the number of items currently stored.
 *
 * @param self Circular buffer handle.
 *
 * @return Number of stored items.
 */
unsigned int circ_buf_size(circ_buf_t *self);


/**
 * @brief Return the buffer capacity in items.
 *
 * @param self Circular buffer handle.
 *
 * @return Maximum number of items the buffer can hold.
 */
unsigned int circ_buf_capacity(circ_buf_t *self);


/**
 * @brief Return the available space in items.
 *
 * @param self Circular buffer handle.
 *
 * @return Number of free items available.
 */
unsigned int circ_buf_avail(circ_buf_t *self);


/**
 * @brief Check whether the buffer is full.
 *
 * @param self Circular buffer handle.
 *
 * @return Non-zero if the buffer is full, otherwise zero.
 */
int circ_buf_full(circ_buf_t *self);


/**
 * @brief Check whether the buffer is empty.
 *
 * @param self Circular buffer handle.
 *
 * @return Non-zero if the buffer is empty, otherwise zero.
 */
int circ_buf_empty(circ_buf_t *self);


/**
 * @brief Push one item into the buffer.
 *
 * @param self Circular buffer handle.
 * @param buf  Pointer to the item to be inserted.
 *
 * @return Number of items inserted.
 */
unsigned int circ_buf_push(circ_buf_t *self, const void *buf);


/**
 * @brief Push multiple items into the buffer.
 *
 * If there is not enough free space, only the items that fit
 * into the buffer are inserted.
 *
 * @param self       Circular buffer handle.
 * @param buf        Pointer to the source data.
 * @param item_count Number of items to insert.
 *
 * @return Number of items actually inserted.
 */
unsigned int circ_buf_push_many(circ_buf_t *self,
                                const void *buf,
                                unsigned int item_count);


/**
 * @brief Push raw bytes into the buffer.
 *
 * If there is not enough free space, only the bytes that fit
 * into the buffer are inserted.
 *
 * @param self       Circular buffer handle.
 * @param buf        Pointer to the source data.
 * @param item_count Number of bytes to insert.
 *
 * @return Number of bytes actually inserted.
 */
unsigned int circ_buf_push_many_uint8(circ_buf_t *self,
                                      const void *buf,
                                      unsigned int item_count);


/**
 * @brief Read data from the buffer without removing it.
 *
 * @param self Circular buffer handle.
 * @param buf  Destination buffer.
 * @param len  Maximum number of bytes to read.
 *
 * @return Number of bytes copied.
 */
unsigned int circ_buf_out_peek(circ_buf_t *self,
                               void *buf,
                               unsigned int len);


/**
 * @brief Pop one item from the buffer.
 *
 * @param self Circular buffer handle.
 * @param buf  Destination buffer.
 *
 * @return Number of items actually removed.
 */
unsigned int circ_buf_pop(circ_buf_t *self, void *buf);


/**
 * @brief Pop multiple items from the buffer.
 *
 * If fewer items are available than requested, only the available
 * items are removed.
 *
 * @param self       Circular buffer handle.
 * @param buf        Destination buffer.
 * @param item_count Number of items to remove.
 *
 * @return Number of items actually removed.
 */
unsigned int circ_buf_pop_many(circ_buf_t *self,
                               void *buf,
                               unsigned int item_count);

#endif /* CIRC_BUF_H */
