/**
 * @file led.h
 * @brief  LED 控制
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-08-11
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _LED_H_
#define _LED_H_

#include "stm32h7xx_ll_gpio.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


#define RUN_LED_GPIO_Pin LL_GPIO_PIN_13
#define RUN_LED_GPIO_GPIO_Port GPIOC

#define led_gpio_on()       LL_GPIO_TogglePin(RUN_LED_GPIO_GPIO_Port, RUN_LED_GPIO_Pin)
#define led_gpio_off()      LL_GPIO_ResetOutputPin(RUN_LED_GPIO_GPIO_Port, RUN_LED_GPIO_Pin)

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _LED_H_