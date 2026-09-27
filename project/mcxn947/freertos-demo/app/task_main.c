/**
 * @file task_main.c
 * @brief FreeRTOS demo hooks
 *
 * Strong hook implementations for this demo: assert failures print their
 * location over Segger RTT (this demo already depends on RTT for its tick
 * output), then park. Stack overflow / malloc failure park silently.
 */
#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"
#include <stdio.h>

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
    (void)pcTaskName;
    for (;;) {
    }
}

void vApplicationMallocFailedHook(void)
{
    for (;;) {
    }
}