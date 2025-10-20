/**
 * @file button.c
 * @brief 基础按钮驱动
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-07-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */


#include <eh_module.h>
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#include <eh.h>
#include <eh_signal.h>
#include <string.h>
#include "button.h"
#include "led.h"

EH_DEFINE_SIGNAL(button_sw3_signal);

#define PCR_IBE_ibe1 0x01u /*!<@brief Input Buffer Enable: Enables */
#define PCR_PE_pe0 0x00u   /*!<@brief Pull Enable: Disables */



void GPIO00_IRQHandler(void){
    GPIO_GpioClearInterruptFlags(GPIO0, BOARD_BUTTON_PINS_SW3_GPIO_PIN_MASK);
    eh_signal_notify(&button_sw3_signal);
}


static int __init board_button_init(void)
{

    //event_button_sw3

    /* Enables the clock for GPIO0: Enables clock */
    CLOCK_EnableClock(kCLOCK_Gpio0);
    /* Enables the clock for PORT0 controller: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port0);

    gpio_pin_config_t SW3_config = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO0_6 (pin C14)  */
    GPIO_PinInit(BOARD_BUTTON_PINS_SW3_GPIO, BOARD_BUTTON_PINS_SW3_PIN, &SW3_config);

    /* Interrupt configuration on GPIO0_6 (pin C14): Interrupt on either edge */
    GPIO_SetPinInterruptConfig(BOARD_BUTTON_PINS_SW3_GPIO, BOARD_BUTTON_PINS_SW3_PIN, kGPIO_InterruptEitherEdge);

    /* PORT0_6 (pin C14) is configured as PIO0_6 */
    PORT_SetPinMux(BOARD_BUTTON_PINS_SW3_PORT, BOARD_BUTTON_PINS_SW3_PIN, kPORT_MuxAlt0);

    PORT0->PCR[6] = ((PORT0->PCR[6] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_PE_MASK | PORT_PCR_IBE_MASK)))

                     /* Pull Enable: Disables. */
                     | PORT_PCR_PE(PCR_PE_pe0)

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));
    /* Make sure, the clock gate for port 0 is enabled (e. g. in pin_mux.c) */
    /* Interrupt vector GPIO00_IRQn priority settings in the NVIC. */
    NVIC_SetPriority(GPIO0_INT_0_IRQN, GPIO0_INT_0_IRQ_PRIORITY);
    /* Enable interrupt GPIO00_IRQn request in the NVIC. */
    EnableIRQ(GPIO0_INT_0_IRQN);

    return 0;
}


eh_module_level0_export(board_button_init, NULL);




