#include "board.h"

#include "mid/fly_log.h"

#include <string.h>

void board_init(void)
{
    /* The boot template keeps the 50 MHz reset clock. */
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __asm__ volatile ("wfi");
}
