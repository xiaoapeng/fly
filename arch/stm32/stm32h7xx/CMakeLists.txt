set(STM32H7XX_INCLUDE_DIRS  
    "${CMAKE_CURRENT_LIST_DIR}/../CMSIS/Include"
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Include/"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Inc/"
)

set(STM32H7XX_LL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lptim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_delayblock.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma2d.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_mdma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_comp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_swpmi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crs.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_usb.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rng.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_hrtim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_utils.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_bdma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_fmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_opamp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_cordic.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_lpuart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_sdmmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_ll_spi.c"
)

set(STM32H7XX_HAL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_otfdec.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ramecc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spdifrx.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cordic.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_swpmi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cortex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_iwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dts.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fdcan.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mdios.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_wwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma2d.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hsem.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sd_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dcmi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cec.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_comp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_fmac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sram.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rcc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_jpeg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_cryp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pwr_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2s_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smartcard_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_flash.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_mmc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_smbus.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_eth.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hrtim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rng.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_lptim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nand.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_qspi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dfsdm.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dsi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_uart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_opamp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_irda.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_hash_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_nor.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_rtc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sdram.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_sai_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dma_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_usart_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ospi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_gfxmmu.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_pssi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32H7xx_HAL_Driver/Src/stm32h7xx_hal_ltdc_ex.c"
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


# STM32H742xx Library
add_library(arch-stm32h742xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h742xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H742xx )
target_include_directories(arch-stm32h742xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h742xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h742xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H743xx Library
add_library(arch-stm32h743xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h743xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H743xx )
target_include_directories(arch-stm32h743xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h743xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h743xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H753xx Library
add_library(arch-stm32h753xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h753xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H753xx )
target_include_directories(arch-stm32h753xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h753xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h753xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H750xx Library
add_library(arch-stm32h750xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h750xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H750xx )
target_include_directories(arch-stm32h750xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h750xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h750xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H747xx Library
add_library(arch-stm32h747xx-cm4 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h747xx-cm4 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H747xx -DCORE_CM4 )
target_include_directories(arch-stm32h747xx-cm4 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h747xx-cm4 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h747xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

add_library(arch-stm32h747xx-cm7 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h747xx-cm7 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H747xx -DCORE_CM7 )
target_include_directories(arch-stm32h747xx-cm7 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h747xx-cm7 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h747xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)


# STM32H747xG Library
add_library(arch-stm32h747xg-cm4 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h747xg-cm4 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H747xG -DCORE_CM4 )
target_include_directories(arch-stm32h747xg-cm4 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h747xg-cm4 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h747xg.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)


add_library(arch-stm32h747xg-cm7 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h747xg-cm7 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H747xG -DCORE_CM7 )
target_include_directories(arch-stm32h747xg-cm7 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h747xg-cm7 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h747xg.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)


# STM32H757xx Library
add_library(arch-stm32h757xx-cm4 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h757xx-cm4 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H757xx -DCORE_CM4 )
target_include_directories(arch-stm32h757xx-cm4 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h757xx-cm4 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h757xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

add_library(arch-stm32h757xx-cm7 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h757xx-cm7 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H757xx -DCORE_CM7 )
target_include_directories(arch-stm32h757xx-cm7 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h757xx-cm7 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h757xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H745xx Library
add_library(arch-stm32h745xx-cm4 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h745xx-cm4 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H745xx -DCORE_CM4 )
target_include_directories(arch-stm32h745xx-cm4 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h745xx-cm4 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h745xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

add_library(arch-stm32h745xx-cm7 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h745xx-cm7 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H745xx -DCORE_CM7 )
target_include_directories(arch-stm32h745xx-cm7 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h745xx-cm7 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h745xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H745xG Library
add_library(arch-stm32h745xg-cm4 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h745xg-cm4 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H745xG -DCORE_CM4 )
target_include_directories(arch-stm32h745xg-cm4 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h745xg-cm4 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h745xg.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

add_library(arch-stm32h745xg-cm7 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h745xg-cm7 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H745xG -DCORE_CM7 )
target_include_directories(arch-stm32h745xg-cm7 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h745xg-cm7 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h745xg.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H755xx Library
add_library(arch-stm32h755xx-cm4 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h755xx-cm4 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H755xx -DCORE_CM4 )
target_include_directories(arch-stm32h755xx-cm4 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h755xx-cm4 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h755xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

add_library(arch-stm32h755xx-cm7 OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h755xx-cm7 PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H755xx -DCORE_CM7 )
target_include_directories(arch-stm32h755xx-cm7 PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h755xx-cm7 PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h755xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H7B0xx Library
add_library(arch-stm32h7b0xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h7b0xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H7B0xx )
target_include_directories(arch-stm32h7b0xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h7b0xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7b0xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H7A3xx Library
add_library(arch-stm32h7a3xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h7a3xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H7A3xx )
target_include_directories(arch-stm32h7a3xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h7a3xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7a3xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H7A3xxQ Library
add_library(arch-stm32h7a3xxq OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h7a3xxq PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H7A3xxQ )
target_include_directories(arch-stm32h7a3xxq PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h7a3xxq PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7a3xxq.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H7B3xx Library
add_library(arch-stm32h7b3xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h7b3xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H7B3xx )
target_include_directories(arch-stm32h7b3xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h7b3xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7b3xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H7B3xxQ Library
add_library(arch-stm32h7b3xxq OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h7b3xxq PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H7B3xxQ )
target_include_directories(arch-stm32h7b3xxq PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h7b3xxq PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h7b3xxq.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H735xx Library
add_library(arch-stm32h735xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h735xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H735xx )
target_include_directories(arch-stm32h735xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h735xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h735xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H733xx Library
add_library(arch-stm32h733xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h733xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H733xx )
target_include_directories(arch-stm32h733xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h733xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h733xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H730xx Library
add_library(arch-stm32h730xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h730xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H730xx )
target_include_directories(arch-stm32h730xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h730xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h730xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H730xxQ Library
add_library(arch-stm32h730xxq OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h730xxq PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H730xxQ )
target_include_directories(arch-stm32h730xxq PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h730xxq PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h730xxq.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H725xx Library
add_library(arch-stm32h725xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h725xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H725xx )
target_include_directories(arch-stm32h725xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h725xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h725xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)

# STM32H723xx Library
add_library(arch-stm32h723xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32h723xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32H723xx )
target_include_directories(arch-stm32h723xx PUBLIC "${STM32H7XX_INCLUDE_DIRS}")
target_sources(arch-stm32h723xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32H7xx/Source/Templates/gcc/startup_stm32h723xx.s"
    ${STM32H7XX_LL_SOURCES}
    ${STM32H7XX_HAL_SOURCES}
)
