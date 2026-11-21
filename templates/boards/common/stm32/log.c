#include "board.h"

#include "autoconf.h"

#ifdef CONFIG_PACKAGE_SEGGER_RTT
#include "SEGGER_RTT.h"
#endif

void board_log(const char *message)
{
#ifdef CONFIG_PACKAGE_SEGGER_RTT
    (void)SEGGER_RTT_WriteString(0u, message);
#else
    (void)message;
#endif
}
