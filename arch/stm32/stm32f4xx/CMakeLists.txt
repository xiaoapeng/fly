set(STM32F4XX_INCLUDE_DIRS  
    "${CMAKE_CURRENT_LIST_DIR}/../CMSIS/Include"
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Include"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Inc"
)

set(STM32F4XX_LL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usb.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_lptim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_utils.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_sdmmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rng.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma2d.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmpi2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fsmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_fmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_ll_tim.c"
)

set(STM32F4XX_HAL_SOURCES
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pccard.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nor.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_mmc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_eth.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_lptim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_irda.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spdifrx.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma2d.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dfsdm.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cec.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpsmbus_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpsmbus.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dma.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pcd_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_smartcard.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_iwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_spi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rtc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_qspi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sram.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hash_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rng.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_nand.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sdram.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_crc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_gpio.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_tim.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cortex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_fmpi2c_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dsi.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_adc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash_ramfunc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_exti.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_smbus.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_usart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_uart.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_can.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2c.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_rcc_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_cryp_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_i2s_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_pwr.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai_ex.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dac.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_ltdc.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_flash.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_wwdg.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_hcd.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_sai.c"
    "${CMAKE_CURRENT_LIST_DIR}/STM32F4xx_HAL_Driver/Src/stm32f4xx_hal_dcmi_ex.c"
)


# arch-stm32f405xx */   /*!< STM32F405RG, STM32F405VG and STM32F405ZG Devices */
# arch-stm32f415xx */   /*!< STM32F415RG, STM32F415VG and STM32F415ZG Devices */
# arch-stm32f407xx */   /*!< STM32F407VG, STM32F407VE, STM32F407ZG, STM32F407ZE, STM32F407IG and STM32F407IE Devices */
# arch-stm32f417xx */   /*!< STM32F417VG, STM32F417VE, STM32F417ZG, STM32F417ZE, STM32F417IG and STM32F417IE Devices */
# arch-stm32f427xx */   /*!< STM32F427VG, STM32F427VI, STM32F427ZG, STM32F427ZI, STM32F427IG and STM32F427II Devices */
# arch-stm32f437xx */   /*!< STM32F437VG, STM32F437VI, STM32F437ZG, STM32F437ZI, STM32F437IG and STM32F437II Devices */
# arch-stm32f429xx */   /*!< STM32F429VG, STM32F429VI, STM32F429ZG, STM32F429ZI, STM32F429BG, STM32F429BI, STM32F429NG, STM32F439NI, STM32F429IG and STM32F429II Devices */
# arch-stm32f439xx */   /*!< STM32F439VG, STM32F439VI, STM32F439ZG, STM32F439ZI, STM32F439BG, STM32F439BI, STM32F439NG, STM32F439NI, STM32F439IG and STM32F439II Devices */
# arch-stm32f401xc */   /*!< STM32F401CB, STM32F401CC, STM32F401RB, STM32F401RC, STM32F401VB and STM32F401VC Devices */
# arch-stm32f401xe */   /*!< STM32F401CD, STM32F401RD, STM32F401VD, STM32F401CE, STM32F401RE and STM32F401VE Devices */
# arch-stm32f410tx */   /*!< STM32F410T8 and STM32F410TB Devices */
# arch-stm32f410cx */   /*!< STM32F410C8 and STM32F410CB Devices */
# arch-stm32f410rx */   /*!< STM32F410R8 and STM32F410RB Devices */
# arch-stm32f411xe */   /*!< STM32F411CC, STM32F411RC, STM32F411VC, STM32F411CE, STM32F411RE and STM32F411VE Devices */
# arch-stm32f446xx */   /*!< STM32F446MC, STM32F446ME, STM32F446RC, STM32F446RE, STM32F446VC, STM32F446VE, STM32F446ZC and STM32F446ZE Devices */
# arch-stm32f469xx */   /*!< STM32F469AI, STM32F469II, STM32F469BI, STM32F469NI, STM32F469AG, STM32F469IG, STM32F469BG and STM32F469NG, STM32F469AE, STM32F469IE, STM32F469BE and STM32F469NE Devices */
# arch-stm32f479xx */   /*!< STM32F479AI, STM32F479II, STM32F479BI, STM32F479NI, STM32F479AG, STM32F479IG, STM32F479BG and STM32F479NG Devices */
# arch-stm32f412cx */   /*!< STM32F412CEU and STM32F412CGU Devices */
# arch-stm32f412zx */   /*!< STM32F412ZET, STM32F412ZGT, STM32F412ZEJ and STM32F412ZGJ Devices */
# arch-stm32f412vx */   /*!< STM32F412VET, STM32F412VGT, STM32F412VEH and STM32F412VGH Devices */
# arch-stm32f412rx */   /*!< STM32F412RET, STM32F412RGT, STM32F412REY and STM32F412RGY Devices */
# arch-stm32f413xx */   /*!< STM32F413CH, STM32F413MH, STM32F413RH, STM32F413VH, STM32F413ZH, STM32F413CG, STM32F413MG, STM32F413RG, STM32F413VG and STM32F413ZG Devices */
# arch-stm32f423xx */   /*!< STM32F423CH, STM32F423RH, STM32F423VH and STM32F423ZH Devices */


