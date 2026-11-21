#include "board.h"

#include <FreeRTOS.h>
#include <task.h>

#include <nanosig/nanosig.h>
#include <nanosig/nanosig_timer.h>

static _Noreturn void stop(const char *message)
{
    board_log(message);
    taskDISABLE_INTERRUPTS();
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

static void run_app(void *argument)
{
    (void)argument;
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

    (void)ns_loop_run(loop);
    stop("nanosig loop stopped\n");
}

int main(void)
{
    board_init();
    const configSTACK_DEPTH_TYPE app_stack_words = 1024u;
    if (xTaskCreate(run_app, "app", app_stack_words, NULL, tskIDLE_PRIORITY + 1u, NULL) != pdPASS) {
        stop("FreeRTOS task creation failed\n");
    }

    vTaskStartScheduler();
    stop("FreeRTOS scheduler failed to start\n");
}

void vApplicationStackOverflowHook(TaskHandle_t task, char *name)
{
    (void)task;
    (void)name;
    stop("FreeRTOS task stack overflow\n");
}
