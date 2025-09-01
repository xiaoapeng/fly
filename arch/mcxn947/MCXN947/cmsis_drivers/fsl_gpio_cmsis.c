/*
 * Copyright 2024 NXP
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "fsl_gpio_cmsis.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

/* Component ID definition, used by tools. */
#ifndef FSL_COMPONENT_ID
#define FSL_COMPONENT_ID "platform.drivers.gpio_cmsis"
#endif

#if (defined(RTE_GPIO_PORT0) && RTE_GPIO_PORT0 && defined(GPIO0)) || \
    (defined(RTE_GPIO_PORT1) && RTE_GPIO_PORT1 && defined(GPIO1)) || \
    (defined(RTE_GPIO_PORT2) && RTE_GPIO_PORT2 && defined(GPIO2)) || \
    (defined(RTE_GPIO_PORT3) && RTE_GPIO_PORT3 && defined(GPIO3)) || \
    (defined(RTE_GPIO_PORT4) && RTE_GPIO_PORT4 && defined(GPIO4)) || \
    (defined(RTE_GPIO_PORT5) && RTE_GPIO_PORT5 && defined(GPIO5)) || \
    (defined(RTE_GPIO_PORTA) && RTE_GPIO_PORTA && defined(GPIOA)) || \
    (defined(RTE_GPIO_PORTB) && RTE_GPIO_PORTB && defined(GPIOB)) || \
    (defined(RTE_GPIO_PORTC) && RTE_GPIO_PORTC && defined(GPIOC)) || \
    (defined(RTE_GPIO_PORTD) && RTE_GPIO_PORTD && defined(GPIOD)) || \
    (defined(RTE_GPIO_PORTE) && RTE_GPIO_PORTE && defined(GPIOE))

/*!
 * @brief Type define for the pin state variable
 *
 */
typedef uint32_t gpio_cmsis_pin_state_t;

/*!
 * @brief Configuration for CMSIS GPIO Inteface instance
 *
 */
typedef struct _gpio_cmsis_config
{
    GPIO_Type *gpio_base;
    PORT_Type *port_base;
} gpio_cmsis_config_t;

/*!
 * @brief Handle for CMSIS GPIO inteface instance
 *
 */
typedef struct _gpio_cmsis_handle
{
    gpio_cmsis_config_t const *config;
    ARM_GPIO_SignalEvent_t cb_event;
} gpio_cmsis_handle_t;

typedef void (*gpio_isr_t)(gpio_cmsis_handle_t *gpioHandle);

static gpio_isr_t s_gpioIsr;

void CMSIS_GPIO_EventIRQ(gpio_cmsis_handle_t *handle);
/*
 * CMSIS GPIO Version
 */
#define ARM_CMSIS_GPIO_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1U, 0U) /* driver version */

/*
 * ARMCC does not support split the data section automatically, so the driver
 * needs to split the data to separate sections explicitly, to reduce codesize.
 */
#if defined(__CC_ARM) || defined(__ARMCC_VERSION)
#define ARMCC_SECTION(section_name) __attribute__((section(section_name)))
#endif /* defined(__CC_ARM) || defined(__ARMCC_VERSION) */

static int32_t CMSIS_GPIO_Setup(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    gpio_pin_config_t pinConfig = {
        kGPIO_DigitalInput,
        0,
    };

    if(handle->cb_event == NULL)
    {
        handle->cb_event= cb_event; /* cb_event is CMSIS driver callback. */
    }

    /* Set pin as GPIO input without pull-resistor and event trigger. */
    PORT_SetPinMux(handle->config->port_base, pin, kPORT_MuxAsGpio);
#if !(defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT)
    PORT_SetPinInterruptConfig(handle->config->port_base, pin, kPORT_InterruptOrDMADisabled);
#endif
    handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_PE_MASK) | PORT_PCR_PE(0);

    GPIO_PinInit(handle->config->gpio_base, pin, &pinConfig);

    return ARM_DRIVER_OK;
}

