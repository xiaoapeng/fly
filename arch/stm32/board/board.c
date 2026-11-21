#include "board.h"

#include FLY_STM32_DEVICE_HEADER
#include "SEGGER_RTT.h"

void board_init(void)
{
    SystemCoreClockUpdate();
    SEGGER_RTT_Init();
}

void board_log(const char *message)
{
    (void)SEGGER_RTT_WriteString(0u, message);
}

void board_idle(void)
{
    __WFI();
}
