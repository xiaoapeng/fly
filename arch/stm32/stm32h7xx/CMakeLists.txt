include("${CMAKE_CURRENT_LIST_DIR}/../stm32-device.cmake")

_fly_fetch_stm32_sources(h7
    faccfec37f82f7a1319c21638111b0f7335de7fe
    fec141ce999da655a48e1a15db83a72d564a1312
)

set(STM32_DEVICE_TEMPLATES "${STM32_DEVICE_DIR}/Source/Templates")
set(STM32_DEVICE_HEADER stm32h7xx.h)
set(STM32_SYSTEM_SOURCE system_stm32h7xx.c)

set(STM32_INCLUDE_DIRS
    "${STM32_CMSIS_INCLUDE}"
    "${STM32_DEVICE_DIR}/Include/"
    "${STM32_HAL_DIR}/Inc/"
)

set(STM32_LL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_lptim.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_usart.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_delayblock.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_dma2d.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_mdma.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_adc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_comp.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_fmac.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_swpmi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_crs.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_exti.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_usb.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_rng.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_hrtim.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_utils.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_bdma.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_fmc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_opamp.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_crc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_tim.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_dma.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_cordic.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_lpuart.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_dac.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_sdmmc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_ll_spi.c"
)

set(STM32_HAL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_otfdec.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dac_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_ramecc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_mdma.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_spdifrx.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_pcd_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dma.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_cordic.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_swpmi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_sd.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_hcd.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_smbus_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_cortex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_flash_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_mmc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_hash.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_iwdg.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dts.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_fdcan.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_tim_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_mdios.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_wwdg.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dma2d.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_cryp_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_hsem.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_spi_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_sd_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dcmi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_cec.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_tim.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_comp.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dfsdm_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_fmac.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_i2s.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_smartcard.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_rng_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_exti.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_crc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_sram.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_eth_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_rcc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_jpeg.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_uart_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_cryp.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_ltdc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_pwr_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_i2s_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_sai.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_smartcard_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_spi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_flash.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_opamp_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_mmc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_smbus.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_eth.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_hrtim.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_rng.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_lptim.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_nand.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_qspi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dfsdm.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dsi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_uart.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_crc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_opamp.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_irda.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_i2c_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_hash_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_nor.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_rtc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_sdram.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_usart.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_sai_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_pcd.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dma_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_adc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_adc.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_usart_ex.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_ospi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_gfxmmu.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_dac.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_pssi.c"
    "${STM32_HAL_DIR}/Src/stm32h7xx_hal_ltdc_ex.c"
)