static int32_t CMSIS_GPIO_SetDirection(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{

    if (direction == ARM_GPIO_INPUT)
    {
        handle->config->gpio_base->PDDR &= GPIO_FIT_REG(~(1UL << pin));
    }
    else
    {
        handle->config->gpio_base->PDDR |= GPIO_FIT_REG((1UL << pin));
    }

    return ARM_DRIVER_OK;
}
static int32_t CMSIS_GPIO_SetOutputMode(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
#if defined(definedSL_FEATURE_PORT_HAS_OPEN_DRAIN) && definedSL_FEATURE_PORT_HAS_OPEN_DRAIN
    handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_ODE_MASK) | PORT_PCR_ODE(mode);
    return ARM_DRIVER_OK;
#else
    return ARM_DRIVER_ERROR_UNSUPPORTED;
#endif
}
static int32_t CMSIS_GPIO_SetPullResistor(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    switch (resistor)
    {
        case ARM_GPIO_PULL_NONE:
            handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_PE_MASK) | PORT_PCR_PE(0);
            break;
        case ARM_GPIO_PULL_UP:
            handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_PE_MASK) | PORT_PCR_PE(1);
            handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_PS_MASK) | PORT_PCR_PS(1);
            break;
        case ARM_GPIO_PULL_DOWN:
            handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_PE_MASK) | PORT_PCR_PE(1);
            handle->config->port_base->PCR[pin] = (handle->config->port_base->PCR[pin] & ~PORT_PCR_PS_MASK) | PORT_PCR_PS(0);
            break;
        default:
            assert(false);
            break;
    }

    return ARM_DRIVER_OK;
}
static int32_t CMSIS_GPIO_SetEventTrigger(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
    s_gpioIsr = CMSIS_GPIO_EventIRQ;
#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT) || \
    (!defined(FSL_FEATURE_SOC_PORT_COUNT))
    switch (trigger)
    {
        case ARM_GPIO_TRIGGER_NONE:
            GPIO_SetPinInterruptConfig(handle->config->gpio_base, pin, kGPIO_InterruptStatusFlagDisabled);
            break;
        case ARM_GPIO_TRIGGER_RISING_EDGE:
            GPIO_SetPinInterruptConfig(handle->config->gpio_base, pin, kGPIO_InterruptRisingEdge);
            break;
        case ARM_GPIO_TRIGGER_FALLING_EDGE:
            GPIO_SetPinInterruptConfig(handle->config->gpio_base, pin, kGPIO_InterruptFallingEdge);
            break;
        case ARM_GPIO_TRIGGER_EITHER_EDGE:
            GPIO_SetPinInterruptConfig(handle->config->gpio_base, pin, kGPIO_InterruptEitherEdge);
            break;
        default:
            assert(false);
            break;
    }
#else
    switch (trigger)
    {
        case ARM_GPIO_TRIGGER_NONE:
            PORT_SetPinInterruptConfig(handle->config->port_base, pin, kPORT_InterruptOrDMADisabled);
            break;
        case ARM_GPIO_TRIGGER_RISING_EDGE:
            PORT_SetPinInterruptConfig(handle->config->port_base, pin, kPORT_InterruptRisingEdge);
            break;
        case ARM_GPIO_TRIGGER_FALLING_EDGE:
            PORT_SetPinInterruptConfig(handle->config->port_base, pin, kPORT_InterruptFallingEdge);
            break;
        case ARM_GPIO_TRIGGER_EITHER_EDGE:
            PORT_SetPinInterruptConfig(handle->config->port_base, pin, kPORT_InterruptEitherEdge);
            break;
        default:
            assert(false);
            break;
    }
#endif

    return ARM_DRIVER_OK;
}

static void CMSIS_GPIO_SetOutput(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin, uint32_t val)
{
    GPIO_PinWrite(handle->config->gpio_base, pin, val);
}

