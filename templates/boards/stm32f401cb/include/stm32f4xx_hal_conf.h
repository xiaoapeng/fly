#ifndef STM32F4XX_HAL_CONF_H
#define STM32F4XX_HAL_CONF_H

/* The SDK's base HAL driver uses RCC, flash and Cortex support. */
#define HAL_MODULE_ENABLED
#define HAL_RCC_MODULE_ENABLED
#define HAL_FLASH_MODULE_ENABLED
#define HAL_CORTEX_MODULE_ENABLED
#define HAL_PWR_MODULE_ENABLED
#define HAL_GPIO_MODULE_ENABLED

#ifndef HSE_VALUE
#define HSE_VALUE 25000000U
#endif
#ifndef EXTERNAL_CLOCK_VALUE
#define EXTERNAL_CLOCK_VALUE 12288000U
#endif
#define HSI_VALUE 16000000U
#define LSI_VALUE 32000U
#define LSE_VALUE 32768U
#define HSE_STARTUP_TIMEOUT 100U
#define LSE_STARTUP_TIMEOUT 5000U
#define TICK_INT_PRIORITY 15U
#define PREFETCH_ENABLE 1U
#define INSTRUCTION_CACHE_ENABLE 1U
#define DATA_CACHE_ENABLE 1U
#define assert_param(expr) ((void)0U)

#include "stm32f4xx_hal_rcc.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_cortex.h"
#include "stm32f4xx_hal_pwr.h"
#include "stm32f4xx_hal_gpio.h"

#endif
