/**
 * @file button.h
 * @brief 按键
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-07-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "gd32vf103.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define BUTTON_PB5_PIN GPIO_PIN_5
#define BUTTON_PB5_EXTI EXTI5_9_IRQn
#define BUTTON_PB5_GPIO_PORT GPIOB
#define BUTTON_PB5_GPIO_CLK RCU_GPIOB


#define BUTTON_PA0_PIN GPIO_PIN_0
#define BUTTON_PA0_EXTI EXTI0_IRQn
#define BUTTON_PA0_GPIO_PORT GPIOA
#define BUTTON_PA0_GPIO_CLK RCU_GPIOA

EH_EXTERN_SIGNAL(button_pb5_signal);
EH_EXTERN_SIGNAL(button_pa0_signal);
/* 定义信号和触发器 */

#define button_pb4_sta()   gpio_input_bit_get(BUTTON_GPIO_PORT, BUTTON_PIN)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _BUTTON_H_