/**
 * @file led.c
 * @brief led控制驱动
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-07-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include "eh.h"
#include "eh_module.h"
#include "fsl_clock.h"
#include "fsl_gpio.h"

#include "fsl_port.h"
#include "led.h"

#define PCR_DSE_dse1 0x01u /*!<@brief Drive Strength Enable: High */
#define PCR_IBE_ibe1 0x01u /*!<@brief Input Buffer Enable: Enables */

int __init board_rgb_init(void)
{
/* Enables the clock for GPIO0: Enables clock */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for GPIO1: Enables clock */
    CLOCK_EnableClock(kCLOCK_Gpio1);
    /* Enables the clock for PORT0 controller: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port0);
    /* Enables the clock for PORT1: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port1);

    gpio_pin_config_t LED_RED_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO0_10 (pin B12)  */
    GPIO_PinInit(BOARD_RGB_PINS_LED_RED_GPIO, BOARD_RGB_PINS_LED_RED_PIN, &LED_RED_config);

    gpio_pin_config_t TEST_OUT_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO0_15 (pin G13)  */
    GPIO_PinInit(BOARD_RGB_PINS_TEST_OUT_GPIO, BOARD_RGB_PINS_TEST_OUT_PIN, &TEST_OUT_config);

    gpio_pin_config_t LED_GREEN_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO0_27 (pin E10)  */
    GPIO_PinInit(BOARD_RGB_PINS_LED_GREEN_GPIO, BOARD_RGB_PINS_LED_GREEN_PIN, &LED_GREEN_config);

    gpio_pin_config_t LED_BLUE_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO1_2 (pin C4)  */
    GPIO_PinInit(BOARD_RGB_PINS_LED_BLUE_GPIO, BOARD_RGB_PINS_LED_BLUE_PIN, &LED_BLUE_config);

    /* PORT0_10 (pin B12) is configured as PIO0_10 */
    PORT_SetPinMux(BOARD_RGB_PINS_LED_RED_PORT, BOARD_RGB_PINS_LED_RED_PIN, kPORT_MuxAlt0);

    PORT0->PCR[10] = ((PORT0->PCR[10] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)))

                      /* Drive Strength Enable: High. */
                      | PORT_PCR_DSE(PCR_DSE_dse1)

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT0_15 (pin G13) is configured as PIO0_15 */
    PORT_SetPinMux(BOARD_RGB_PINS_TEST_OUT_PORT, BOARD_RGB_PINS_TEST_OUT_PIN, kPORT_MuxAlt0);

    PORT0->PCR[15] = ((PORT0->PCR[15] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)))

                      /* Drive Strength Enable: High. */
                      | PORT_PCR_DSE(PCR_DSE_dse1)

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT0_27 (pin E10) is configured as PIO0_27 */
    PORT_SetPinMux(BOARD_RGB_PINS_LED_GREEN_PORT, BOARD_RGB_PINS_LED_GREEN_PIN, kPORT_MuxAlt0);

    PORT0->PCR[27] = ((PORT0->PCR[27] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)))

                      /* Drive Strength Enable: High. */
                      | PORT_PCR_DSE(PCR_DSE_dse1)

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_2 (pin C4) is configured as PIO1_2 */
    PORT_SetPinMux(BOARD_RGB_PINS_LED_BLUE_PORT, BOARD_RGB_PINS_LED_BLUE_PIN, kPORT_MuxAlt0);

    PORT1->PCR[2] = ((PORT1->PCR[2] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)))

                     /* Drive Strength Enable: High. */
                     | PORT_PCR_DSE(PCR_DSE_dse1)

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));
    led_rgb_r_set_val(0);
    led_rgb_g_set_val(0);
    led_rgb_b_set_val(0);
    return 0;
}

eh_module_level0_export(board_rgb_init, NULL);