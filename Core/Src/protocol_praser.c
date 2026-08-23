#include "protocol_praser.h"

static circ_buf_t tx_buffer;
static cbuf_handle_t tx_buf = &tx_buffer;
static uint8_t tx_buf_mem[TX_BUFFER_SIZE];
static uint16_t seq_number_tx = 0;

static cbuf_handle_t header_circ_buf;
static cbuf_handle_t usb_circ_buf;

//bool usb_send(const uint8_t *data, uint16_t len)
static uint8_t send_frame(Frame_t* frame)
{
    if (CDC_Transmit_FS((uint8_t *)frame, frame->payload_len + 4) == USBD_OK)
        return 1;

    // USB BUSY, we put  data to fifo
    if (circ_buf_free(tx_buf) < frame->payload_len + 4)
        return 0;

    circ_buf_push_many(tx_buf, (uint8_t *)frame, frame->payload_len + 4);

    return 1;
}

void usb_tx_process(void)
{
    static uint8_t usb_packet[64];

    if (circ_buf_empty(tx_buf))
        return;

    size_t size = circ_buf_size(tx_buf);

    if(size <= 0 ) return;

    if (size > 64)
        size = 64;

    circ_buf_out_peek(tx_buf, usb_packet, size);

    if (CDC_Transmit_FS(usb_packet, size) == USBD_OK)
        circ_buf_skip(tx_buf, size);
}


void parse_init(cbuf_handle_t header_buf, cbuf_handle_t usb_buf)
{
    circ_buf_init(&tx_buffer, tx_buf_mem, TX_BUFFER_SIZE, 1U);
    header_circ_buf = header_buf;
    usb_circ_buf = usb_buf;
    seq_number_tx = 0U;
}

uint16_t generate_next_seq(){
	seq_number_tx ++;
	return seq_number_tx;
}

void parse_frame(uint8_t * flag) {
    Frame_t frame;

    &flag = 0;



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
            if (circ_buf_pop(usb_circ_buf, &discard) != 0) return;
        }
        
    }


    for (uint8_t i = 0; i < frame.payload_len; i++) {
        if (circ_buf_pop(usb_circ_buf, &frame.payload[i]) != 0) {

            return;
        }
    }


    switch (frame.type) {
        case FRAME_SYNC:
            parse_sync(&frame, flag);
            break;
        case FRAME_STATUS_REQ:
            parse_status_req(&frame, flag);
            break;
        case FRAME_STATUS_RESP:
            parse_status_resp(&frame, flag);
            break;
        case FRAME_ESTOP:
            parse_estop(&frame);
            break;
        default:

            break;
    }
}


void send_sync(void) {

    char payload[] = "Hi!\n";

    Frame_t * frame;
    frame->payload_len = 4;
    frame->payload = payload;
    frame->type = FRAME_SYNC;
    

    send_frame(frame);
}


void prase_sync(Frame_t* frame, uint8_t * flag) {
    if (frame->payload_len < 4) return;

    char payload[] = "Jo!\n";

    Frame_t * frame_resp;
    frame_resp->payload_len = 4;
    frame_resp->payload = payload;
    frame_resp->type = FRAME_ACK;
    frame_resp->seq = generate_next_seq();
    flag = frame->type;
    send_frame(frame);
}

void send_status()
{
    uint8_t payload[4];

    payload[0] = (uint8_t)currentState;
    payload[1] = 0;
    payload[2] = 0;
    payload[3] = 0;

    Frame_t * frame_resp;
    frame_resp->payload_len = 4;
    frame_resp->payload = payload;
    frame_resp->type = FRAME_STATUS_RESP;
    frame_resp->seq = generate_next_seq();
    flag = frame->type;
    send_frame(frame);
}

void send_status_req()
{

    Frame_t * frame_resp;
    frame_resp->payload_len = 0;
    frame_resp->type = FRAME_STATUS_RESP;
    frame_resp->seq = generate_next_seq();
    flag = frame->type;
    send_frame(frame);
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

