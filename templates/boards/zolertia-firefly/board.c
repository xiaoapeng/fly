#include "board.h"

#include "mid/fly_log.h"

#include <string.h>

void board_init(void)
{
    /* The startup template keeps the reset-default 16 MHz oscillator. */
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __asm__ volatile ("wfi");
}
