#include "usb_praser.h";
#include "axis.h"


void read(uint8_t* Buf, uint32_t Len){
	if(read_frame_bytes == frame_size) read_frame_bytes = 0;

	while(Len - read_buf_bytes > 0){
		if(read_frame_bytes>=4){
			uint32_t bytes_to_read = frame_size - read_frame_bytes;
			if(Len - read_buf_bytes < bytes_to_read) bytes_to_read = Len - read_buf_bytes;

			switch(header[0]){
				case 1:
					circ_buf_push_many_uint8(axis_map[0].buffer, Buf + read_buf_bytes, bytes_to_read);
				break;
				case 2:
				break;
				case 3:
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
		}
	}
}
