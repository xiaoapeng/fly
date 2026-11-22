include("${FLY_TOP_DIR}/cmake/fetch_git_project.cmake")

# Commits are from the official MCUXpresso SDK v25.06.00 manifest.
fly_fetch_git_project(vendor-mcxn947
    GIT_REPOSITORY "https://github.com/nxp-mcuxpresso/mcux-devices-mcx.git"
    GIT_TAG "cb21024505166d9b78dce33b71345db9f55d9643"
    SOURCE_DIR_VAR mcxn947_device_source
    SPARSE_PATHS "MCXN/MCXN947" "MCXN/periph"
)

fly_fetch_git_project(nxp-mcu-sdk-cmsis
    GIT_REPOSITORY "https://github.com/nxp-mcuxpresso/mcu-sdk-cmsis.git"
    GIT_TAG "a76ba3495fc5a68c71bc098ec177613b9b28cca5"
    SOURCE_DIR_VAR mcxn947_cmsis_source
    SPARSE_PATHS "Core/Include"
)

# The FreeRTOS wrappers remain available as headers for applications.
fly_fetch_git_project(nxp-mcux-freertos-drivers
    GIT_REPOSITORY "https://github.com/nxp-mcuxpresso/mcux-freertos-drivers.git"
    GIT_TAG "845ff93b52bbc1852c5b9911e74524f6baed2ba6"
    SOURCE_DIR_VAR mcxn947_freertos_driver_source
    SPARSE_PATHS "lpflexcomm"
)

# Select the exact IP variants used by this chip, even when NXP reuses filenames.
set(mcxn947_core_driver_sources
    "cache/cache64/fsl_cache.c"
    "cdog/fsl_cdog.c"
    "common/fsl_common.c"
    "common/fsl_common_arm.c"
    "crc/fsl_crc.c"
    "ctimer/fsl_ctimer.c"
    "dac14/fsl_dac14.c"
    "dac_1/fsl_dac.c"
    "edma4/fsl_edma.c"
    "eim/fsl_eim.c"
    "erm/fsl_erm.c"
    "evtg/fsl_evtg.c"
    "ewm/fsl_ewm.c"
    "flexcan/fsl_flexcan.c"
    "flexio/fsl_flexio.c"
    "flexio/i2c/fsl_flexio_i2c_master.c"
    "flexio/mculcd/fsl_flexio_mculcd.c"
    "flexio/mculcd/fsl_flexio_mculcd_edma.c"
    "flexio/spi/fsl_flexio_spi.c"
    "flexio/spi/fsl_flexio_spi_edma.c"
    "flexio/uart/fsl_flexio_uart.c"
    "flexio/uart/fsl_flexio_uart_edma.c"
    "flexspi/fsl_flexspi.c"
    "flexspi/fsl_flexspi_edma.c"
    "gdet/fsl_gdet.c"
    "gpio/fsl_gpio.c"
    "i3c/fsl_i3c.c"
    "inputmux/fsl_inputmux.c"
    "intm/fsl_intm.c"
    "irtc/fsl_irtc.c"
    "itrc/fsl_itrc.c"
    "lpadc/fsl_lpadc.c"
    "lpc_freqme/fsl_freqme.c"
    "lpcmp/fsl_lpcmp.c"
    "lpflexcomm/fsl_lpflexcomm.c"
    "lpflexcomm/lpi2c/fsl_lpi2c.c"
    "lpflexcomm/lpi2c/fsl_lpi2c_edma.c"
    "lpflexcomm/lpspi/fsl_lpspi.c"
    "lpflexcomm/lpspi/fsl_lpspi_edma.c"
    "lpflexcomm/lpuart/fsl_lpuart.c"
    "lpflexcomm/lpuart/fsl_lpuart_edma.c"
    "lptmr/fsl_lptmr.c"
    "mcx_cmc/fsl_cmc.c"
    "mcx_enet/fsl_enet.c"
    "mcx_spc/fsl_spc.c"
    "mcx_vbat/fsl_vbat.c"
    "mrt/fsl_mrt.c"
    "opamp/fsl_opamp.c"
    "ostimer/fsl_ostimer.c"
    "pdm/fsl_pdm.c"
    "pdm/fsl_pdm_edma.c"
    "pint/fsl_pint.c"
    "plu/fsl_plu.c"
    "powerquad/fsl_powerquad_basic.c"
    "powerquad/fsl_powerquad_data.c"
    "powerquad/fsl_powerquad_filter.c"
    "powerquad/fsl_powerquad_math.c"
    "powerquad/fsl_powerquad_matrix.c"
    "powerquad/fsl_powerquad_transform.c"
    "puf_v3/fsl_puf_v3.c"
    "pwm/fsl_pwm.c"
    "sai/fsl_sai.c"
    "sai/fsl_sai_edma.c"
    "sctimer/fsl_sctimer.c"
    "smartcard/fsl_smartcard_emvsim.c"
    "smartcard/fsl_smartcard_phy_emvsim.c"
    "syspm/fsl_syspm.c"
    "tdet/fsl_tdet.c"
    "tsi/tsi_v6/fsl_tsi_v6.c"
    "usdhc/fsl_usdhc.c"
    "utick/fsl_utick.c"
    "vref_1/fsl_vref.c"
    "wuu/fsl_wuu.c"
    "wwdt/fsl_wwdt.c"
)

