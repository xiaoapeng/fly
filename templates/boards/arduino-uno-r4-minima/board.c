#include "board.h"

#include "mid/fly_log.h"

#include <string.h>

void board_init(void)
{
    /* Keep the clock inherited from the Arduino bootloader. */
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __asm__ volatile ("wfi");
}
