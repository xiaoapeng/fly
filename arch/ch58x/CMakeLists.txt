

set(CH58X_INCLUDE_DIRS  
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/inc/"
    "${CMAKE_CURRENT_LIST_DIR}/RVMSIS/"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/"
)

set(CH58X_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/RVMSIS/core_riscv.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_timer0.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_usbhostClass.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_uart1.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_spi1.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_timer2.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_usb2hostClass.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_timer3.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_uart2.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_clk.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_usb2dev.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_usbdev.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_timer1.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_usbhostBase.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_usb2hostBase.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_uart3.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_sys.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_spi0.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_uart0.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_pwm.c"
    "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/CH58x_flash.c"
)


add_library(arch-ch58x OBJECT EXCLUDE_FROM_ALL)

target_include_directories(arch-ch58x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/inc/" )
target_include_directories(arch-ch58x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/RVMSIS/" )
target_include_directories(arch-ch58x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/" )

target_sources(arch-ch58x PRIVATE
    ${CH58X_SOURCES} 
    "${CMAKE_CURRENT_LIST_DIR}/Startup/startup_CH583.S"
)
# target_link_directories(arch-ch58x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/")
target_link_libraries(arch-ch58x PUBLIC "${CMAKE_CURRENT_LIST_DIR}/StdPeriphDriver/libISP583.a")