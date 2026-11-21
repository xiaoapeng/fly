#include "biz/app_demo.h"
#include "runtime.h"

#include <nanosig/nanosig_timer.h>
#include <nanosig/ns_debug.h>

static ns_timer_t ready_timer;
static ns_connection_t ready_connection;

static void on_ready(void *user_data, const void *payload)
{
    (void)user_data;
    (void)payload;
    ns_sysln("nanosig ready");
    (void)app_runtime_request_stop();
}

int app_demo_init(ns_loop_t *loop)
{
    int status = ns_timer_init(&ready_timer, 100000u, NS_TIMER_ATTR_ONESHOT);
    if (status != NS_OK) {
        return status;
    }

    status = ns_signal_connect(&ready_timer.signal, on_ready, loop, NULL, &ready_connection);
    if (status == NS_OK) {
        status = ns_timer_start(&ready_timer);
        if (status == NS_OK) {
            return NS_OK;
        }
        (void)ns_signal_disconnect(&ready_connection);
    }

    (void)ns_timer_deinit(&ready_timer);
    return status;
}

int app_demo_deinit(void)
{
    int status = ns_signal_disconnect(&ready_connection);
    int timer_status = ns_timer_deinit(&ready_timer);
    return status == NS_OK ? timer_status : status;
}
