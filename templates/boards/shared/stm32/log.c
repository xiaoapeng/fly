#include "board.h"

#include "mid/fly_log.h"

#include <string.h>


void board_log(const char *message)
{
    (void)fly_log_write(message, strlen(message));
}
