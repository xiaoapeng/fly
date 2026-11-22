#include "board.h"

#include "mid/fly_log.h"
#include "nrf.h"

#include <string.h>

void board_init(void)
{
    /* The Nordic reset handler calls SystemInit before main. */
    __enable_irq();
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __WFI();
}
