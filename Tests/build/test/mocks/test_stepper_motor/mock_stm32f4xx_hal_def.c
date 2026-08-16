#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "stm32f4xx_hal.h"
#include "mock_stm32f4xx_hal_def.h"
#include "cmock.h"


static struct mock_stm32f4xx_hal_defInstance
{
  unsigned char placeHolder;
} Mock;

extern int GlobalExpectCount;
extern int GlobalVerifyOrder;

void mock_stm32f4xx_hal_def_Verify(void)
{
}

void mock_stm32f4xx_hal_def_Init(void)
{
  mock_stm32f4xx_hal_def_Destroy();
}

void mock_stm32f4xx_hal_def_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
  GlobalExpectCount = 0;
  GlobalVerifyOrder = 0;
}

