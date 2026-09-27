/**
 * @file init.c
 * @brief FreeRTOS demo board init (no eh dependency)
 */

#include "clock_config.h"

extern int board_debug_init(void);
extern void led_init(void);

void init(void)
{
    BOARD_InitBootClocks();
    board_debug_init();
    led_init();
}