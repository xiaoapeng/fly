set(STM32F1XX_INCLUDE_DIRS  
    "${CMAKE_CURRENT_LIST_DIR}/../CMSIS/Include"
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Include"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Inc"
)

set(STM32F1XX_LL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_fsmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_sdmmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_usb.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_ll_utils.c"
)

set(STM32F1XX_HAL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_adc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_can.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cec.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_cortex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dac_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_eth.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_flash_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_gpio_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_hcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_i2s.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_irda.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_iwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_mmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nand.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_nor.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pccard.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pcd_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rcc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_rtc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_smartcard.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_sram.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_tim_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_uart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F1xx_HAL_Driver/Src/stm32f1xx_hal_wwdg.c"
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

# STM32F100xB 系列
add_library(arch-stm32f100xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f100xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F100xB )
target_include_directories(arch-stm32f100xb PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f100xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f100xb.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F100xE 系列
add_library(arch-stm32f100xe OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f100xe PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F100xE )
target_include_directories(arch-stm32f100xe PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f100xe PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f100xe.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F101x6 系列
add_library(arch-stm32f101x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f101x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F101x6 )
target_include_directories(arch-stm32f101x6 PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f101x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f101x6.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F101xB 系列
add_library(arch-stm32f101xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f101xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F101xB )
target_include_directories(arch-stm32f101xb PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f101xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f101xb.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F101xE 系列
add_library(arch-stm32f101xe OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f101xe PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F101xE )
target_include_directories(arch-stm32f101xe PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f101xe PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f101xe.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F101xG 系列
add_library(arch-stm32f101xg OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f101xg PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F101xG )
target_include_directories(arch-stm32f101xg PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f101xg PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f101xg.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F102x6 系列
add_library(arch-stm32f102x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f102x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F102x6 )
target_include_directories(arch-stm32f102x6 PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f102x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f102x6.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F102xB 系列
add_library(arch-stm32f102xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f102xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F102xB )
target_include_directories(arch-stm32f102xb PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f102xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f102xb.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F103x6 系列
add_library(arch-stm32f103x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f103x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F103x6 )
target_include_directories(arch-stm32f103x6 PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f103x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103x6.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F103xB 系列
add_library(arch-stm32f103xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f103xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F103xB )
target_include_directories(arch-stm32f103xb PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f103xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xb.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F103xE 系列
add_library(arch-stm32f103xe OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f103xe PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F103xE )
target_include_directories(arch-stm32f103xe PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f103xe PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xe.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F103xG 系列
add_library(arch-stm32f103xg OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f103xg PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F103xG )
target_include_directories(arch-stm32f103xg PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f103xg PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f103xg.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F105xC 系列
add_library(arch-stm32f105xc OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f105xc PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F105xC )
target_include_directories(arch-stm32f105xc PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f105xc PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f105xc.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)

# STM32F107xC 系列
add_library(arch-stm32f107xc OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f107xc PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F107xC )
target_include_directories(arch-stm32f107xc PUBLIC "${STM32F1XX_INCLUDE_DIRS}")
target_sources(arch-stm32f107xc PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F1xx/Source/Templates/gcc/startup_stm32f107xc.s"
    ${STM32F1XX_LL_SOURCES}
    ${STM32F1XX_HAL_SOURCES}
)
