add_library(arch-gd32e23x OBJECT EXCLUDE_FROM_ALL)

# target_compile_definitions(arch-gd32e23x PUBLIC -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32e23x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/CMSIS/GD/GD32E23x/Include")
target_include_directories(arch-gd32e23x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/CMSIS/")
target_include_directories(arch-gd32e23x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Include")
target_sources(arch-gd32e23x PRIVATE  "${CMAKE_CURRENT_LIST_DIR}/CMSIS/GD/GD32E23x/Source/system_gd32e23x.c")
target_sources(arch-gd32e23x PRIVATE  "${CMAKE_CURRENT_LIST_DIR}/CMSIS/GD/GD32E23x/Source/GCC/startup_gd32e23x.s")
target_sources(arch-gd32e23x PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_fwdgt.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_pmu.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_misc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_rcu.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_wwdgt.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_timer.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_cmp.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_fmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_syscfg.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_dbg.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32E23x_standard_peripheral/Source/gd32e23x_crc.c"
)
    