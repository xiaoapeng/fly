/**
 * @file led.c
 * @brief FreeRTOS demo RGB LED driver
 */

#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"
#include "pin_mux.h"
#include "led.h"

#define PCR_DSE_dse1 0x01u
#define PCR_IBE_ibe1 0x01u

void led_init(void)
{
    gpio_pin_config_t out_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U,
    };

    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Gpio1);
    CLOCK_EnableClock(kCLOCK_Port0);
    CLOCK_EnableClock(kCLOCK_Port1);

    GPIO_PinInit(BOARD_RGB_PINS_LED_RED_GPIO,
                 BOARD_RGB_PINS_LED_RED_PIN, &out_config);
    GPIO_PinInit(BOARD_RGB_PINS_LED_GREEN_GPIO,
                 BOARD_RGB_PINS_LED_GREEN_PIN, &out_config);
    GPIO_PinInit(BOARD_RGB_PINS_LED_BLUE_GPIO,
                 BOARD_RGB_PINS_LED_BLUE_PIN, &out_config);

    PORT_SetPinMux(BOARD_RGB_PINS_LED_RED_PORT,
                   BOARD_RGB_PINS_LED_RED_PIN, kPORT_MuxAlt0);
    PORT_SetPinMux(BOARD_RGB_PINS_LED_GREEN_PORT,
                   BOARD_RGB_PINS_LED_GREEN_PIN, kPORT_MuxAlt0);
    PORT_SetPinMux(BOARD_RGB_PINS_LED_BLUE_PORT,
                   BOARD_RGB_PINS_LED_BLUE_PIN, kPORT_MuxAlt0);

    PORT0->PCR[BOARD_RGB_PINS_LED_RED_PIN] =
        (PORT0->PCR[BOARD_RGB_PINS_LED_RED_PIN] &
         ~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)) |
        PORT_PCR_DSE(PCR_DSE_dse1) | PORT_PCR_IBE(PCR_IBE_ibe1);
    PORT0->PCR[BOARD_RGB_PINS_LED_GREEN_PIN] =
        (PORT0->PCR[BOARD_RGB_PINS_LED_GREEN_PIN] &
         ~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)) |
        PORT_PCR_DSE(PCR_DSE_dse1) | PORT_PCR_IBE(PCR_IBE_ibe1);
    PORT1->PCR[BOARD_RGB_PINS_LED_BLUE_PIN] =
        (PORT1->PCR[BOARD_RGB_PINS_LED_BLUE_PIN] &
         ~(PORT_PCR_DSE_MASK | PORT_PCR_IBE_MASK)) |
        PORT_PCR_DSE(PCR_DSE_dse1) | PORT_PCR_IBE(PCR_IBE_ibe1);

    led_set(0);
}

/* mask bit0=red, bit1=green, bit2=blue */
void led_set(uint8_t mask)
{
    GPIO_PinWrite(BOARD_RGB_PINS_LED_RED_GPIO,
                  BOARD_RGB_PINS_LED_RED_GPIO_PIN,
                  (mask & 0x01u) ? 1U : 0U);
    GPIO_PinWrite(BOARD_RGB_PINS_LED_GREEN_GPIO,
                  BOARD_RGB_PINS_LED_GREEN_GPIO_PIN,
                  (mask & 0x02u) ? 1U : 0U);
    GPIO_PinWrite(BOARD_RGB_PINS_LED_BLUE_GPIO,
                  BOARD_RGB_PINS_LED_BLUE_GPIO_PIN,
                  (mask & 0x04u) ? 1U : 0U);
}