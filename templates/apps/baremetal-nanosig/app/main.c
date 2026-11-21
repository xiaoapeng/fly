#include "biz/heartbeat.h"
#include "board.h"
#include "board_time.h"
#include "mid/firmware_info.h"
#include "runtime.h"

#include <nanosig/nanosig_status.h>

int main(void)
{
    board_init();
    fly_log_firmware_info();
    if (!board_time_start()) {
        board_log("board timebase failed\n");
        goto idle;
    }
    if (app_runtime_init() != NS_OK) {
        board_log("nanosig runtime initialization failed\n");
        goto idle;
    }
    if (heartbeat_init(app_runtime_get_loop()) != NS_OK) {
        (void)app_runtime_deinit();
        board_log("heartbeat initialization failed\n");
        goto idle;
    }

    const int run_status = app_runtime_run();
    const int heartbeat_status = heartbeat_deinit();
    const int runtime_status = app_runtime_deinit();
    board_log(run_status == NS_OK && heartbeat_status == NS_OK && runtime_status == NS_OK
              ? "nanosig stopped\n" : "nanosig application failed\n");

idle:
    for (;;) {
        board_idle();
    }
}
