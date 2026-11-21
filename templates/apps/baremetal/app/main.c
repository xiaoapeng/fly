#include "board.h"
#include "mid/firmware_info.h"

int main(void)
{
    board_init();
    fly_log_firmware_info();
    board_log("boot ok\n");

    for (;;) {
        board_idle();
    }
}
