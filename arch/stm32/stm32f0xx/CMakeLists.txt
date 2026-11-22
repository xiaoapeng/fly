include("${CMAKE_CURRENT_LIST_DIR}/../stm32-device.cmake")

_fly_fetch_stm32_sources(f0
    3973d99a89ac5fdf1f2593abc29a73d2981ce94a
    115eb1dc87e26ea29f4f2ca58003650381aabba2
)

set(STM32_DEVICE_TEMPLATES "${STM32_DEVICE_DIR}/Source/Templates")
set(STM32_DEVICE_HEADER stm32f0xx.h)
set(STM32_SYSTEM_SOURCE system_stm32f0xx.c)

set(STM32_INCLUDE_DIRS
    "${STM32_CMSIS_INCLUDE}"
    "${STM32_DEVICE_DIR}/Include"
    "${STM32_HAL_DIR}/Inc"
)

set(STM32_LL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_crc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_utils.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_comp.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_adc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_usb.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_spi.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_exti.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_dac.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_crs.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_tim.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_dma.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_ll_usart.c"
)

set(STM32_HAL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_rcc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_smartcard.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_usart.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_dma.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_dac_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_cortex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_comp.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_rtc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_iwdg.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_spi.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_wwdg.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_smbus.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_usart_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_pwr_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_tsc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_pcd.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_tim.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_dac.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_uart_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_pcd_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_i2s.c"
    #"${STM32_HAL_DIR}/Src/stm32f0xx_hal_timebase_rtc_wakeup_template.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_smartcard_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_cec.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_can.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_i2c_ex.c"
    #"${STM32_HAL_DIR}/Src/stm32f0xx_hal_timebase_tim_template.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_adc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_crc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_irda.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_tim_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_flash_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_uart.c"
    #"${STM32_HAL_DIR}/Src/stm32f0xx_hal_timebase_rtc_alarm_template.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_spi_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_flash.c"
    #"${STM32_HAL_DIR}/Src/stm32f0xx_hal_msp_template.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_exti.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_crc.c"
    "${STM32_HAL_DIR}/Src/stm32f0xx_hal_adc_ex.c"
)

#  arch-stm32f030x6  /*!< STM32F030x4, STM32F030x6 Devices (STM32F030xx microcontrollers where the Flash memory ranges between 16 and 32 Kbytes) */
#  arch-stm32f030x8  /*!< STM32F030x8 Devices (STM32F030xx microcontrollers where the Flash memory is 64 Kbytes) */
#  arch-stm32f031x6  /*!< STM32F031x4, STM32F031x6 Devices (STM32F031xx microcontrollers where the Flash memory ranges between 16 and 32 Kbytes) */
#  arch-stm32f038xx  /*!< STM32F038xx Devices (STM32F038xx microcontrollers where the Flash memory is 32 Kbytes) */
#  arch-stm32f042x6  /*!< STM32F042x4, STM32F042x6 Devices (STM32F042xx microcontrollers where the Flash memory ranges between 16 and 32 Kbytes) */
#  arch-stm32f048xx  /*!< STM32F048xx Devices (STM32F048xx microcontrollers where the Flash memory is 32 Kbytes) */
#  arch-stm32f051x8  /*!< STM32F051x4, STM32F051x6, STM32F051x8 Devices (STM32F051xx microcontrollers where the Flash memory ranges between 16 and 64 Kbytes) */
#  arch-stm32f058xx  /*!< STM32F058xx Devices (STM32F058xx microcontrollers where the Flash memory is 64 Kbytes) */
#  arch-stm32f070x6  /*!< STM32F070x6 Devices (STM32F070x6 microcontrollers where the Flash memory ranges between 16 and 32 Kbytes) */
#  arch-stm32f070xB  /*!< STM32F070xB Devices (STM32F070xB microcontrollers where the Flash memory ranges between 64 and 128 Kbytes) */
#  arch-stm32f071xB  /*!< STM32F071x8, STM32F071xB Devices (STM32F071xx microcontrollers where the Flash memory ranges between 64 and 128 Kbytes) */
#  arch-stm32f072xB  /*!< STM32F072x8, STM32F072xB Devices (STM32F072xx microcontrollers where the Flash memory ranges between 64 and 128 Kbytes) */
#  arch-stm32f078xx  /*!< STM32F078xx Devices (STM32F078xx microcontrollers where the Flash memory is 128 Kbytes) */
#  arch-stm32f030xC  /*!< STM32F030xC Devices (STM32F030xC microcontrollers where the Flash memory is 256 Kbytes) */
#  arch-stm32f091xC  /*!< STM32F091xB, STM32F091xC Devices (STM32F091xx microcontrollers where the Flash memory ranges between 128 and 256 Kbytes) */
#  arch-stm32f098xx  /*!< STM32F098xx Devices (STM32F098xx microcontrollers where the Flash memory is 256 Kbytes) */

_fly_add_stm32_device(arch-stm32f030x6 STM32F030x6 startup_stm32f030x6.s)
_fly_add_stm32_device(arch-stm32f030x8 STM32F030x8 startup_stm32f030x8.s)
_fly_add_stm32_device(arch-stm32f031x6 STM32F031x6 startup_stm32f031x6.s)
_fly_add_stm32_device(arch-stm32f038xx STM32F038xx startup_stm32f038xx.s)
_fly_add_stm32_device(arch-stm32f042x6 STM32F042x6 startup_stm32f042x6.s)
_fly_add_stm32_device(arch-stm32f048xx STM32F048xx startup_stm32f048xx.s)
_fly_add_stm32_device(arch-stm32f051x8 STM32F051x8 startup_stm32f051x8.s)
_fly_add_stm32_device(arch-stm32f058xx STM32F058xx startup_stm32f058xx.s)
_fly_add_stm32_device(arch-stm32f070x6 STM32F070x6 startup_stm32f070x6.s)
_fly_add_stm32_device(arch-stm32f070xb STM32F070xB startup_stm32f070xb.s)
_fly_add_stm32_device(arch-stm32f071xb STM32F071xB startup_stm32f071xb.s)
_fly_add_stm32_device(arch-stm32f072xb STM32F072xB startup_stm32f072xb.s)
_fly_add_stm32_device(arch-stm32f078xx STM32F078xx startup_stm32f078xx.s)
_fly_add_stm32_device(arch-stm32f030xc STM32F030xC startup_stm32f030xc.s)
_fly_add_stm32_device(arch-stm32f091xc STM32F091xC startup_stm32f091xc.s)
_fly_add_stm32_device(arch-stm32f098xx STM32F098xx startup_stm32f098xx.s)
