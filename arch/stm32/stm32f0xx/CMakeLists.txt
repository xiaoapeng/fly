set(STM32F0XX_INCLUDE_DIRS  
    "${CMAKE_CURRENT_LIST_DIR}/../CMSIS/Include"
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Include"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Inc"
)

set(STM32F0XX_LL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_utils.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_comp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_usb.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_crs.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_ll_usart.c"
)

set(STM32F0XX_HAL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_smartcard.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dac_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cortex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_comp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rtc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_iwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_wwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_smbus.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_usart_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tsc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_pcd_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2s.c"
    #"${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_timebase_rtc_wakeup_template.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_smartcard_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_cec.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_can.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_i2c_ex.c"
    #"${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_timebase_tim_template.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_crc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_irda.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_tim_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_uart.c"
    #"${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_timebase_rtc_alarm_template.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_spi_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_flash.c"
    #"${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_msp_template.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F0xx_HAL_Driver/Src/stm32f0xx_hal_adc_ex.c"
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


# STM32F030x6 系列
add_library(arch-stm32f030x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f030x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F030x6 )
target_include_directories(arch-stm32f030x6 PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f030x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f030x6.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F030x8 系列
add_library(arch-stm32f030x8 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f030x8 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F030x8 )
target_include_directories(arch-stm32f030x8 PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f030x8 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f030x8.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F031x6 系列
add_library(arch-stm32f031x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f031x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F031x6 )
target_include_directories(arch-stm32f031x6 PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f031x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f031x6.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F038xx 系列
add_library(arch-stm32f038xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f038xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F038xx )
target_include_directories(arch-stm32f038xx PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f038xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f038xx.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F042x6 系列
add_library(arch-stm32f042x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f042x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F042x6 )
target_include_directories(arch-stm32f042x6 PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f042x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f042x6.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F048xx 系列
add_library(arch-stm32f048xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f048xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F048xx )
target_include_directories(arch-stm32f048xx PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f048xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f048xx.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F051x8 系列
add_library(arch-stm32f051x8 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f051x8 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F051x8 )
target_include_directories(arch-stm32f051x8 PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f051x8 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f051x8.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F058xx 系列
add_library(arch-stm32f058xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f058xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F058xx )
target_include_directories(arch-stm32f058xx PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f058xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f058xx.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F070x6 系列
add_library(arch-stm32f070x6 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f070x6 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F070x6 )
target_include_directories(arch-stm32f070x6 PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f070x6 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f070x6.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F070xB 系列
add_library(arch-stm32f070xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f070xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F070xB )
target_include_directories(arch-stm32f070xb PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f070xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f070xb.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F071xB 系列
add_library(arch-stm32f071xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f071xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F071xB )
target_include_directories(arch-stm32f071xb PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f071xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f071xb.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F072xB 系列
add_library(arch-stm32f072xb OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f072xb PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F072xB )
target_include_directories(arch-stm32f072xb PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f072xb PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f072xb.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F078xx 系列
add_library(arch-stm32f078xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f078xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F078xx )
target_include_directories(arch-stm32f078xx PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f078xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f078xx.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F030xC 系列
add_library(arch-stm32f030xc OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f030xc PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F030xC )
target_include_directories(arch-stm32f030xc PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f030xc PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f030xc.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F091xC 系列
add_library(arch-stm32f091xc OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f091xc PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F091xC )
target_include_directories(arch-stm32f091xc PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f091xc PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f091xc.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)

# STM32F098xx 系列
add_library(arch-stm32f098xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f098xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F098xx )
target_include_directories(arch-stm32f098xx PUBLIC "${STM32F0XX_INCLUDE_DIRS}")
target_sources(arch-stm32f098xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F0xx/Source/Templates/gcc/startup_stm32f098xx.s"
    ${STM32F0XX_LL_SOURCES}
    ${STM32F0XX_HAL_SOURCES}
)
