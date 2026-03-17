#include "usb_praser.h"
#include "axis.h"

static uint8_t frame_type = 0;

static uint32_t read_frame_bytes = 0;
static uint32_t read_buf_bytes = 0;
static uint32_t frame_size = 1024;

static uint8_t header[4];

static circ_buf_t header_circ_buf;
static circ_buf_t usb_circ_buf;

void usb_praser_init(circ_buf_t header, circ_buf_t usb){
	usb_circ_buf = usb;
	header_circ_buf = header;

}

void read_usb_praser(uint8_t* Buf, uint32_t Len){
	if(read_frame_bytes == frame_size) read_frame_bytes = 0;

	while(Len - read_buf_bytes > 0){
		if(read_frame_bytes>=4){
			frame_size  = header[1];
			uint32_t bytes_to_read = ((frame_size - read_frame_bytes)>>2)<<2;
			if(Len - read_buf_bytes < bytes_to_read) bytes_to_read = Len - read_buf_bytes;

			switch(header[0]){
				case 1:
					circ_buf_push_many_uint8(axis_map[0].buffer, Buf + read_buf_bytes, bytes_to_read);
				break;
					circ_buf_push_many_uint8(axis_map[1].buffer, Buf + read_buf_bytes, bytes_to_read);
				case 2:
					circ_buf_push_many_uint8(axis_map[2].buffer, Buf + read_buf_bytes, bytes_to_read);
				break;
				case 3:
					circ_buf_push_many_uint8(axis_map[3].buffer, Buf + read_buf_bytes, bytes_to_read);
				break;
				default:
					circ_buf_push_many(&usb_circ_buf, Buf + read_buf_bytes, bytes_to_read);
				break;

			}
			read_buf_bytes += bytes_to_read;
			read_frame_bytes += bytes_to_read;
		}
		else{
			uint32_t bytes_to_read = Len - read_buf_bytes;
			if(Len - read_buf_bytes >= 4 - read_frame_bytes) bytes_to_read =  4 - read_frame_bytes;

			memcpy(header + read_frame_bytes, Buf + read_buf_bytes, bytes_to_read);



			read_buf_bytes += bytes_to_read;
			read_frame_bytes += bytes_to_read;

			if(read_frame_bytes >= 0) circ_buf_push(&header_circ_buf, header);
		}
	}
}


