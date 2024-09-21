set(GD32VF103_INCLUDE_DIRS  
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Include/"
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/drivers/"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/"
)

set(GD32VF103_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_bkp.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_can.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_dbg.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_eclic.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_exmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_fmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_fwdgt.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_pmu.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_rcu.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_timer.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/GD32VF103_standard_peripheral/Source/gd32vf103_wwdgt.c"
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/drivers/n200_func.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/init.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/handlers.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/your_printf.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/_exit.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/close.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/fstat.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/isatty.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/lseek.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/read.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/sbrk.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/write.c"
	"${CMAKE_CURRENT_LIST_DIR}/RISCV/stubs/write_hex.c"
)

add_library(arch-gd32vf103t OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103t PUBLIC -DGD32VF103T_START -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103t PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103t PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/start.S"
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)



add_library(arch-gd32vf103c OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103c PUBLIC -DGD32VF103C_START -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103c PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103c PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/start.S"
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)


add_library(arch-gd32vf103r OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103r PUBLIC -DGD32VF103R_START -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103r PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103r PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/start.S"
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)

add_library(arch-gd32vf103v OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-gd32vf103v PUBLIC -DGD32VF103V_EVAL -DUSE_STDPERIPH_DRIVER)
target_include_directories(arch-gd32vf103v PUBLIC "${GD32VF103_INCLUDE_DIRS}")
target_sources(arch-gd32vf103v PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/start.S"
    "${CMAKE_CURRENT_LIST_DIR}/RISCV/env_Eclipse/entry.S"
    "${GD32VF103_SOURCES}"
)