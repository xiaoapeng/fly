/**
 * @file main.c
 * @brief FreeRTOS 2-task demo: RTT tick + LED blink on mcxn947
 */

#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"
#include "led.h"
#include <stdio.h>

extern void init(void);

static void tick_task(void *p)
{
    (void)p;
    /* n counts half-second slots: one full second every 2 lines. */
    uint32_t n = 0;
    for (;;) {
        char msg[32];
        int len = snprintf(msg, sizeof(msg), "tick @%u\n", (unsigned)n);
        if (len > 0) {
            SEGGER_RTT_Write(0, msg, (unsigned)len);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        n++;
    }
}

static void led_task(void *p)
{
    (void)p;
    uint8_t v = 0;
    for (;;) {
        led_set(v ^= 0x01u);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    init();
    SEGGER_RTT_Write(0, "freertos-demo\n", 13);

    xTaskCreate(tick_task, "tick", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(led_task,  "led",  configMINIMAL_STACK_SIZE, NULL, 2, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
    return 0;
}