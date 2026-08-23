#ifndef SYSTEM_CHECK_H
#define SYSTEM_CHECK_H

#include <stdint.h>

#define USB_CONNECTION_TRY_NUM  5
#define MAX_USB_SYNC_RESP_TIME 50

uint8_t System_Check(void);
uint8_t getErrorCode();




#endif
