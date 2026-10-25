#include "protocol_praser.h"
#include "debug.h"
#include <string.h>

static circ_buf_t tx_buffer;

static cbuf_handle_t tx_buf = &tx_buffer;

static uint8_t tx_buf_mem[TX_BUFFER_SIZE];

static uint16_t seq_number_tx = 0;

static cbuf_handle_t header_circ_buf;

static cbuf_handle_t usb_circ_buf;

// bool usb_send(const uint8_t *data, uint16_t len)

static uint8_t send_frame(Frame_t *frame)
{
    unsigned int frame_length;
    unsigned int pushed;

    if (frame == NULL) {
        debug_error("send_frame: NULL frame\r\n");
        return 0;
    }

    if ((unsigned int)frame->payload_len > MAX_PAYLOAD_SIZE) {
        debug_error("send_frame: payload_len %u exceeds max\r\n", (unsigned)frame->payload_len);
        return 0;
    }

    frame_length = (unsigned int)frame->payload_len + 4U;

    if (CDC_Transmit_FS((uint8_t *)frame, (uint16_t)frame_length) == USBD_OK)
        return 1;

    // USB BUSY, we put data to fifo
    if (circ_buf_capacity(tx_buf) < frame_length)
        return 0;

    pushed = circ_buf_push_many(tx_buf, (uint8_t *)frame, frame_length);
    if (pushed != frame_length) {
        debug_error("send_frame: TX buffer truncated\r\n");
    }

    return 1;
}

void usb_tx_process(void)
{
    static uint8_t usb_packet[64];
    USBD_StatusTypeDef usb_status;
    size_t size;

    if (tx_buf == NULL) {
        debug_error("usb_tx_process: TX buffer not initialized\r\n");
        return;
    }

    if (circ_buf_empty(tx_buf))
        return;

    size = circ_buf_size(tx_buf);

    if (size == 0U)
        return;

    if (size > 64U)
        size = 64U;

    if (circ_buf_out_peek(tx_buf, usb_packet, (unsigned int)size) != (unsigned int)size) {
        debug_error("usb_tx_process: peek failed\r\n");
        return;
    }

    usb_status = CDC_Transmit_FS(usb_packet, (uint16_t)size);
    if (usb_status == USBD_OK) {
        circ_buf_skip(tx_buf, (int)size);
    } else if (usb_status != USBD_BUSY) {
        debug_error("usb_tx_process: transmit failed %d\r\n", (int)usb_status);
    }
}

void parse_init(cbuf_handle_t header_buf, cbuf_handle_t usb_buf)
{
    if ((header_buf == NULL) || (usb_buf == NULL)) {
        debug_error("parse_init: NULL buffer\r\n");
        return;
    }

    if ((header_buf->data == NULL) || (usb_buf->data == NULL)) {
        debug_error("parse_init: uninitialized buffer\r\n");
        return;
    }

    circ_buf_init(&tx_buffer, tx_buf_mem, TX_BUFFER_SIZE, 1U);

    header_circ_buf = header_buf;
    usb_circ_buf = usb_buf;

    seq_number_tx = 0U;
}

uint16_t generate_next_seq(void)
{
    seq_number_tx++;

    return seq_number_tx;
}

