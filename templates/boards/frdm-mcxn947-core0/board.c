#include "board.h"

#include "mid/fly_log.h"

#include <string.h>

#include "autoconf.h"
#include "clock_config.h"

#if defined(CONFIG_PACKAGE_SCHEDULER_FREERTOS)
_Static_assert(CONFIG_PACKAGE_FREERTOS_SYSTICK_CLOCK_HZ == BOARD_BOOTCLOCKPLL150M_CORE_CLOCK,
               "FreeRTOS SysTick clock must match the board core clock");
#endif

void board_init(void)
{
    BOARD_InitBootClocks();
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __WFI();
}
