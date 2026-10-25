/**
 * @file main.c
 * @brief FreeRTOS demo entry: board init, then start the sole app task
 */

#include "FreeRTOS.h"
#include "task.h"
#include "SEGGER_RTT.h"

#include "task_main.h"

extern void init(void);

int main(void)
{
    init();
    SEGGER_RTT_Write(0, "freertos-demo\n", 13);

    xTaskCreate(main_task, "main", configMINIMAL_STACK_SIZE * 8, NULL, 2, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
    return 0;
}
