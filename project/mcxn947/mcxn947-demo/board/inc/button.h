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

#include "fsl_gpio.h"
#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/* Symbols to be used with GPIO driver */
#define BOARD_BUTTON_PINS_SW3_GPIO GPIO0               /*!<@brief GPIO peripheral base pointer */
#define BOARD_BUTTON_PINS_SW3_GPIO_PIN 6U              /*!<@brief GPIO pin number */
#define BOARD_BUTTON_PINS_SW3_GPIO_PIN_MASK (1U << 6U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_BUTTON_PINS_SW3_PORT PORT0               /*!<@brief PORT peripheral base pointer */
#define BOARD_BUTTON_PINS_SW3_PIN 6U                   /*!<@brief PORT pin number */
#define BOARD_BUTTON_PINS_SW3_PIN_MASK (1U << 6U)      /*!<@brief PORT pin mask */

/* Alias for GPIO0 peripheral */
#define GPIO0_GPIO GPIO0
/* Alias for PORT0 */
#define GPIO0_PORT PORT0
/* GPIO0 interrupt vector ID (number). */
#define GPIO0_INT_0_IRQN GPIO00_IRQn
/* GPIO0 interrupt vector priority. */
#define GPIO0_INT_0_IRQ_PRIORITY 0


EH_EXTERN_SIGNAL(button_sw3_signal);
/* 定义信号和触发器 */

#define button_sw3_sta()   GPIO_PinRead(BOARD_BUTTON_PINS_SW3_GPIO, BOARD_BUTTON_PINS_SW3_GPIO_PIN)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _BUTTON_H_