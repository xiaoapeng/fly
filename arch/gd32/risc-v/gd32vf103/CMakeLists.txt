include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# GigaDevice GD32VF103 Firmware Library V1.1.1.
fly_fetch_git_project(vendor-gd32vf103
    GIT_REPOSITORY "https://github.com/GigaDevice-Semiconductor/GD32VF103_Firmware_Library.git"
    GIT_TAG "7ab0521e96461b8833f2f4974cf6bdd1f80a7101"
    SOURCE_DIR_VAR gd32vf103_vendor_source
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
)
set(gd32vf103_vendor_dir "${gd32vf103_vendor_source}/Firmware")

set(GD32VF103_INCLUDE_DIRS
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Include/"
    "${gd32vf103_vendor_dir}/RISCV/drivers/"
	"${gd32vf103_vendor_dir}/RISCV/stubs/"
)

set(GD32VF103_SOURCES
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_adc.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_bkp.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_can.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_crc.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_dac.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_dbg.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_dma.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_eclic.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_exmc.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_exti.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_fmc.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_fwdgt.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_gpio.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_i2c.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_pmu.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_rcu.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_rtc.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_spi.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_timer.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_usart.c"
    "${gd32vf103_vendor_dir}/GD32VF103_standard_peripheral/Source/gd32vf103_wwdgt.c"
    "${gd32vf103_vendor_dir}/RISCV/drivers/n200_func.c"
	"${gd32vf103_vendor_dir}/RISCV/env_Eclipse/init.c"
	"${gd32vf103_vendor_dir}/RISCV/env_Eclipse/handlers.c"
	"${gd32vf103_vendor_dir}/RISCV/env_Eclipse/your_printf.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/_exit.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/close.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/fstat.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/isatty.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/lseek.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/read.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/sbrk.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/write.c"
	"${gd32vf103_vendor_dir}/RISCV/stubs/write_hex.c"
)

add_library(arch-gd32vf103t OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103t PUBLIC -DGD32VF103T_START -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103t PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103t PRIVATE
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/start.S"
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)



add_library(arch-gd32vf103c OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103c PUBLIC -DGD32VF103C_START -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103c PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103c PRIVATE
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/start.S"
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)


add_library(arch-gd32vf103r OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103r PUBLIC -DGD32VF103R_START -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103r PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103r PRIVATE
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/start.S"
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)

add_library(arch-gd32vf103v OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103v PUBLIC -DGD32VF103V_EVAL -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103v PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103v PRIVATE
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/start.S"
    "${gd32vf103_vendor_dir}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)
