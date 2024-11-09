/**
 * @file led.h
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-09-21
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */



#ifndef _LED_H_
#define _LED_H_


#include "gd32vf103.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define LED_PIN GPIO_PIN_13
#define LED_GPIO_PORT GPIOC
#define LED_GPIO_CLK RCU_GPIOC

#define led_on()  gpio_bit_set(LED_GPIO_PORT, LED_PIN)
#define led_off() gpio_bit_reset(LED_GPIO_PORT, LED_PIN)
#define led_toggle() gpio_bit_write(LED_GPIO_PORT, LED_PIN, !gpio_output_bit_get(LED_GPIO_PORT, LED_PIN))

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _LED_H_