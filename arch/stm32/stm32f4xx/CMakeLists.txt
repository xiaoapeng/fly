include("${CMAKE_CURRENT_LIST_DIR}/../stm32-device.cmake")

_fly_fetch_stm32_sources(f4
    3b64b7b0b546cefe84f13c65508126eea58b97d8
    bfa4a8fde4fa5141619be006df8ef3e08265bd65
)

set(STM32_DEVICE_TEMPLATES "${STM32_DEVICE_DIR}/Source/Templates")
set(STM32_DEVICE_HEADER stm32f4xx.h)
set(STM32_SYSTEM_SOURCE system_stm32f4xx.c)

set(STM32_INCLUDE_DIRS
    "${STM32_CMSIS_INCLUDE}"
    "${STM32_DEVICE_DIR}/Include"
    "${STM32_HAL_DIR}/Inc"
)

set(STM32_LL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_usb.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_lptim.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_utils.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_sdmmc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_crc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_rng.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_dma2d.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_dac.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_fmpi2c.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_usart.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_exti.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_spi.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_dma.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_adc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_fsmc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_fmc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_ll_tim.c"
)

set(STM32_HAL_SOURCES
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_pccard.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_nor.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_mmc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dma_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_i2c_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_fmpi2c.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_eth.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_lptim.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_adc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_rtc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_irda.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_spdifrx.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dma2d.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dfsdm.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_pcd.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_cec.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_fmpsmbus_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_fmpsmbus.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dma.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_pcd_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_pwr_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_smartcard.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_rcc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_iwdg.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_cryp.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_tim_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_spi.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_rtc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_qspi.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_sram.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_hash.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_hash_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_flash_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_rng.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_sd.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_nand.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dcmi.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_sdram.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_crc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_gpio.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_tim.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_ltdc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_cortex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_fmpi2c_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dsi.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_adc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_flash_ramfunc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_exti.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_smbus.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_usart.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_i2s.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_uart.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_can.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_i2c.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_rcc_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dac_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_cryp_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_i2s_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_pwr.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_sai_ex.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dac.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_ltdc.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_flash.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_wwdg.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_hcd.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_sai.c"
    "${STM32_HAL_DIR}/Src/stm32f4xx_hal_dcmi_ex.c"
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

_fly_add_stm32_device(arch-stm32f405xx STM32F405xx startup_stm32f405xx.s)
_fly_add_stm32_device(arch-stm32f415xx STM32F415xx startup_stm32f415xx.s)
_fly_add_stm32_device(arch-stm32f407xx STM32F407xx startup_stm32f407xx.s)
_fly_add_stm32_device(arch-stm32f417xx STM32F417xx startup_stm32f417xx.s)
_fly_add_stm32_device(arch-stm32f427xx STM32F427xx startup_stm32f427xx.s)
_fly_add_stm32_device(arch-stm32f437xx STM32F437xx startup_stm32f437xx.s)
_fly_add_stm32_device(arch-stm32f429xx STM32F429xx startup_stm32f429xx.s)
_fly_add_stm32_device(arch-stm32f439xx STM32F439xx startup_stm32f439xx.s)
_fly_add_stm32_device(arch-stm32f401xc STM32F401xC startup_stm32f401xc.s)
_fly_add_stm32_device(arch-stm32f401xe STM32F401xE startup_stm32f401xe.s)
_fly_add_stm32_device(arch-stm32f410tx STM32F410Tx startup_stm32f410tx.s)
_fly_add_stm32_device(arch-stm32f410cx STM32F410Cx startup_stm32f410cx.s)
_fly_add_stm32_device(arch-stm32f410rx STM32F410Rx startup_stm32f410rx.s)
_fly_add_stm32_device(arch-stm32f411xe STM32F411xE startup_stm32f411xe.s)
_fly_add_stm32_device(arch-stm32f446xx STM32F446xx startup_stm32f446xx.s)
_fly_add_stm32_device(arch-stm32f469xx STM32F469xx startup_stm32f469xx.s)
_fly_add_stm32_device(arch-stm32f479xx STM32F479xx startup_stm32f479xx.s)
_fly_add_stm32_device(arch-stm32f412cx STM32F412Cx startup_stm32f412cx.s)
_fly_add_stm32_device(arch-stm32f412zx STM32F412Zx startup_stm32f412zx.s)
_fly_add_stm32_device(arch-stm32f412vx STM32F412Vx startup_stm32f412vx.s)
_fly_add_stm32_device(arch-stm32f412rx STM32F412Rx startup_stm32f412rx.s)
_fly_add_stm32_device(arch-stm32f413xx STM32F413xx startup_stm32f413xx.s)
_fly_add_stm32_device(arch-stm32f423xx STM32F423xx startup_stm32f423xx.s)
