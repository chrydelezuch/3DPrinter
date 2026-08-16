#include "usb_praser.h"
#include "axis.h"

#define USB_PRASER_MAX_ITERATIONS 2048U

static uint32_t read_frame_bytes = 0U;
static uint32_t read_buf_bytes = 0U;
static uint32_t frame_size = 1024U;

static uint8_t header[4U];

static circ_buf_t header_circ_buf;
static circ_buf_t usb_circ_buf;

void usb_praser_init(circ_buf_t header, circ_buf_t usb){
	usb_circ_buf = usb;
	header_circ_buf = header;

}

void read_usb_praser(uint8_t* Buf, uint32_t Len)
{
    uint32_t iteration_count = 0U;

    if (read_frame_bytes >= frame_size) {
        read_frame_bytes = 0U;
        read_buf_bytes = 0U;
    }

    while ((Len > read_buf_bytes) && (iteration_count < USB_PRASER_MAX_ITERATIONS)) {
        iteration_count++;

        if (read_frame_bytes >= 4U) {
            uint32_t bytes_to_read;

            frame_size = (uint32_t)header[1];
            bytes_to_read = ((frame_size - read_frame_bytes) >> 2U) << 2U;
            if ((Len - read_buf_bytes) < bytes_to_read) {
                bytes_to_read = Len - read_buf_bytes;
            }

            switch (header[0]) {
                case 1U:
                    circ_buf_push_many_uint8(axis_map[0].buffer, Buf + read_buf_bytes, bytes_to_read);
                    break;
                case 2U:
                    circ_buf_push_many_uint8(axis_map[1].buffer, Buf + read_buf_bytes, bytes_to_read);
                    break;
                case 3U:
                    circ_buf_push_many_uint8(axis_map[2].buffer, Buf + read_buf_bytes, bytes_to_read);
                    break;
                case 4U:
                    circ_buf_push_many_uint8(axis_map[3].buffer, Buf + read_buf_bytes, bytes_to_read);
                    break;
                default:
                    circ_buf_push_many(&usb_circ_buf, Buf + read_buf_bytes, bytes_to_read);
                    break;
            }
            read_buf_bytes += bytes_to_read;
            read_frame_bytes += bytes_to_read;
        } else {
            uint32_t bytes_to_read = Len - read_buf_bytes;
            uint32_t header_remaining = 4U - read_frame_bytes;

            if (bytes_to_read > header_remaining) {
                bytes_to_read = header_remaining;
            }

            memcpy(header + read_frame_bytes, Buf + read_buf_bytes, bytes_to_read);

            read_buf_bytes += bytes_to_read;
            read_frame_bytes += bytes_to_read;

            if (read_frame_bytes >= 4U) {
                circ_buf_push(&header_circ_buf, header);
            }
        }
    }
}

