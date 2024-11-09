/**
 * @file led.c
 * @brief  led 控制驱动
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-09-21
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */


#include "eh.h"
#include "eh_module.h"
#include "led.h"

#include "gd32vf103.h"



int __init board_rgb_init(void){
    /* enable the led clock */
    rcu_periph_clock_enable(LED_GPIO_CLK);
    gpio_init(LED_GPIO_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, LED_PIN);
    GPIO_BC(LED_GPIO_PORT) = LED_PIN;
    return 0;
}

eh_module_level0_export(board_rgb_init, NULL);