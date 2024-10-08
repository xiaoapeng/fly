/***********************************************************************************************************************
 * This file was generated by the MCUXpresso Config Tools. Any manual edits made to this file
 * will be overwritten if the respective MCUXpresso Config Tools is used to update this file.
 **********************************************************************************************************************/
/*
 * How to setup clock using clock driver functions:
 *
 * 1. Setup clock sources.
 *
 * 2. Set up wait states of the flash.
 *
 * 3. Set up all dividers.
 *
 * 4. Set up all selectors to provide selected clocks.
 *
 */

/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!GlobalInfo
product: Clocks v13.0
processor: MCXN947
package_id: MCXN947VDF
mcu_data: ksdk2_0
processor_version: 15.1.0
board: FRDM-MCXN947
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

#include "fsl_clock.h"
#include "clock_config.h"
#include "fsl_spc.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Variables
 ******************************************************************************/
/* System clock frequency. */
extern uint32_t SystemCoreClock;

/*******************************************************************************
 ************************ BOARD_InitBootClocks function ************************
 ******************************************************************************/
void BOARD_InitBootClocks(void)
{
    BOARD_BootClockPLL150M();
}

/*******************************************************************************
 ******************** Configuration BOARD_BootClockPLL150M *********************
 ******************************************************************************/
/* clang-format off */
/* TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
!!Configuration
name: BOARD_BootClockPLL150M
called_from_default_init: true
outputs:
- {id: CLK_144M_clock.outFreq, value: 144 MHz}
- {id: CLK_48M_clock.outFreq, value: 48 MHz}
- {id: ENETRMII_clock.outFreq, value: 50 MHz, locked: true, accuracy: '0.001'}
- {id: FLEXCOMM4_clock.outFreq, value: 12 MHz, locked: true, accuracy: '0.001'}
- {id: FRO_12M_clock.outFreq, value: 12 MHz}
- {id: FRO_HF_clock.outFreq, value: 48 MHz}
- {id: MAIN_clock.outFreq, value: 150 MHz, locked: true, accuracy: '0.001'}
- {id: PLL0_CLK_clock.outFreq, value: 150 MHz, locked: true, accuracy: '0.001'}
- {id: Slow_clock.outFreq, value: 37.5 MHz}
- {id: System_clock.outFreq, value: 150 MHz}
- {id: gdet_clock.outFreq, value: 48 MHz}
- {id: trng_clock.outFreq, value: 48 MHz}
settings:
- {id: PLL0_Mode, value: Normal}
- {id: RunPowerMode, value: OD}
- {id: SCGMode, value: PLL0}
- {id: ENETRMIICLKDIV, value: Enable}
- {id: FLEXCOMM4CLKDIV_HALT, value: Enable}
- {id: SCG.PLL0M_MULT.scale, value: '50', locked: true}
- {id: SCG.PLL0SRCSEL.sel, value: SCG.FIRC_48M}
- {id: SCG.PLL0_NDIV.scale, value: '8', locked: true}
- {id: SCG.SCSSEL.sel, value: SCG.PLL0_CLK}
- {id: SCG_SOSCCSR_SOSCEN_CFG, value: Enabled}
- {id: SYSCON.ENETRMIICLKDIV.scale, value: '3', locked: true}
- {id: SYSCON.ENETRMIICLKSEL.sel, value: SCG.PLL0_CLK}
- {id: SYSCON.FCCLKSEL4.sel, value: SCG.FRO_12M}
- {id: SYSCON.FLEXSPICLKSEL.sel, value: NO_CLOCK}
- {id: SYSCON.FREQMEREFCLKSEL.sel, value: SYSCON.evtg_out0a}
- {id: SYSCON.FREQMETARGETCLKSEL.sel, value: SYSCON.evtg_out0a}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS **********/
/* clang-format on */

/*******************************************************************************
 * Variables for BOARD_BootClockPLL150M configuration
 ******************************************************************************/