static uint32_t CMSIS_GPIO_GetInput(gpio_cmsis_handle_t *handle, ARM_GPIO_Pin_t pin)
{
    return (GPIO_PinRead(handle->config->gpio_base, pin) != 0U);
}

void CMSIS_GPIO_EventIRQ(gpio_cmsis_handle_t *handle)
{
    assert(handle != NULL);

    uint32_t pins;
    uint32_t cb_event = 0U;
    uint32_t pin      = 0U;

#if (defined(FSL_FEATURE_PORT_HAS_NO_INTERRUPT) && FSL_FEATURE_PORT_HAS_NO_INTERRUPT) || \
    (!defined(FSL_FEATURE_SOC_PORT_COUNT))
    gpio_interrupt_config_t event = kGPIO_InterruptStatusFlagDisabled;
    pins = GPIO_GpioGetInterruptFlags(handle->config->gpio_base);

    while(pins >= 1U)
    {
        if((pins & (1U << pin)) != 0U)
        {
            event = (gpio_interrupt_config_t)((handle->config->gpio_base->ICR[pin] & GPIO_ICR_IRQC_MASK) >> GPIO_ICR_IRQC_SHIFT);
            break;
        }
        pin++;
    }

    switch (event)
    {
        case kGPIO_InterruptRisingEdge:
            cb_event = ARM_GPIO_TRIGGER_RISING_EDGE;
            break;
        case kGPIO_InterruptFallingEdge:
            cb_event = ARM_GPIO_TRIGGER_FALLING_EDGE;
            break;
        case kGPIO_InterruptEitherEdge:
            cb_event = ARM_GPIO_TRIGGER_EITHER_EDGE;
            break;
        default:
            assert(false);
            break;
    }

    GPIO_GpioClearInterruptFlags(handle->config->gpio_base, (1U << pin));
#else
    port_interrupt_t event = kPORT_InterruptOrDMADisabled;
    pins = GPIO_PortGetInterruptFlags(handle->config->gpio_base);

    while(pins >= 1U)
    {
        if((pins & (1U << pin)) != 0U)
        {
            event = (port_interrupt_t)((handle->config->port_base->PCR[pin] & PORT_PCR_IRQC_MASK) >> PORT_PCR_IRQC_SHIFT);
            break;
        }
        pin++;
    }

    switch (event)
    {
        case kPORT_InterruptRisingEdge:
            cb_event = ARM_GPIO_TRIGGER_RISING_EDGE;
            break;
        case kPORT_InterruptFallingEdge:
            cb_event = ARM_GPIO_TRIGGER_FALLING_EDGE;
            break;
        case kPORT_InterruptEitherEdge:
            cb_event = ARM_GPIO_TRIGGER_EITHER_EDGE;
            break;
        default:
            assert(false);
            break;
    }

    GPIO_PortClearInterruptFlags(handle->config->gpio_base, (1U << pin));
#endif
    handle->cb_event(pin, cb_event);
}
#endif //(RTE_GPIO_PORT0 && defined(GPIO0)) || (RTE_GPIO_PORT1 && defined(GPIO1))

/******************************* GPIO PORT 0 **********************************************/
#if defined(GPIO0) && defined(RTE_GPIO_PORT0) && RTE_GPIO_PORT0

static const gpio_cmsis_config_t s_gpio_port0_cmsis_config = {
    .gpio_base = GPIO0,
    .port_base = PORT0,
};

static gpio_cmsis_handle_t s_gpio_port0_handle = {
    .config = &s_gpio_port0_cmsis_config,
};

static int32_t GPIO_PORT0_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_port0_handle, pin, cb_event);
}

static int32_t GPIO_PORT0_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_port0_handle, pin, direction);
}

static int32_t GPIO_PORT0_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_port0_handle, pin, mode);
}

static int32_t GPIO_PORT0_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_port0_handle, pin, resistor);
}

