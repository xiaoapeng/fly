/**
 * @file led.c
 * @brief led控制驱动
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-07-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include <eh_module.h>

#include "led.h"

#include "stm32h7xx_ll_gpio.h"
#include "stm32h7xx_ll_bus.h"


int __init board_led_init(void)
{
    LL_GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* GPIO Ports Clock Enable */
    /**/
    LL_GPIO_ResetOutputPin(RUN_LED_GPIO_GPIO_Port, RUN_LED_GPIO_Pin);

    /**/
    GPIO_InitStruct.Pin = RUN_LED_GPIO_Pin;
    GPIO_InitStruct.Mode = LL_GPIO_MODE_OUTPUT;
    GPIO_InitStruct.Speed = LL_GPIO_SPEED_FREQ_LOW;
    GPIO_InitStruct.OutputType = LL_GPIO_OUTPUT_PUSHPULL;
    LL_GPIO_Init(RUN_LED_GPIO_GPIO_Port, &GPIO_InitStruct);
    return 0;
}



eh_module_level0_export(board_led_init, NULL);