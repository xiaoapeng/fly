/**
 * @file task_main.c
 * @brief FreeRTOS demo: the sole app task plus the strong hooks
 *
 * Strong hook implementations for this demo: assert failures print their
 * location over Segger RTT (this demo already depends on RTT for its log
 * output), then park. Stack overflow / malloc failure park silently.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"

#include "key.h"
#include "runtime.h"
#include "task_main.h"
#include "test.h"

#include <stdio.h>

/* Sole thread: owns the nanosig main loop for the whole app lifetime. */
void main_task(void *p)
{
    (void)p;

    int rc = runtime_init();
    if(rc == NS_OK) {
        rc = key_init();
    }
    if(rc == NS_OK) {
        rc = test_init();
    }
    if(rc != NS_OK) {
        char msg[48];
        int len = snprintf(msg, sizeof(msg), "init failed rc=%d\n", rc);
        if(len > 0) {
            SEGGER_RTT_Write(0, msg, (unsigned)len);
        }
        vTaskDelete(NULL);
        return;
    }

    (void)runtime_run();
    (void)test_deinit();
    (void)key_deinit();
    (void)runtime_deinit();
    SEGGER_RTT_WriteString(0, "runtime exited\n");
    vTaskDelete(NULL);
}

void vAssertCalled(const char *pcFile, unsigned long ulLine)
{
    /* Halt with the assert location on RTT so rttlog shows it. */
    static char msg[128];
    int n = snprintf(msg, sizeof(msg), "ASSERT: %s:%lu\n", pcFile, ulLine);
    if (n > 0) {
        SEGGER_RTT_Write(0, msg, (unsigned)n);
    }
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    static char msg[96];
    int n = snprintf(msg, sizeof(msg), "STACK OVERFLOW in task: %s\n",
                     pcTaskName ? pcTaskName : "?");
    if (n > 0) {
        SEGGER_RTT_Write(0, msg, (unsigned)n);
    }
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    for (;;) {
    }
}