static int32_t GPIO_PORT0_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIO0_IRQ
    EnableIRQ(RTE_GPIO0_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_port0_handle, pin, trigger);
}

static uint32_t GPIO_PORT0_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_port0_handle, pin);
}

static void GPIO_PORT0_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_port0_handle, pin, val);
}

#ifdef RTE_GPIO0_IRQ_HANDLER
void RTE_GPIO0_IRQ_HANDLER(void);
void RTE_GPIO0_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_port0_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORT0 = {
    .Setup           = GPIO_PORT0_Setup,
    .SetDirection    = GPIO_PORT0_SetDirection,
    .SetOutputMode   = GPIO_PORT0_SetOutputMode,
    .SetPullResistor = GPIO_PORT0_SetPullResistor,
    .SetEventTrigger = GPIO_PORT0_SetEventTrigger,
    .SetOutput       = GPIO_PORT0_SetOutput,
    .GetInput        = GPIO_PORT0_GetInput,
};
#endif

#if defined(GPIO1) && defined(RTE_GPIO_PORT1) && RTE_GPIO_PORT1
static const gpio_cmsis_config_t s_gpio_port1_cmsis_config = {
    .gpio_base = GPIO1,
    .port_base = PORT1,
};

static gpio_cmsis_handle_t s_gpio_port1_handle = {
    .config = &s_gpio_port1_cmsis_config,
};

static int32_t GPIO_PORT1_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_port1_handle, pin, cb_event);
}

static int32_t GPIO_PORT1_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_port1_handle, pin, direction);
}

static int32_t GPIO_PORT1_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_port1_handle, pin, mode);
}

static int32_t GPIO_PORT1_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_port1_handle, pin, resistor);
}

static int32_t GPIO_PORT1_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIO1_IRQ
    EnableIRQ(RTE_GPIO1_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_port1_handle, pin, trigger);
}

static uint32_t GPIO_PORT1_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_port1_handle, pin);
}

static void GPIO_PORT1_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_port1_handle, pin, val);
}

#ifdef RTE_GPIO1_IRQ_HANDLER
void RTE_GPIO1_IRQ_HANDLER(void);
void RTE_GPIO1_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_port1_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORT1 = {
    .Setup           = GPIO_PORT1_Setup,
    .SetDirection    = GPIO_PORT1_SetDirection,
    .SetOutputMode   = GPIO_PORT1_SetOutputMode,
    .SetPullResistor = GPIO_PORT1_SetPullResistor,
    .SetEventTrigger = GPIO_PORT1_SetEventTrigger,
    .SetOutput       = GPIO_PORT1_SetOutput,
    .GetInput        = GPIO_PORT1_GetInput,
};

#endif

#if defined(GPIO2) && defined(RTE_GPIO_PORT2) && RTE_GPIO_PORT2

static const gpio_cmsis_config_t s_gpio_port2_cmsis_config = {
    .gpio_base = GPIO2,
    .port_base = PORT2,
};

static gpio_cmsis_handle_t s_gpio_port2_handle = {
    .config = &s_gpio_port2_cmsis_config,
};

static int32_t GPIO_PORT2_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_port2_handle, pin, cb_event);
}

static int32_t GPIO_PORT2_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_port2_handle, pin, direction);
}

static int32_t GPIO_PORT2_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_port2_handle, pin, mode);
}

static int32_t GPIO_PORT2_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_port2_handle, pin, resistor);
}

static int32_t GPIO_PORT2_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIO2_IRQ
    EnableIRQ(RTE_GPIO2_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_port2_handle, pin, trigger);
}

static uint32_t GPIO_PORT2_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_port2_handle, pin);
}

static void GPIO_PORT2_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_port2_handle, pin, val);
}

