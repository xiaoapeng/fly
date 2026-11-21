#include "board.h"
#include "board_time.h"

#include <nanosig/nanosig.h>
#include <nanosig/nanosig_timer.h>
#include <nanosig_baremetal_api.h>

static _Noreturn void stop(const char *message)
{
    board_log(message);
    for (;;) {
        board_idle();
    }
}

static void on_ready(void *user_data, const void *payload)
{
    (void)user_data;
    (void)payload;
    board_log("nanosig ready\n");
}

int main(void)
{
    board_init();
    if (!board_time_start()) {
        stop("board timebase failed\n");
    }

    ns_baremetal_set_now_us(board_time_us);
    if (ns_init() != NS_OK) {
        stop("nanosig initialization failed\n");
    }

    ns_loop_t *loop;
    if (ns_loop_create(&loop, NULL) != NS_OK) {
        stop("nanosig loop creation failed\n");
    }

    /* The start-up timer and its connection live for the lifetime of the loop. */
    ns_timer_t ready_timer;
    ns_connection_t ready_connection;
    if (ns_timer_init(&ready_timer, 100000u, NS_TIMER_ATTR_ONESHOT) != NS_OK ||
        ns_signal_connect(&ready_timer.signal, on_ready, loop, NULL, &ready_connection) != NS_OK ||
        ns_timer_start(&ready_timer) != NS_OK) {
        stop("nanosig start-up timer failed\n");
    }

    for (;;) {
        if (ns_loop_poll(loop, 0u) != NS_OK) {
            stop("nanosig poll failed\n");
        }
        board_idle();
    }
}
