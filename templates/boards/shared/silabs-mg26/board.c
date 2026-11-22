#include "board.h"
#include "em_device.h"
#include "mid/fly_log.h"

#include <string.h>

void board_init(void)
{
    SystemCoreClockUpdate();
}

void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}

void board_idle(void)
{
    __WFI();
}
