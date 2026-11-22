#ifndef FLY_BOARD_TIME_H
#define FLY_BOARD_TIME_H

#include <stdbool.h>
#include <stdint.h>

/* Start the bare-metal timebase after board_init(). */
bool board_time_start(void);

/* Monotonic time in microseconds with millisecond resolution. */
uint64_t board_time_us(void);

#endif
