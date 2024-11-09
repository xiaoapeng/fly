/**
 * @file led.h
 * @brief led驱动
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-07-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _LED_H_
#define _LED_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


/*! @name PORT0_10 (coord B12), P0_10/SJ5[1]
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_RGB_PINS_LED_RED_GPIO GPIO0                /*!<@brief GPIO peripheral base pointer */
#define BOARD_RGB_PINS_LED_RED_GPIO_PIN 10U              /*!<@brief GPIO pin number */
#define BOARD_RGB_PINS_LED_RED_GPIO_PIN_MASK (1U << 10U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_RGB_PINS_LED_RED_PORT PORT0                /*!<@brief PORT peripheral base pointer */
#define BOARD_RGB_PINS_LED_RED_PIN 10U                   /*!<@brief PORT pin number */
#define BOARD_RGB_PINS_LED_RED_PIN_MASK (1U << 10U)      /*!<@brief PORT pin mask */
                                                         /* @} */

/*! @name PORT0_27 (coord E10), P0_27/SJ6[1]
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_RGB_PINS_LED_GREEN_GPIO GPIO0                /*!<@brief GPIO peripheral base pointer */
#define BOARD_RGB_PINS_LED_GREEN_GPIO_PIN 27U              /*!<@brief GPIO pin number */
#define BOARD_RGB_PINS_LED_GREEN_GPIO_PIN_MASK (1U << 27U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_RGB_PINS_LED_GREEN_PORT PORT0                /*!<@brief PORT peripheral base pointer */
#define BOARD_RGB_PINS_LED_GREEN_PIN 27U                   /*!<@brief PORT pin number */
#define BOARD_RGB_PINS_LED_GREEN_PIN_MASK (1U << 27U)      /*!<@brief PORT pin mask */
                                                           /* @} */

/*! @name PORT1_2 (coord C4), P1_2/SJ4[1]
  @{ */

/* Symbols to be used with GPIO driver */
#define BOARD_RGB_PINS_LED_BLUE_GPIO GPIO1               /*!<@brief GPIO peripheral base pointer */
#define BOARD_RGB_PINS_LED_BLUE_GPIO_PIN 2U              /*!<@brief GPIO pin number */
#define BOARD_RGB_PINS_LED_BLUE_GPIO_PIN_MASK (1U << 2U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_RGB_PINS_LED_BLUE_PORT PORT1               /*!<@brief PORT peripheral base pointer */
#define BOARD_RGB_PINS_LED_BLUE_PIN 2U                   /*!<@brief PORT pin number */
#define BOARD_RGB_PINS_LED_BLUE_PIN_MASK (1U << 2U)      /*!<@brief PORT pin mask */
                                                         /* @} */
/* Symbols to be used with GPIO driver */
#define BOARD_RGB_PINS_TEST_OUT_GPIO GPIO0                /*!<@brief GPIO peripheral base pointer */
#define BOARD_RGB_PINS_TEST_OUT_GPIO_PIN 15U              /*!<@brief GPIO pin number */
#define BOARD_RGB_PINS_TEST_OUT_GPIO_PIN_MASK (1U << 15U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_RGB_PINS_TEST_OUT_PORT PORT0                /*!<@brief PORT peripheral base pointer */
#define BOARD_RGB_PINS_TEST_OUT_PIN 15U                   /*!<@brief PORT pin number */
#define BOARD_RGB_PINS_TEST_OUT_PIN_MASK (1U << 15U)      /*!<@brief PORT pin mask */
                                                          /* @} */

#define led_rgb_r_set_val(val) GPIO_PinWrite(BOARD_RGB_PINS_LED_RED_GPIO, BOARD_RGB_PINS_LED_RED_GPIO_PIN, !(val))
#define led_rgb_g_set_val(val) GPIO_PinWrite(BOARD_RGB_PINS_LED_GREEN_GPIO, BOARD_RGB_PINS_LED_GREEN_GPIO_PIN, !(val))
#define led_rgb_b_set_val(val) GPIO_PinWrite(BOARD_RGB_PINS_LED_BLUE_GPIO, BOARD_RGB_PINS_LED_BLUE_GPIO_PIN, !(val))
#define led_test_out_set_val(val) GPIO_PinWrite(BOARD_RGB_PINS_TEST_OUT_GPIO, BOARD_RGB_PINS_TEST_OUT_GPIO_PIN, val)
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _LED_H_