/*******************************************************************************
 * Code for BOARD_BootClockPLL150M configuration
 ******************************************************************************/
void BOARD_BootClockPLL150M(void)
{
    CLOCK_EnableClock(kCLOCK_Scg);                     /*!< Enable SCG clock */

    /* FRO OSC setup - begin, attach FRO12M to MainClock for safety switching */
    CLOCK_AttachClk(kFRO12M_to_MAIN_CLK);              /*!< Switch to FRO 12M first to ensure we can change the clock setting */

    /* Set the DCDC VDD regulator to 1.2 V voltage level */
    spc_active_mode_dcdc_option_t dcdcOpt = {
      .DCDCVoltage       = kSPC_DCDC_OverdriveVoltage,
      .DCDCDriveStrength = kSPC_DCDC_NormalDriveStrength,
    };
    SPC_SetActiveModeDCDCRegulatorConfig(SPC0, &dcdcOpt);
    /* Set the LDO_CORE VDD regulator to 1.2 V voltage level */
    spc_active_mode_core_ldo_option_t ldoOpt = {
      .CoreLDOVoltage       = kSPC_CoreLDO_OverDriveVoltage,
      .CoreLDODriveStrength = kSPC_CoreLDO_NormalDriveStrength,
    };
    SPC_SetActiveModeCoreLDORegulatorConfig(SPC0, &ldoOpt);
    /* Configure Flash wait-states to support 1.2V voltage level and 150000000Hz frequency */;
    FMU0->FCTRL = (FMU0->FCTRL & ~((uint32_t)FMU_FCTRL_RWSC_MASK)) | (FMU_FCTRL_RWSC(0x3U));
    /* Specifies the 1.2V operating voltage for the SRAM's read/write timing margin */
    spc_sram_voltage_config_t sramCfg = {
      .operateVoltage       = kSPC_sramOperateAt1P2V,
      .requestVoltageUpdate = true,
    };
    SPC_SetSRAMOperateVoltage(SPC0, &sramCfg);

    CLOCK_SetupFROHFClocking(48000000U);               /*!< Enable FRO HF(48MHz) output */
    /*!< Set up PLL0 */
    const pll_setup_t pll0Setup = {
        .pllctrl = SCG_APLLCTRL_SOURCE(1U) | SCG_APLLCTRL_SELI(27U) | SCG_APLLCTRL_SELP(13U),
        .pllndiv = SCG_APLLNDIV_NDIV(8U),
        .pllpdiv = SCG_APLLPDIV_PDIV(1U),
        .pllmdiv = SCG_APLLMDIV_MDIV(50U),
        .pllRate = 150000000U
    };
    CLOCK_SetPLL0Freq(&pll0Setup);                       /*!< Configure PLL0 to the desired values */
    CLOCK_SetPll0MonitorMode(kSCG_Pll0MonitorDisable);    /* Pll0 Monitor is disabled */

    /*!< Set up clock selectors  */
    CLOCK_AttachClk(kPLL0_to_MAIN_CLK);
    CLOCK_AttachClk(kFRO12M_to_FLEXCOMM4);                 /*!< Switch FLEXCOMM4 to FRO12M */
    CLOCK_AttachClk(kPLL0_to_ENETRMII);                 /*!< Switch ENETRMII to PLL0 */

    /*!< Set up dividers */
    CLOCK_SetClkDiv(kCLOCK_DivAhbClk, 1U);           /*!< Set AHBCLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivFlexcom4Clk, 1U);           /*!< Set FLEXCOMM4CLKDIV divider to value 1 */
    CLOCK_SetClkDiv(kCLOCK_DivEnetrmiiClk, 3U);           /*!< Set ENETRMIICLKDIV divider to value 3 */

    /* Set SystemCoreClock variable */
    SystemCoreClock = BOARD_BOOTCLOCKPLL150M_CORE_CLOCK;
}