#ifdef RTE_GPIO2_IRQ_HANDLER
void RTE_GPIO2_IRQ_HANDLER(void);
void RTE_GPIO2_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_port2_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORT2 = {
    .Setup           = GPIO_PORT2_Setup,
    .SetDirection    = GPIO_PORT2_SetDirection,
    .SetOutputMode   = GPIO_PORT2_SetOutputMode,
    .SetPullResistor = GPIO_PORT2_SetPullResistor,
    .SetEventTrigger = GPIO_PORT2_SetEventTrigger,
    .SetOutput       = GPIO_PORT2_SetOutput,
    .GetInput        = GPIO_PORT2_GetInput,
};

#endif

#if defined(GPIO3) && defined(RTE_GPIO_PORT3) && RTE_GPIO_PORT3

static const gpio_cmsis_config_t s_gpio_port3_cmsis_config = {
    .gpio_base = GPIO3,
    .port_base = PORT3,
};

static gpio_cmsis_handle_t s_gpio_port3_handle = {
    .config = &s_gpio_port3_cmsis_config,
};

static int32_t GPIO_PORT3_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_port3_handle, pin, cb_event);
}

static int32_t GPIO_PORT3_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_port3_handle, pin, direction);
}

static int32_t GPIO_PORT3_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_port3_handle, pin, mode);
}

static int32_t GPIO_PORT3_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_port3_handle, pin, resistor);
}

static int32_t GPIO_PORT3_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIO3_IRQ
    EnableIRQ(RTE_GPIO3_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_port3_handle, pin, trigger);
}

static uint32_t GPIO_PORT3_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_port3_handle, pin);
}

static void GPIO_PORT3_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_port3_handle, pin, val);
}

#ifdef RTE_GPIO3_IRQ_HANDLER
void RTE_GPIO3_IRQ_HANDLER(void);
void RTE_GPIO3_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_port3_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORT3 = {
    .Setup           = GPIO_PORT3_Setup,
    .SetDirection    = GPIO_PORT3_SetDirection,
    .SetOutputMode   = GPIO_PORT3_SetOutputMode,
    .SetPullResistor = GPIO_PORT3_SetPullResistor,
    .SetEventTrigger = GPIO_PORT3_SetEventTrigger,
    .SetOutput       = GPIO_PORT3_SetOutput,
    .GetInput        = GPIO_PORT3_GetInput,
};

#endif

#if defined(GPIO4) && defined(RTE_GPIO_PORT4) && RTE_GPIO_PORT4

static const gpio_cmsis_config_t s_gpio_port4_cmsis_config = {
    .gpio_base = GPIO4,
    .port_base = PORT4,
};

static gpio_cmsis_handle_t s_gpio_port4_handle = {
    .config = &s_gpio_port4_cmsis_config,
};

static int32_t GPIO_PORT4_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_port4_handle, pin, cb_event);
}

static int32_t GPIO_PORT4_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_port4_handle, pin, direction);
}

static int32_t GPIO_PORT4_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_port4_handle, pin, mode);
}

static int32_t GPIO_PORT4_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_port4_handle, pin, resistor);
}

static int32_t GPIO_PORT4_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIO4_IRQ
    EnableIRQ(RTE_GPIO4_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_port4_handle, pin, trigger);
}

static uint32_t GPIO_PORT4_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_port4_handle, pin);
}

static void GPIO_PORT4_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_port4_handle, pin, val);
}

#ifdef RTE_GPIO4_IRQ_HANDLER
void RTE_GPIO4_IRQ_HANDLER(void);
void RTE_GPIO4_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_port4_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORT4 = {
    .Setup           = GPIO_PORT4_Setup,
    .SetDirection    = GPIO_PORT4_SetDirection,
    .SetOutputMode   = GPIO_PORT4_SetOutputMode,
    .SetPullResistor = GPIO_PORT4_SetPullResistor,
    .SetEventTrigger = GPIO_PORT4_SetEventTrigger,
    .SetOutput       = GPIO_PORT4_SetOutput,
    .GetInput        = GPIO_PORT4_GetInput,
};
#endif

