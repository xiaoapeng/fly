#include "biz/app_demo.h"
#include "board.h"
#include "mid/firmware_info.h"
#include "runtime.h"

#include <FreeRTOS.h>
#include <task.h>

#include <nanosig/nanosig_status.h>

static _Noreturn void stop(const char *message)
{
    board_log(message);
    taskDISABLE_INTERRUPTS();
    for (;;) {
        board_idle();
    }
}

static void run_app(void *argument)
{
    (void)argument;
    if (app_runtime_init() != NS_OK) {
        stop("nanosig runtime initialization failed\n");
    }
    if (app_demo_init(app_runtime_get_loop()) != NS_OK) {
        (void)app_runtime_deinit();
        stop("nanosig demo initialization failed\n");
    }

    const int run_status = app_runtime_run();
    const int demo_status = app_demo_deinit();
    const int runtime_status = app_runtime_deinit();
    if (run_status != NS_OK || demo_status != NS_OK || runtime_status != NS_OK) {
        stop("nanosig demo failed\n");
    }
    board_log("nanosig stopped\n");
    vTaskDelete(NULL);
}

int main(void)
{
    board_init();
    fly_log_firmware_info();
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
