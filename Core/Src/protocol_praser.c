#include "protocol_praser.h"

static circ_buf_t tx_buffer;
static cbuf_handle_t tx_buf = &tx_buffer;
static uint8_t tx_buf_mem[TX_BUFFER_SIZE];
static uint16_t seq_number_tx = 0;

static void send_frame(FrameType_e type, uint8_t payload_len, uint16_t seq,
                       uint8_t *payload, cbuf_handle_t tx_buf)
{
    uint8_t header[4];

    header[0] = (uint8_t)type;
    header[1] = payload_len;
    header[2] = (uint8_t)(seq & 0xFF);
    header[3] = (uint8_t)(seq >> 8);

    circ_buf_push_many(tx_buf, header, 4);




    if (payload_len > 0 && payload != NULL)
        circ_buf_push_many(tx_buf, payload, payload_len);
}

void usb_tx_process(void)
{
    static uint8_t usb_packet[64];

    if (circ_buf_empty(tx_buf))
        return;

    size_t size = circ_buf_size(tx_buf);

    if (size > 64)
        size = 64;

    for (size_t i = 0; i < size; i++)
        circ_buf_pop(tx_buf, &usb_packet[i]);

    if (CDC_Transmit_FS(usb_packet, size) == USBD_BUSY)
    {
        circ_buf_push_many(tx_buf, usb_packet, size);
    }
}

void prase_init(){
	circ_buf_init(&tx_buffer, tx_buf_mem, TX_BUFFER_SIZE, 1);
	seq_number_tx = 0;
}

uint16_t generate_next_seq(){
	seq_number_tx ++;
	return seq_number_tx;
}

void parse_frame(cbuf_handle_t header_circ_buf, cbuf_handle_t usb_circ_buf) {
    Frame_t frame;


    if (circ_buf_size(header_circ_buf) < 4) return;


    if (circ_buf_pop(header_circ_buf, &frame.type) != 0) return;


    if (circ_buf_pop(header_circ_buf, &frame.payload_len) != 0) return;


    uint8_t seq_bytes[2];
    if (circ_buf_pop(header_circ_buf, &seq_bytes[0]) != 0) return;
    if (circ_buf_pop(header_circ_buf, &seq_bytes[1]) != 0) return;

    frame.seq = (uint16_t)(seq_bytes[0] | (seq_bytes[1] << 8));


    if (frame.type >= FRAME_MOVE_X && frame.type <= FRAME_MOVE_E) {

        for (uint8_t i = 0; i < frame.payload_len; i++) {
            uint8_t discard;
            if (circ_buf_pop(usb_circ_buf, &discard) != 0) break;
        }
        return;
    }


    for (uint8_t i = 0; i < frame.payload_len; i++) {
        if (circ_buf_pop(usb_circ_buf, &frame.payload[i]) != 0) {

            return;
        }
    }


    switch (frame.type) {
        case FRAME_SYNC:
            parse_sync(&frame);
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

            break;
    }
}


void parse_sync(Frame_t* frame) {
    if (frame->payload_len < 4) return;


    uint32_t host_time = frame->payload[0] |
                         (frame->payload[1] << 8) |
                         (frame->payload[2] << 16) |
                         (frame->payload[3] << 24);

   // TO DO
   // aktualizacja czasu mcu

    //currentState = STATE_READY;
    send_frame(FRAME_SYNC, 0, generate_next_seq(), NULL, tx_buf);
}


void send_status()
{
    uint8_t payload[4];

    payload[0] = (uint8_t)currentState;
    payload[1] = 0;
    payload[2] = 0;
    payload[3] = 0;

    send_frame(FRAME_STATUS_RESP, 4, generate_next_seq(), payload, tx_buf);
}

void send_status_req()
{
    send_frame(FRAME_STATUS_RESP, 0, generate_next_seq(), NULL, tx_buf);
}

void parse_status_req(Frame_t* frame)
{
    send_status();
}

void parse_status_resp(Frame_t* frame){
	if (frame->payload_len < 4)
	        return;

	uint8_t state = frame->payload[0];
	uint8_t errors = frame->payload[1];
	uint8_t queue_free = frame->payload[2];

	system_update_remote_state(state);
	system_update_remote_errors(errors);
	system_update_remote_queue(queue_free);
}

void parse_estop(Frame_t* frame)
{

	Emergency_Stop_Activate();


    //send_frame(FRAME_ESTOP, 0, frame->seq, NULL, tx_buf);
}


