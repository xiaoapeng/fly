include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# WCH CH583 EVT v1.7 with its 2023-02-14 driver update.
fly_fetch_git_project(vendor-ch58x
    GIT_REPOSITORY "https://github.com/openwch/ch583.git"
    GIT_TAG "b9f232a6cc98592001b5ddc1fe00e4e584e271e8"
    SOURCE_DIR_VAR ch58x_vendor_source
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
    SPARSE_PATHS "EVT/EXAM/SRC"
)
set(ch58x_vendor_dir "${ch58x_vendor_source}/EVT/EXAM/SRC")

set(CH58X_INCLUDE_DIRS
    "${ch58x_vendor_dir}/StdPeriphDriver/inc/"
    "${ch58x_vendor_dir}/RVMSIS/"
    "${ch58x_vendor_dir}/StdPeriphDriver/"
)

set(CH58X_SOURCES
    "${ch58x_vendor_dir}/RVMSIS/core_riscv.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_timer0.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_usbhostClass.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_gpio.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_adc.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_uart1.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_spi1.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_timer2.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_usb2hostClass.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_timer3.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_uart2.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_clk.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_usb2dev.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_usbdev.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_timer1.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_usbhostBase.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_i2c.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_usb2hostBase.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_uart3.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_sys.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_pwr.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_spi0.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_uart0.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_pwm.c"
    "${ch58x_vendor_dir}/StdPeriphDriver/CH58x_flash.c"
)


add_library(arch-ch58x OBJECT EXCLUDE_FROM_ALL)

target_include_directories(arch-ch58x PUBLIC ${CH58X_INCLUDE_DIRS})

target_sources(arch-ch58x PRIVATE
    ${CH58X_SOURCES}
    "${ch58x_vendor_dir}/Startup/startup_CH583.S"
)
target_link_libraries(arch-ch58x PUBLIC "${ch58x_vendor_dir}/StdPeriphDriver/libISP583.a")
