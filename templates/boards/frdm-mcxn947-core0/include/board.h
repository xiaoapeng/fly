#ifndef FLY_BOARD_H
#define FLY_BOARD_H

/* Initialize board clocks and the debug console before starting the app. */
void board_init(void);

/* Write a NUL-terminated message to the board's debug console. */
void board_log(const char *message);

/* Wait for an interrupt in the bare-metal idle loop. */
void board_idle(void);

#endif
