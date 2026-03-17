#ifndef USB_PRASER_H
#define USB_PRASER_H

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "circular_buffer.h"


void read_usb_praser(uint8_t* Buf, uint32_t Len);

void usb_praser_init(circ_buf_t header, circ_buf_t usb);


#endif /* CIRC_BUF_H */