#if defined(GPIO5) && defined(RTE_GPIO_PORT5) && RTE_GPIO_PORT5

const gpio_cmsis_config_t s_gpio_port5_cmsis_config = {
    .gpio_base = GPIO5,
    .port_base = PORT5,
};

static gpio_cmsis_handle_t s_gpio_port5_handle = {
    .config         = &s_gpio_port5_cmsis_config,
};

static int32_t GPIO_PORT5_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_port5_handle, pin, cb_event);
}

static int32_t GPIO_PORT5_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_port5_handle, pin, direction);
}

static int32_t GPIO_PORT5_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_port5_handle, pin, mode);
}

static int32_t GPIO_PORT5_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_port5_handle, pin, resistor);
}

static int32_t GPIO_PORT5_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIO5_IRQ
    EnableIRQ(RTE_GPIO5_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_port5_handle, pin, trigger);
}

static uint32_t GPIO_PORT5_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_port5_handle, pin);
}

static void GPIO_PORT5_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_port5_handle, pin, val);
}

#ifdef RTE_GPIO5_IRQ_HANDLER
void RTE_GPIO5_IRQ_HANDLER(void);
void RTE_GPIO5_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_port5_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORT5 = {
    .Setup           = GPIO_PORT5_Setup,
    .SetDirection    = GPIO_PORT5_SetDirection,
    .SetOutputMode   = GPIO_PORT5_SetOutputMode,
    .SetPullResistor = GPIO_PORT5_SetPullResistor,
    .SetEventTrigger = GPIO_PORT5_SetEventTrigger,
    .SetOutput       = GPIO_PORT5_SetOutput,
    .GetInput        = GPIO_PORT5_GetInput,
};
#endif

#if defined(GPIOA) && defined(RTE_GPIO_PORTA) && RTE_GPIO_PORTA

const gpio_cmsis_config_t s_gpio_portA_cmsis_config = {
    .gpio_base = GPIOA,
    .port_base = PORTA,
};

static gpio_cmsis_handle_t s_gpio_portA_handle = {
    .config         = &s_gpio_portA_cmsis_config,
};

static int32_t GPIO_PORTA_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_portA_handle, pin, cb_event);
}

static int32_t GPIO_PORTA_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_portA_handle, pin, direction);
}

static int32_t GPIO_PORTA_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_portA_handle, pin, mode);
}

static int32_t GPIO_PORTA_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_portA_handle, pin, resistor);
}

static int32_t GPIO_PORTA_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIOA_IRQ
    EnableIRQ(RTE_GPIOA_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_portA_handle, pin, trigger);
}

static uint32_t GPIO_PORTA_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_portA_handle, pin);
}

static void GPIO_PORTA_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_portA_handle, pin, val);
}

#ifdef RTE_GPIOA_IRQ_HANDLER
void RTE_GPIOA_IRQ_HANDLER(void);
void RTE_GPIOA_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_portA_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORTA = {
    .Setup           = GPIO_PORTA_Setup,
    .SetDirection    = GPIO_PORTA_SetDirection,
    .SetOutputMode   = GPIO_PORTA_SetOutputMode,
    .SetPullResistor = GPIO_PORTA_SetPullResistor,
    .SetEventTrigger = GPIO_PORTA_SetEventTrigger,
    .SetOutput       = GPIO_PORTA_SetOutput,
    .GetInput        = GPIO_PORTA_GetInput,
};
#endif

#if defined(GPIOB) && defined(RTE_GPIO_PORTB) && RTE_GPIO_PORTB

const gpio_cmsis_config_t s_gpio_portB_cmsis_config = {
    .gpio_base = GPIOB,
    .port_base = PORTB,
};

static gpio_cmsis_handle_t s_gpio_portB_handle = {
    .config = &s_gpio_portB_cmsis_config,
};

static int32_t GPIO_PORTB_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_portB_handle, pin, cb_event);
}

