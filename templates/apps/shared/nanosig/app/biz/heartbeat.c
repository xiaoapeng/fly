#include "biz/heartbeat.h"

#include <nanosig/nanosig_timer.h>
#include <nanosig/ns_debug.h>

static const ns_time_us_t heartbeat_period_us = 1000000u;
static ns_timer_t heartbeat_timer;
static ns_connection_t heartbeat_connection;

static void on_heartbeat(void *user_data, const void *payload)
{
    (void)user_data;
    (void)payload;
    ns_sysln("heartbeat");
}

int heartbeat_init(ns_loop_t *loop)
{
    int status = ns_timer_init(&heartbeat_timer, heartbeat_period_us, NS_TIMER_ATTR_REPEAT);
    if (status != NS_OK) {
        return status;
    }

    status = ns_signal_connect(&heartbeat_timer.signal, on_heartbeat, loop, NULL, &heartbeat_connection);
    if (status == NS_OK) {
        status = ns_timer_start(&heartbeat_timer);
        if (status == NS_OK) {
            return NS_OK;
        }
        (void)ns_signal_disconnect(&heartbeat_connection);
    }

    (void)ns_timer_deinit(&heartbeat_timer);
    return status;
}

int heartbeat_deinit(void)
{
    const int status = ns_signal_disconnect(&heartbeat_connection);
    const int timer_status = ns_timer_deinit(&heartbeat_timer);
    return status == NS_OK ? timer_status : status;
}