# arch-stm32h742xx */   /*!< STM32H742VI, STM32H742ZI, STM32H742AI, STM32H742II, STM32H742BI, STM32H742XI Devices */
# arch-stm32h743xx */   /*!< STM32H743VI, STM32H743ZI, STM32H743AI, STM32H743II, STM32H743BI, STM32H743XI Devices */
# arch-stm32h753xx */   /*!< STM32H753VI, STM32H753ZI, STM32H753AI, STM32H753II, STM32H753BI, STM32H753XI Devices */
# arch-stm32h750xx */   /*!< STM32H750V, STM32H750I, STM32H750X Devices */
# arch-stm32h747xx */   /*!< STM32H747ZI, STM32H747AI, STM32H747II, STM32H747BI, STM32H747XI Devices */
# arch-stm32h747xg */   /*!< STM32H747AG, STM32H747IG, STM32H747BG, STM32H747XG */
# arch-stm32h757xx */   /*!< STM32H757ZI, STM32H757AI, STM32H757II, STM32H757BI, STM32H757XI Devices */
# arch-stm32h745xx */   /*!< STM32H745ZI, STM32H745II, STM32H745BI, STM32H745XI Devices  */
# arch-stm32h745xg */   /*!< STM32H745ZG, STM32H745IG, STM32H745BG, STM32H745XG Devices  */
# arch-stm32h755xx */   /*!< STM32H755ZI, STM32H755II, STM32H755BI, STM32H755XI Devices  */
# arch-stm32h7b0xx */   /*!< STM32H7B0ABIxQ, STM32H7B0IBTx, STM32H7B0RBTx, STM32H7B0VBTx, STM32H7B0ZBTx, STM32H7B0IBKxQ */
# arch-stm32h7a3xx */   /*!< STM32H7A3IIK6, STM32H7A3IIT6, STM32H7A3NIH6, STM32H7A3RIT6, STM32H7A3VIH6, STM32H7A3VIT6, STM32H7A3ZIT6 */
# arch-stm32h7a3xxq */  /*!< STM32H7A3QIY6Q, STM32H7A3IIK6Q, STM32H7A3IIT6Q, STM32H7A3LIH6Q, STM32H7A3VIH6Q, STM32H7A3VIT6Q, STM32H7A3AII6Q, STM32H7A3ZIT6Q */
# arch-stm32h7b3xx */   /*!< STM32H7B3IIK6, STM32H7B3IIT6, STM32H7B3NIH6, STM32H7B3RIT6, STM32H7B3VIH6, STM32H7B3VIT6, STM32H7B3ZIT6 */
# arch-stm32h7b3xxq */  /*!< STM32H7B3QIY6Q, STM32H7B3IIK6Q, STM32H7B3IIT6Q, STM32H7B3LIH6Q, STM32H7B3VIH6Q, STM32H7B3VIT6Q, STM32H7B3AII6Q, STM32H7B3ZIT6Q */
# arch-stm32h735xx */   /*!< STM32H735AGI6, STM32H735IGK6, STM32H735RGV6, STM32H735VGT6, STM32H735VGY6, STM32H735ZGT6 Devices */
# arch-stm32h733xx */   /*!< STM32H733VGH6, STM32H733VGT6, STM32H733ZGI6, STM32H733ZGT6, Devices */
# arch-stm32h730xx */   /*!< STM32H730VBH6, STM32H730VBT6, STM32H730ZBT6, STM32H730ZBI6 Devices */
# arch-stm32h730xxq */  /*!< STM32H730IBT6Q, STM32H730ABI6Q, STM32H730IBK6Q Devices */
# arch-stm32h725xx */   /*!< STM32H725AGI6, STM32H725IGK6, STM32H725IGT6, STM32H725RGV6, STM32H725VGT6, STM32H725VGY6, STM32H725ZGT6, STM32H725REV6, SM32H725VET6, STM32H725ZET6, STM32H725AEI6, STM32H725IET6, STM32H725IEK6  Devices */
# arch-stm32h723xx */   /*!< STM32H723VGH6, STM32H723VGT6, STM32H723ZGI6, STM32H723ZGT6, STM32H723VET6, STM32H723VEH6, STM32H723ZET6, STM32H723ZEI6 Devices */

