include("${CMAKE_CURRENT_LIST_DIR}/../stm32-device.cmake")

_fly_fetch_stm32_sources(f1
    49daecceeb4b251999a784995e8b8d714b59dbc2
    7bda31007fdc1a3e45077cf151d137b4e8e38651
)

set(STM32_DEVICE_TEMPLATES "${STM32_DEVICE_DIR}/Source/Templates")
set(STM32_DEVICE_HEADER stm32f1xx.h)
set(STM32_SYSTEM_SOURCE system_stm32f1xx.c)

set(STM32_INCLUDE_DIRS
    "${STM32_CMSIS_INCLUDE}"
    "${STM32_DEVICE_DIR}/Include"
    "${STM32_HAL_DIR}/Inc"
)

set(STM32_LL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_adc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_crc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_dac.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_dma.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_exti.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_fsmc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_sdmmc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_spi.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_tim.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_usart.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_usb.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_ll_utils.c"
)

set(STM32_HAL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_adc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_adc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_can.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_cec.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_cortex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_crc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_dac.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_dac_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_dma.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_eth.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_exti.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_flash.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_flash_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_gpio_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_hcd.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_i2s.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_irda.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_iwdg.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_mmc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_nand.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_nor.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_pccard.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_pcd.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_pcd_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_rcc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_rtc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_sd.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_smartcard.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_spi.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_sram.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_tim.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_tim_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_uart.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_usart.c"
    "${STM32_HAL_DIR}/Src/stm32f1xx_hal_wwdg.c"
)

# arch-stm32f100xb  /*!< STM32F100C4, STM32F100R4, STM32F100C6, STM32F100R6, STM32F100C8, STM32F100R8, STM32F100V8, STM32F100CB, STM32F100RB and STM32F100VB */
# arch-stm32f100xe  /*!< STM32F100RC, STM32F100VC, STM32F100ZC, STM32F100RD, STM32F100VD, STM32F100ZD, STM32F100RE, STM32F100VE and STM32F100ZE */
# arch-stm32f101x6  /*!< STM32F101C4, STM32F101R4, STM32F101T4, STM32F101C6, STM32F101R6 and STM32F101T6 Devices */
# arch-stm32f101xb  /*!< STM32F101C8, STM32F101R8, STM32F101T8, STM32F101V8, STM32F101CB, STM32F101RB, STM32F101TB and STM32F101VB */
# arch-stm32f101xe  /*!< STM32F101RC, STM32F101VC, STM32F101ZC, STM32F101RD, STM32F101VD, STM32F101ZD, STM32F101RE, STM32F101VE and STM32F101ZE */ 
# arch-stm32f101xg  /*!< STM32F101RF, STM32F101VF, STM32F101ZF, STM32F101RG, STM32F101VG and STM32F101ZG */
# arch-stm32f102x6  /*!< STM32F102C4, STM32F102R4, STM32F102C6 and STM32F102R6 */
# arch-stm32f102xb  /*!< STM32F102C8, STM32F102R8, STM32F102CB and STM32F102RB */
# arch-stm32f103x6  /*!< STM32F103C4, STM32F103R4, STM32F103T4, STM32F103C6, STM32F103R6 and STM32F103T6 */
# arch-stm32f103xb  /*!< STM32F103C8, STM32F103R8, STM32F103T8, STM32F103V8, STM32F103CB, STM32F103RB, STM32F103TB and STM32F103VB */
# arch-stm32f103xe  /*!< STM32F103RC, STM32F103VC, STM32F103ZC, STM32F103RD, STM32F103VD, STM32F103ZD, STM32F103RE, STM32F103VE and STM32F103ZE */
# arch-stm32f103xg  /*!< STM32F103RF, STM32F103VF, STM32F103ZF, STM32F103RG, STM32F103VG and STM32F103ZG */
# arch-stm32f105xc  /*!< STM32F105R8, STM32F105V8, STM32F105RB, STM32F105VB, STM32F105RC and STM32F105VC */
# arch-stm32f107xc  /*!< STM32F107RB, STM32F107VB, STM32F107RC and STM32F107VC */

_fly_add_stm32_device(arch-stm32f100xb STM32F100xB startup_stm32f100xb.s)
_fly_add_stm32_device(arch-stm32f100xe STM32F100xE startup_stm32f100xe.s)
_fly_add_stm32_device(arch-stm32f101x6 STM32F101x6 startup_stm32f101x6.s)
_fly_add_stm32_device(arch-stm32f101xb STM32F101xB startup_stm32f101xb.s)
_fly_add_stm32_device(arch-stm32f101xe STM32F101xE startup_stm32f101xe.s)
_fly_add_stm32_device(arch-stm32f101xg STM32F101xG startup_stm32f101xg.s)
_fly_add_stm32_device(arch-stm32f102x6 STM32F102x6 startup_stm32f102x6.s)
_fly_add_stm32_device(arch-stm32f102xb STM32F102xB startup_stm32f102xb.s)
_fly_add_stm32_device(arch-stm32f103x6 STM32F103x6 startup_stm32f103x6.s)
_fly_add_stm32_device(arch-stm32f103xb STM32F103xB startup_stm32f103xb.s)
_fly_add_stm32_device(arch-stm32f103xe STM32F103xE startup_stm32f103xe.s)
_fly_add_stm32_device(arch-stm32f103xg STM32F103xG startup_stm32f103xg.s)
_fly_add_stm32_device(arch-stm32f105xc STM32F105xC startup_stm32f105xc.s)
_fly_add_stm32_device(arch-stm32f107xc STM32F107xC startup_stm32f107xc.s)