# Retain the export's header-only drivers along with those built above.
set(mcxn947_core_driver_dirs
    "drivers/mailbox"
    "drivers/npx"
    "drivers/otp"
    "drivers/port"
    "drivers/qdc"
    "drivers/sema42"
    "drivers/sinc"
    "drivers/smartdma"
    "drivers/trdc_1"
)
foreach(source IN LISTS mcxn947_core_driver_sources)
    get_filename_component(driver_dir "drivers/${source}" DIRECTORY)
    list(APPEND mcxn947_core_driver_dirs "${driver_dir}")
endforeach()
list(REMOVE_DUPLICATES mcxn947_core_driver_dirs)
fly_fetch_git_project(nxp-mcuxsdk-core
    GIT_REPOSITORY "https://github.com/nxp-mcuxpresso/mcuxsdk-core.git"
    GIT_TAG "2893c6df8c59f5763476300b584907c4d59ff2e2"
    SOURCE_DIR_VAR mcxn947_core_source
    SPARSE_PATHS ${mcxn947_core_driver_dirs}
    PATCH_DIR "${CMAKE_CURRENT_LIST_DIR}/patches"
)

set(mcxn947_device_dir "${mcxn947_device_source}/MCXN/MCXN947")
list(TRANSFORM mcxn947_core_driver_dirs PREPEND "${mcxn947_core_source}/")
list(TRANSFORM mcxn947_core_driver_sources PREPEND "${mcxn947_core_source}/drivers/")

add_library(arch-mcxn947-core0 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-mcxn947-core0 PUBLIC
    __FPU_PRESENT
    __STARTUP_CLEAR_BSS
    CPU_MCXN947VDF_cm33_core0
)
target_include_directories(arch-mcxn947-core0 PUBLIC
    "${mcxn947_cmsis_source}/Core/Include"
    "${mcxn947_device_dir}"
    "${mcxn947_device_dir}/drivers"
    "${mcxn947_device_source}/MCXN/periph"
    "${mcxn947_device_dir}/drivers/romapi/nboot"
    "${mcxn947_device_dir}/drivers/romapi/mem_interface"
    "${mcxn947_device_dir}/drivers/romapi/flash"
    "${mcxn947_device_dir}/drivers/romapi/runbootloader"
    "${mcxn947_freertos_driver_source}/lpflexcomm/lpi2c"
    "${mcxn947_freertos_driver_source}/lpflexcomm/lpspi"
    "${mcxn947_freertos_driver_source}/lpflexcomm/lpuart"
    ${mcxn947_core_driver_dirs}
)
target_sources(arch-mcxn947-core0 PRIVATE
    "${mcxn947_device_dir}/gcc/startup_MCXN947_cm33_core0.S"
    "${mcxn947_device_dir}/system_MCXN947_cm33_core0.c"
    "${mcxn947_device_dir}/drivers/romapi/mem_interface/src/fsl_mem_interface.c"
    "${mcxn947_device_dir}/drivers/romapi/nboot/src/fsl_nboot.c"
    "${mcxn947_device_dir}/drivers/romapi/flash/src/fsl_flash.c"
    "${mcxn947_device_dir}/drivers/romapi/runbootloader/src/fsl_runbootloader.c"
    "${mcxn947_device_dir}/drivers/fsl_clock.c"
    "${mcxn947_device_dir}/drivers/fsl_reset.c"
    "${mcxn947_device_dir}/drivers/fsl_edma_soc.c"
    ${mcxn947_core_driver_sources}
)
