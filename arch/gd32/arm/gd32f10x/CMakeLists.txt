add_library(arch-gd32f10x OBJECT EXCLUDE_FROM_ALL)

target_compile_definitions(arch-gd32f10x PUBLIC -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32f10x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/CMSIS/GD/GD32F10x/Include")
target_include_directories(arch-gd32f10x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/CMSIS/")
target_include_directories(arch-gd32f10x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Include")
target_sources(arch-gd32f10x PRIVATE  "${CMAKE_CURRENT_LIST_DIR}/CMSIS/GD/GD32F10x/Source/system_gd32f10x.c")
target_sources(arch-gd32f10x PRIVATE  "${CMAKE_CURRENT_LIST_DIR}/CMSIS/GD/GD32F10x/Source/GCC/startup_gd32f10x_hd.S")
target_sources(arch-gd32f10x PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_pmu.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_sdio.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_fwdgt.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_timer.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_dbg.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_enet.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_rcu.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_exmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_can.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_wwdgt.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_fmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_misc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32F10x_standard_peripheral/Source/gd32f10x_bkp.c"
)
    