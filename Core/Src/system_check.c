#include "system_check.h"
#include "protocol_praser.h"
#include "debug.h"
#include "stm32f4xx_hal.h"

// first bit from the right is comunication
static uint8_t error_code = 0x00;



uint8_t System_Check(void)
{
    // TO DO 
    // endstop test
    // checking temperature



    // Checking usb connection

    error_code |= 0x02; // set timeout error 0

    for(int i = 0; i<USB_CONNECTION_TRY_NUM; i++){


        send_sync();
        uint8_t usb_flag =0;
        uint32_t start_time;
        start_time = HAL_GetTick();
        
        while(1){
            usb_tx_process();
            parse_frame(&usb_flag);
            if(usb_flag == FRAME_SYNC_ACK || (HAL_GetTick() - start_time) > MAX_USB_SYNC_RESP_TIME) break;
        }

        if(usb_flag == FRAME_SYNC_ACK ){
            debug_info("System_Check: USB sync established\r\n");
            error_code |= (~0x02); // reset timeout error / set timeout error to 1
            break;
        }

        debug_warn("System_Check: USB sync attempt %d timed out\r\n", i + 1);
    }
    
    


    return error_code;
}

uint8_t get_error_code() {return error_code;}

/* Symbol name used by Inc/system_check.h. Kept separate from get_error_code
 * so the existing definition is not modified. */
uint8_t getErrorCode(void) {return error_code;}
