#include "board.h"

#include FLY_STM32_DEVICE_HEADER

void board_init(void)
{
    SystemCoreClockUpdate();
}

void board_idle(void)
{
    __WFI();
}
