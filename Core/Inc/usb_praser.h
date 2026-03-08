#ifndef USB_PRASER_H
#define USB_PRASER_H

#include "main.h"
#include "circular_buffer.h"

static uint8_t frame_type = 0;

static uint32_t read_frame_bytes = 0;
static uint32_t read_buf_bytes = 0;
static const uint32_t frame_size = 1024;

static uint8_t header[4];

void read(uint8_t* Buf, uint32_t Len);


#endif /* CIRC_BUF_H */