void parse_frame(uint8_t *flag)
{
    Frame_t frame;
    uint8_t stored_length;

    if (flag == NULL) {
        debug_error("parse_frame: NULL flag\r\n");
        return;
    }

    *flag = 0;

    if ((header_circ_buf == NULL) || (usb_circ_buf == NULL)) {
        debug_error("parse_frame: buffers not initialized\r\n");
        return;
    }

    if (circ_buf_size(header_circ_buf) < 4U)
        return;

    if (circ_buf_pop(header_circ_buf, &frame.type) != 0)
        return;

    if (circ_buf_pop(header_circ_buf, &frame.payload_len) != 0)
        return;

    uint8_t seq_bytes[2];

    if (circ_buf_pop(header_circ_buf, &seq_bytes[0]) != 0)
        return;

    if (circ_buf_pop(header_circ_buf, &seq_bytes[1]) != 0)
        return;

    frame.seq = (uint16_t)(seq_bytes[0] | (seq_bytes[1] << 8));

    if ((unsigned int)frame.payload_len > MAX_PAYLOAD_SIZE) {
        debug_error("parse_frame: payload_len %u exceeds max\r\n", (unsigned)frame.payload_len);
        stored_length = MAX_PAYLOAD_SIZE;
    } else {
        stored_length = frame.payload_len;
    }

    if (frame.type >= FRAME_MOVE_X && frame.type <= FRAME_MOVE_E)
    {
        for (uint8_t i = 0; i < frame.payload_len; i++)
        {
            uint8_t discard;

            if (circ_buf_pop(usb_circ_buf, &discard) != 0)
                return;
        }
    }

    for (uint8_t i = 0; i < frame.payload_len; i++)
    {
        uint8_t payload_byte;

        if (circ_buf_pop(usb_circ_buf, &payload_byte) != 0)
            return;

        if (i < stored_length)
        {
            frame.payload[i] = payload_byte;
        }
    }

    switch (frame.type)
    {
        case FRAME_SYNC:
        case FRAME_ACK:
        case FRAME_MOVE_X:
        case FRAME_MOVE_Y:
        case FRAME_MOVE_Z:
        case FRAME_MOVE_E:
            break;

        case FRAME_SYNC_ACK:
            parse_sync_ack(&frame, flag);
            break;

        case FRAME_STATUS_REQ:
            parse_status_req(&frame);
            break;

        case FRAME_STATUS_RESP:
            parse_status_resp(&frame);

            break;

        case FRAME_ESTOP:
            parse_estop(&frame);
            break;

        default:
            debug_warn("parse_frame: unknown frame type 0x%02x\r\n", (unsigned)frame.type);
            break;
    }
}

void send_sync(void)
{
    char payload[] = "Hi!\n";
    Frame_t frame;

    frame.payload_len = 4;
    memcpy(frame.payload, payload, frame.payload_len);
    frame.type = FRAME_SYNC;
    frame.seq = 0U;

    if (send_frame(&frame) == 0U) {
        debug_error("send_sync: transmit failed\r\n");
    }
}


//TO DO
void parse_status_resp(Frame_t *frame){
	(void)frame;
	return;
}

//TO DO
void parse_estop(Frame_t *frame){
	(void)frame;
	return;
}

void parse_sync_ack(Frame_t *frame, uint8_t *flag)
{
    char payload[] = "Jo!\n";
    Frame_t frame_resp;

    if ((frame == NULL) || (flag == NULL)) {
        debug_error("parse_sync_ack: NULL argument\r\n");
        return;
    }

    if (frame->payload_len < 4)
        return;

    frame_resp.payload_len = 4;
    memcpy(frame_resp.payload, payload, frame_resp.payload_len);
    frame_resp.type = FRAME_ACK;
    frame_resp.seq = generate_next_seq();

    *flag = frame_resp.type;

    if (send_frame(&frame_resp) == 0U) {
        debug_error("parse_sync_ack: transmit failed\r\n");
    }
}

void send_status(void)
{
    uint8_t payload[4];

    payload[0] = (uint8_t)currentState;
    payload[1] = 0;
    payload[2] = 0;
    payload[3] = 0;

    Frame_t frame_resp;

    frame_resp.payload_len = 4;
    memcpy(frame_resp.payload, payload, frame_resp.payload_len);
    frame_resp.type = FRAME_STATUS_RESP;
    frame_resp.seq = generate_next_seq();

    if (send_frame(&frame_resp) == 0U) {
        debug_error("send_status: transmit failed\r\n");
    }
}

void send_status_req(void)
{
    Frame_t frame_resp;

    frame_resp.payload_len = 0;
    frame_resp.type = FRAME_STATUS_RESP;
    frame_resp.seq = generate_next_seq();

    if (send_frame(&frame_resp) == 0U) {
        debug_error("send_status_req: transmit failed\r\n");
    }
}

void parse_status_req(Frame_t *frame)
{
    (void)frame;

}