if(CONFIG_ARCH_PROFILE STREQUAL "cm7")
    _fly_add_stm32_device(arch-stm32h742xx STM32H742xx startup_stm32h742xx.s)
    _fly_add_stm32_device(arch-stm32h743xx STM32H743xx startup_stm32h743xx.s)
    _fly_add_stm32_device(arch-stm32h753xx STM32H753xx startup_stm32h753xx.s)
    _fly_add_stm32_device(arch-stm32h750xx STM32H750xx startup_stm32h750xx.s)
    _fly_add_stm32_device(arch-stm32h747xx-cm7 STM32H747xx startup_stm32h747xx.s CORE_CM7)
    _fly_add_stm32_device(arch-stm32h747xg-cm7 STM32H747xG startup_stm32h747xg.s CORE_CM7)
    _fly_add_stm32_device(arch-stm32h757xx-cm7 STM32H757xx startup_stm32h757xx.s CORE_CM7)
    _fly_add_stm32_device(arch-stm32h745xx-cm7 STM32H745xx startup_stm32h745xx.s CORE_CM7)
    _fly_add_stm32_device(arch-stm32h745xg-cm7 STM32H745xG startup_stm32h745xg.s CORE_CM7)
    _fly_add_stm32_device(arch-stm32h755xx-cm7 STM32H755xx startup_stm32h755xx.s CORE_CM7)
    _fly_add_stm32_device(arch-stm32h7b0xx STM32H7B0xx startup_stm32h7b0xx.s)
    _fly_add_stm32_device(arch-stm32h7b0xxq STM32H7B0xxQ startup_stm32h7b0xxq.s)
    _fly_add_stm32_device(arch-stm32h7a3xx STM32H7A3xx startup_stm32h7a3xx.s)
    _fly_add_stm32_device(arch-stm32h7a3xxq STM32H7A3xxQ startup_stm32h7a3xxq.s)
    _fly_add_stm32_device(arch-stm32h7b3xx STM32H7B3xx startup_stm32h7b3xx.s)
    _fly_add_stm32_device(arch-stm32h7b3xxq STM32H7B3xxQ startup_stm32h7b3xxq.s)
    _fly_add_stm32_device(arch-stm32h735xx STM32H735xx startup_stm32h735xx.s)
    _fly_add_stm32_device(arch-stm32h733xx STM32H733xx startup_stm32h733xx.s)
    _fly_add_stm32_device(arch-stm32h730xx STM32H730xx startup_stm32h730xx.s)
    _fly_add_stm32_device(arch-stm32h730xxq STM32H730xxQ startup_stm32h730xxq.s)
    _fly_add_stm32_device(arch-stm32h725xx STM32H725xx startup_stm32h725xx.s)
    _fly_add_stm32_device(arch-stm32h723xx STM32H723xx startup_stm32h723xx.s)
elseif(CONFIG_ARCH_PROFILE STREQUAL "cm4")
    # The shared vendor startup selects M7 in assembly, overriding GCC's -mcpu.
    function(_fly_add_stm32h7_cm4_device target device_macro startup)
        set(vendor_source "${STM32_DEVICE_TEMPLATES}/gcc/${startup}")
        set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${vendor_source}")
        file(READ "${vendor_source}" content)
        string(REGEX MATCHALL "(^|\n)[ \t]*\\.cpu[ \t]+[^\r\n]+" cpu_directives "${content}")
        list(LENGTH cpu_directives directive_count)
        if(NOT directive_count EQUAL 1 OR NOT cpu_directives MATCHES "(^|\n)[ \t]*\\.cpu[ \t]+cortex-m7[ \t]*$")
            message(FATAL_ERROR "Expected one Cortex-M7 .cpu directive in ${vendor_source}")
        endif()
        string(REGEX REPLACE "(^|\n)([ \t]*\\.cpu[ \t]+)cortex-m7([ \t]*)(\r?\n|$)"
            "\\1\\2cortex-m4\\3\\4" content "${content}")
        set(cm4_source "${CMAKE_CURRENT_BINARY_DIR}/cm4/${startup}")
        file(GENERATE OUTPUT "${cm4_source}" CONTENT "${content}")
        set_source_files_properties("${cm4_source}" PROPERTIES GENERATED TRUE)
        _fly_add_stm32_device(${target} ${device_macro} "${cm4_source}" CORE_CM4)
    endfunction()

    _fly_add_stm32h7_cm4_device(arch-stm32h747xx-cm4 STM32H747xx startup_stm32h747xx.s)
    _fly_add_stm32h7_cm4_device(arch-stm32h747xg-cm4 STM32H747xG startup_stm32h747xg.s)
    _fly_add_stm32h7_cm4_device(arch-stm32h757xx-cm4 STM32H757xx startup_stm32h757xx.s)
    _fly_add_stm32h7_cm4_device(arch-stm32h745xx-cm4 STM32H745xx startup_stm32h745xx.s)
    _fly_add_stm32h7_cm4_device(arch-stm32h745xg-cm4 STM32H745xG startup_stm32h745xg.s)
    _fly_add_stm32h7_cm4_device(arch-stm32h755xx-cm4 STM32H755xx startup_stm32h755xx.s)
endif()
