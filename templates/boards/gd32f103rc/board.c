#include "board.h"

#include "mid/fly_log.h"

#include <string.h>

#include "gd32f10x.h"

void board_init(void)
{
    /* The vendor startup code calls SystemInit before main. */
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
