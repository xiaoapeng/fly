/**
 * @file debug.c
 * @brief FreeRTOS demo LPUART init (no eh dependency)
 */

#include "fsl_lpuart.h"
#include "fsl_port.h"

static void board_debug_pin_mux_init(void)
{
    const port_pin_config_t tx = {
        kPORT_PullDisable, kPORT_LowPullResistor, kPORT_FastSlewRate,
        kPORT_PassiveFilterDisable, kPORT_OpenDrainDisable,
        kPORT_LowDriveStrength, kPORT_MuxAlt2, kPORT_InputBufferEnable,
        kPORT_InputNormal, kPORT_UnlockRegister,
    };
    PORT_SetPinConfig(PORT1, 8U, &tx);
    PORT_SetPinConfig(PORT1, 9U, &tx);
}

int board_debug_init(void)
{
    lpuart_config_t config;
    /* PORT register access requires the PORT clock first; writing PCR on a
     * clock-gated PORT triggers a HardFault (bus error) on MCXN947. */
    CLOCK_EnableClock(kCLOCK_Port1);
    board_debug_pin_mux_init();
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200;
    config.enableTx     = true;
    config.enableRx     = true;
    LPUART_Init(LPUART4, &config, 12000000UL);
    return 0;
}