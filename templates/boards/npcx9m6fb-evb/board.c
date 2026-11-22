#include "board.h"

#include "mid/fly_log.h"

#include <string.h>

void board_init(void)
{
    __asm__ volatile ("cpsie i" ::: "memory");
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __asm__ volatile ("wfi");
}
