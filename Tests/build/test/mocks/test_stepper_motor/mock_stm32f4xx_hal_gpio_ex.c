#include <string.h>
#include <stdlib.h>
#include <setjmp.h>
#include "cmock.h"
#include "mock_stm32f4xx_hal_gpio_ex.h"


static struct mock_stm32f4xx_hal_gpio_exInstance
{
  unsigned char placeHolder;
} Mock;

extern int GlobalExpectCount;
extern int GlobalVerifyOrder;

void mock_stm32f4xx_hal_gpio_ex_Verify(void)
{
}

void mock_stm32f4xx_hal_gpio_ex_Init(void)
{
  mock_stm32f4xx_hal_gpio_ex_Destroy();
}

void mock_stm32f4xx_hal_gpio_ex_Destroy(void)
{
  CMock_Guts_MemFreeAll();
  memset(&Mock, 0, sizeof(Mock));
  GlobalExpectCount = 0;
  GlobalVerifyOrder = 0;
}

