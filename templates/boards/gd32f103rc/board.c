#include "board.h"

#include "autoconf.h"
#include "gd32f10x.h"

#ifdef CONFIG_PACKAGE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

void board_init(void)
{
    /* The vendor startup code calls SystemInit before main. */
#ifdef CONFIG_PACKAGE_SEGGER_RTT
    SEGGER_RTT_Init();
#endif
    __enable_irq();
}

void board_log(const char *message)
{
#ifdef CONFIG_PACKAGE_SEGGER_RTT
    (void)SEGGER_RTT_WriteString(0u, message);
#else
    (void)message;
#endif
}

void board_idle(void)
{
    __WFI();
}
