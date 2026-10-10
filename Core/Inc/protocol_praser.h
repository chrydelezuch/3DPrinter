#ifndef PROTOCOL_PRASER_H
#define PROTOCOL_PRASER_H

#include <stdint.h>
#include <stddef.h>

#include "circ_buf.h"
#include "usbd_cdc_if.h"

#ifdef __cplusplus
extern "C" {
#endif


// ############################################################
// Frame types
// ############################################################

typedef enum
{
    FRAME_SYNC        = 0x01,
    FRAME_SYNC_ACK    = 0x02,
    FRAME_ACK         = 0x03,
    FRAME_STATUS_REQ  = 0x04,
    FRAME_STATUS_RESP = 0x05,
    FRAME_ESTOP       = 0x06,
    FRAME_MOVE_X      = 0x07,
    FRAME_MOVE_Y      = 0x08,
    FRAME_MOVE_Z      = 0x09,
    FRAME_MOVE_E      = 0x10

} FrameType_e;


// ############################################################
// Constants
// ############################################################

#define MAX_PAYLOAD_SIZE 12U
#define TX_BUFFER_SIZE   1024U


// ############################################################
// Frame structure
// ############################################################

typedef struct
{
    FrameType_e type;
    uint8_t     payload_len;
    uint16_t    seq;
    uint8_t     payload[MAX_PAYLOAD_SIZE];

} Frame_t;


// ############################################################
// Parser
// ############################################################

void parse_init(cbuf_handle_t header_buf,
                cbuf_handle_t usb_buf);

void parse_frame(uint8_t *flag);


// ############################################################
// Sequence number
// ############################################################

uint16_t generate_next_seq(void);


// ############################################################
// USB TX
// ############################################################

void usb_tx_process(void);


// ############################################################
// Sending frames
// ############################################################

void send_sync(void);

void send_status(void);

void send_status_req(void);


// ############################################################
// Parsing frames
// ############################################################

void parse_sync(Frame_t *frame,
                uint8_t *flag);

void parse_status_req(Frame_t *frame);

void parse_status_resp(Frame_t *frame);

void parse_estop(Frame_t *frame);


#ifdef __cplusplus
}
#endif

#endif /* PROTOCOL_PRASER_H */