# STM32F405xx
add_library(arch-stm32f405xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f405xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F405xx)
target_include_directories(arch-stm32f405xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f405xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f405xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F415xx
add_library(arch-stm32f415xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f415xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F415xx)
target_include_directories(arch-stm32f415xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f415xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f415xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F407xx
add_library(arch-stm32f407xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f407xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F407xx)
target_include_directories(arch-stm32f407xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f407xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f407xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F417xx
add_library(arch-stm32f417xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f417xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F417xx)
target_include_directories(arch-stm32f417xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f417xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f417xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F427xx
add_library(arch-stm32f427xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f427xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F427xx)
target_include_directories(arch-stm32f427xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f427xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f427xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F437xx
add_library(arch-stm32f437xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f437xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F437xx)
target_include_directories(arch-stm32f437xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f437xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f437xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F429xx
add_library(arch-stm32f429xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f429xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F429xx)
target_include_directories(arch-stm32f429xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f429xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f429xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F439xx
add_library(arch-stm32f439xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f439xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F439xx)
target_include_directories(arch-stm32f439xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f439xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f439xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F401xC
add_library(arch-stm32f401xc OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f401xc PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F401xC)
target_include_directories(arch-stm32f401xc PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f401xc PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f401xc.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F401xE
add_library(arch-stm32f401xe OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f401xe PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F401xE)
target_include_directories(arch-stm32f401xe PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f401xe PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f401xe.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F410Tx
add_library(arch-stm32f410tx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f410tx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F410Tx)
target_include_directories(arch-stm32f410tx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f410tx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f410tx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F410Cx
add_library(arch-stm32f410cx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f410cx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F410Cx)
target_include_directories(arch-stm32f410cx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f410cx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f410cx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F410Rx
add_library(arch-stm32f410rx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f410rx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F410Rx)
target_include_directories(arch-stm32f410rx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f410rx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f410rx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F411xE
add_library(arch-stm32f411xe OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f411xe PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F411xE)
target_include_directories(arch-stm32f411xe PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f411xe PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f411xe.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F446xx
add_library(arch-stm32f446xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f446xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F446xx)
target_include_directories(arch-stm32f446xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f446xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f446xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F469xx
add_library(arch-stm32f469xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f469xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F469xx)
target_include_directories(arch-stm32f469xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f469xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f469xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F479xx
add_library(arch-stm32f479xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f479xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F479xx)
target_include_directories(arch-stm32f479xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f479xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f479xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F412Cx
add_library(arch-stm32f412cx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f412cx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F412Cx)
target_include_directories(arch-stm32f412cx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f412cx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f412cx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F412Zx
add_library(arch-stm32f412zx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f412zx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F412Zx)
target_include_directories(arch-stm32f412zx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f412zx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f412zx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F412Vx
add_library(arch-stm32f412vx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f412vx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F412Vx)
target_include_directories(arch-stm32f412vx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f412vx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f412vx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F412Rx
add_library(arch-stm32f412rx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f412rx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F412Rx)
target_include_directories(arch-stm32f412rx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f412rx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f412rx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F413xx
add_library(arch-stm32f413xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f413xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F413xx)
target_include_directories(arch-stm32f413xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f413xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f413xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)

# STM32F423xx
add_library(arch-stm32f423xx OBJECT EXCLUDE_FROM_ALL)
target_compile_definitions(arch-stm32f423xx PUBLIC -DUSE_FULL_LL_DRIVER -DUSE_HAL_DRIVER -DSTM32F423xx)
target_include_directories(arch-stm32f423xx PUBLIC "${STM32F4XX_INCLUDE_DIRS}")
target_sources(arch-stm32f423xx PRIVATE
    "${CMAKE_CURRENT_LIST_DIR}/Device/ST/STM32F4xx/Source/Templates/gcc/startup_stm32f423xx.s"
    ${STM32F4XX_LL_SOURCES}
    ${STM32F4XX_HAL_SOURCES}
)
