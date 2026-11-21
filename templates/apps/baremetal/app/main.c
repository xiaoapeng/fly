#include "board.h"

int main(void)
{
    board_init();
    board_log("boot ok\n");

    for (;;) {
        board_idle();
    }
}