static int32_t GPIO_PORTB_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_portB_handle, pin, direction);
}

static int32_t GPIO_PORTB_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_portB_handle, pin, mode);
}

static int32_t GPIO_PORTB_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_portB_handle, pin, resistor);
}

static int32_t GPIO_PORTB_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIOB_IRQ
    EnableIRQ(RTE_GPIOB_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_portB_handle, pin, trigger);
}

static uint32_t GPIO_PORTB_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_portB_handle, pin);
}

static void GPIO_PORTB_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_portB_handle, pin, val);
}

#ifdef RTE_GPIOB_IRQ_HANDLER
void RTE_GPIOB_IRQ_HANDLER(void);
void RTE_GPIOB_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_portB_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORTB = {
    .Setup           = GPIO_PORTB_Setup,
    .SetDirection    = GPIO_PORTB_SetDirection,
    .SetOutputMode   = GPIO_PORTB_SetOutputMode,
    .SetPullResistor = GPIO_PORTB_SetPullResistor,
    .SetEventTrigger = GPIO_PORTB_SetEventTrigger,
    .SetOutput       = GPIO_PORTB_SetOutput,
    .GetInput        = GPIO_PORTB_GetInput,
};
#endif

#if defined(GPIOC) && defined(RTE_GPIO_PORTC) && RTE_GPIO_PORTC

const gpio_cmsis_config_t s_gpio_portC_cmsis_config = {
    .gpio_base = GPIOC,
    .port_base = PORTC,
};

static gpio_cmsis_handle_t s_gpio_portC_handle = {
    .config = &s_gpio_portC_cmsis_config,
};

static int32_t GPIO_PORTC_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_portC_handle, pin, cb_event);
}

static int32_t GPIO_PORTC_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_portC_handle, pin, direction);
}

static int32_t GPIO_PORTC_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_portC_handle, pin, mode);
}

static int32_t GPIO_PORTC_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_portC_handle, pin, resistor);
}

static int32_t GPIO_PORTC_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIOC_IRQ
    EnableIRQ(RTE_GPIOC_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_portC_handle, pin, trigger);
}

static uint32_t GPIO_PORTC_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_portC_handle, pin);
}

static void GPIO_PORTC_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_portC_handle, pin, val);
}

#ifdef RTE_GPIOC_IRQ_HANDLER
void RTE_GPIOC_IRQ_HANDLER(void);
void RTE_GPIOC_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_portC_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORTC = {
    .Setup           = GPIO_PORTC_Setup,
    .SetDirection    = GPIO_PORTC_SetDirection,
    .SetOutputMode   = GPIO_PORTC_SetOutputMode,
    .SetPullResistor = GPIO_PORTC_SetPullResistor,
    .SetEventTrigger = GPIO_PORTC_SetEventTrigger,
    .SetOutput       = GPIO_PORTC_SetOutput,
    .GetInput        = GPIO_PORTC_GetInput,
};
#endif

#if defined(GPIOD) && defined(RTE_GPIO_PORTD) && RTE_GPIO_PORTD

const gpio_cmsis_config_t s_gpio_portD_cmsis_config = {
    .gpio_base = GPIOD,
    .port_base = PORTD,
};

static gpio_cmsis_handle_t s_gpio_portD_handle = {
    .config = &s_gpio_portD_cmsis_config,
};

static int32_t GPIO_PORTD_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_portD_handle, pin, cb_event);
}

static int32_t GPIO_PORTD_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_portD_handle, pin, direction);
}

static int32_t GPIO_PORTD_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_portD_handle, pin, mode);
}

static int32_t GPIO_PORTD_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_portD_handle, pin, resistor);
}

static int32_t GPIO_PORTD_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIOD_IRQ
    EnableIRQ(RTE_GPIOD_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_portD_handle, pin, trigger);
}

static uint32_t GPIO_PORTD_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_portD_handle, pin);
}

