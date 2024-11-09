/**
 * @file button.c
 * @brief 基础按键驱动
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-09-21
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */


#include <eh.h>
#include <eh_module.h>
#include <eh_signal.h>
#include "button.h"

EH_DEFINE_SIGNAL(button_pb5_signal);
EH_DEFINE_SIGNAL(button_pa0_signal);


void EXTI0_IRQHandler(void){
    if(RESET != exti_interrupt_flag_get(EXTI_0)) {
        exti_interrupt_flag_clear(EXTI_0);
        eh_signal_notify(&button_pa0_signal);
    }
}

void EXTI5_9_IRQHandler(void){
    if(RESET != exti_interrupt_flag_get(EXTI_5)) {
        exti_interrupt_flag_clear(EXTI_5);
        eh_signal_notify(&button_pb5_signal);
    }
}


static int __init board_button_init(void)
{
    eh_signal_register(&button_pb5_signal);
    eh_signal_register(&button_pa0_signal);
    
    rcu_periph_clock_enable(BUTTON_PB5_GPIO_CLK);
    gpio_init(BUTTON_PB5_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, BUTTON_PB5_PIN);
    
    eclic_irq_enable(BUTTON_PB5_EXTI, 1U, 0U);

    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOB, GPIO_PIN_SOURCE_5);

    exti_init(EXTI_5, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_5);

    
    rcu_periph_clock_enable(BUTTON_PA0_GPIO_CLK);
    gpio_init(BUTTON_PA0_GPIO_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, BUTTON_PA0_PIN);
    
    eclic_irq_enable(BUTTON_PA0_EXTI, 2U, 0U);

    gpio_exti_source_select(GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_0);

    exti_init(EXTI_0, EXTI_INTERRUPT, EXTI_TRIG_FALLING);
    exti_interrupt_flag_clear(EXTI_0);


    return 0;
}

static void __exit board_button_exit(void){
    eh_signal_unregister(&button_pa0_signal);
    eh_signal_unregister(&button_pb5_signal);
}

eh_module_level0_export(board_button_init, board_button_exit);