static void GPIO_PORTD_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_portD_handle, pin, val);
}

#ifdef RTE_GPIOD_IRQ_HANDLER
void RTE_GPIOD_IRQ_HANDLER(void);
void RTE_GPIOD_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_portD_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORTD = {
    .Setup           = GPIO_PORTD_Setup,
    .SetDirection    = GPIO_PORTD_SetDirection,
    .SetOutputMode   = GPIO_PORTD_SetOutputMode,
    .SetPullResistor = GPIO_PORTD_SetPullResistor,
    .SetEventTrigger = GPIO_PORTD_SetEventTrigger,
    .SetOutput       = GPIO_PORTD_SetOutput,
    .GetInput        = GPIO_PORTD_GetInput,
};
#endif

#if defined(GPIOE) && defined(RTE_GPIO_PORTE) && RTE_GPIO_PORTE

const gpio_cmsis_config_t s_gpio_portE_cmsis_config = {
    .gpio_base = GPIOE,
    .port_base = PORTE,
};

static gpio_cmsis_handle_t s_gpio_portE_handle = {
    .config = &s_gpio_portE_cmsis_config,
};

static int32_t GPIO_PORTE_Setup(ARM_GPIO_Pin_t pin, ARM_GPIO_SignalEvent_t cb_event)
{
    return CMSIS_GPIO_Setup(&s_gpio_portE_handle, pin, cb_event);
}

static int32_t GPIO_PORTE_SetDirection(ARM_GPIO_Pin_t pin, ARM_GPIO_DIRECTION direction)
{
    return CMSIS_GPIO_SetDirection(&s_gpio_portE_handle, pin, direction);
}

static int32_t GPIO_PORTE_SetOutputMode(ARM_GPIO_Pin_t pin, ARM_GPIO_OUTPUT_MODE mode)
{
    return CMSIS_GPIO_SetOutputMode(&s_gpio_portE_handle, pin, mode);
}

static int32_t GPIO_PORTE_SetPullResistor(ARM_GPIO_Pin_t pin, ARM_GPIO_PULL_RESISTOR resistor)
{
    return CMSIS_GPIO_SetPullResistor(&s_gpio_portE_handle, pin, resistor);
}

static int32_t GPIO_PORTE_SetEventTrigger(ARM_GPIO_Pin_t pin, ARM_GPIO_EVENT_TRIGGER trigger)
{
#ifdef RTE_GPIOE_IRQ
    EnableIRQ(RTE_GPIOE_IRQ);
#endif
    return CMSIS_GPIO_SetEventTrigger(&s_gpio_portE_handle, pin, trigger);
}

static uint32_t GPIO_PORTE_GetInput(ARM_GPIO_Pin_t pin)
{
    return CMSIS_GPIO_GetInput(&s_gpio_portE_handle, pin);
}

static void GPIO_PORTE_SetOutput(ARM_GPIO_Pin_t pin, uint32_t val)
{
    CMSIS_GPIO_SetOutput(&s_gpio_portE_handle, pin, val);
}

#ifdef RTE_GPIOE_IRQ_HANDLER
void RTE_GPIOE_IRQ_HANDLER(void);
void RTE_GPIOE_IRQ_HANDLER(void)
{
    s_gpioIsr(&s_gpio_portE_handle);
    SDK_ISR_EXIT_BARRIER;
}
#endif

ARM_DRIVER_GPIO Driver_GPIO_PORTE = {
    .Setup           = GPIO_PORTE_Setup,
    .SetDirection    = GPIO_PORTE_SetDirection,
    .SetOutputMode   = GPIO_PORTE_SetOutputMode,
    .SetPullResistor = GPIO_PORTE_SetPullResistor,
    .SetEventTrigger = GPIO_PORTE_SetEventTrigger,
    .SetOutput       = GPIO_PORTE_SetOutput,
    .GetInput        = GPIO_PORTE_GetInput,
};
#endif
