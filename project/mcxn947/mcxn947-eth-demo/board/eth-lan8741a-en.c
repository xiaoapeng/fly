/**
 * @file eth-lan8741a-en.c
 * @brief eth-lan8741a-en ethernet phy driver
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-02
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#include <stdalign.h>

#include "eh.h"
#include "eh_event.h"
#include "eh_event_flags.h"
#include "eh_error.h"
#include "eh_mem.h"
#include "eh_module.h"
#include "eh_debug.h"
#include "eh_sleep.h"
#include "eh_signal.h"

#include "eh_types.h"
#include "fsl_common.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "fsl_enet.h"


#define PCR_IBE_ibe1 0x01u        /*!<@brief Input Buffer Enable: Enables */
#define PORT5_PCR_MUX_mux00 0x00u /*!<@brief Pin Multiplex Control: Alternative 0 (GPIO) */

#define PHY_BASICCONTROL_REG        (0x00U) /*!< The PHY basic control register. */
#define PHY_BASICSTATUS_REG         (0x01U) /*!< The PHY basic status register. */
#define PHY_ID1_REG                 (0x02U) /*!< The PHY ID one register. */
#define PHY_ID2_REG                 (0x03U) /*!< The PHY ID two register. */
#define PHY_AUTONEG_ADVERTISE_REG   (0x04U) /*!< The PHY auto-negotiate advertise register. */
#define PHY_AUTONEG_LINKPARTNER_REG (0x05U) /*!< The PHY auto negotiation link partner ability register. */
#define PHY_AUTONEG_EXPANSION_REG   (0x06U) /*!< The PHY auto negotiation expansion register. */
#define PHY_1000BASET_CONTROL_REG   (0x09U) /*!< The PHY 1000BASE-T control register. */
#define PHY_MMD_ACCESS_CONTROL_REG  (0x0DU) /*!< The PHY MMD access control register. */
#define PHY_MMD_ACCESS_DATA_REG     (0x0EU) /*!< The PHY MMD access data register. */

/* Symbols to be used with GPIO driver */
#define BOARD_ETH_PINS_ETH_RESET_GPIO GPIO5               /*!<@brief GPIO peripheral base pointer */
#define BOARD_ETH_PINS_ETH_RESET_GPIO_PIN 8U              /*!<@brief GPIO pin number */
#define BOARD_ETH_PINS_ETH_RESET_GPIO_PIN_MASK (1U << 8U) /*!<@brief GPIO pin mask */

/* Symbols to be used with PORT driver */
#define BOARD_ETH_PINS_ETH_RESET_PORT PORT5               /*!<@brief PORT peripheral base pointer */
#define BOARD_ETH_PINS_ETH_RESET_PIN 8U                   /*!<@brief PORT pin number */
#define BOARD_ETH_PINS_ETH_RESET_PIN_MASK (1U << 8U)      /*!<@brief PORT pin mask */
                                                          /* @} */
#define BOARD_ETH_ENET_RXBD_NUM 5
#define BOARD_ETH_ENET_TXBD_NUM 5
#define BOARD_ETH_ENET_FRAME_MAX_FRAMELEN  eh_align_up(1518, 4)
#define BOARD_ETH_ENET_PRIORITY (2U)
#define BOARD_ETH_ENET_PHY_ADDR (0x00)

#define BOARD_ETH_EVENT_FLAGS_RX_INTEVENT               (0x1UL << kENET_RxIntEvent)
#define BOARD_ETH_EVENT_FLAGS_TX_INTEVENT               (0x1UL << kENET_TxIntEvent)
#define BOARD_ETH_EVENT_FLAGS_WAKEUP_INTEVENT           (0x1UL << kENET_WakeUpIntEvent)
#define BOARD_ETH_EVENT_FLAGS_TIMESTAMP_INTEVENT        (0x1UL << kENET_TimeStampIntEvent)
#define BOARD_ETH_EVENT_FLAGS_MASK (BOARD_ETH_EVENT_FLAGS_RX_INTEVENT | BOARD_ETH_EVENT_FLAGS_TX_INTEVENT | BOARD_ETH_EVENT_FLAGS_WAKEUP_INTEVENT | BOARD_ETH_EVENT_FLAGS_TIMESTAMP_INTEVENT)



static alignas(ENET_BUFF_ALIGNMENT) enet_rx_bd_struct_t s_rx_buff_descrip[BOARD_ETH_ENET_RXBD_NUM];
static alignas(ENET_BUFF_ALIGNMENT) enet_tx_bd_struct_t s_tx_buff_descrip[BOARD_ETH_ENET_TXBD_NUM];
static uint32_t rx_buffer_start_addr[BOARD_ETH_ENET_RXBD_NUM];
static enet_tx_reclaim_info_t   s_tx_dirty[BOARD_ETH_ENET_TXBD_NUM];
static enet_handle_t s_enet_handle;

EH_DEFINE_STATIC_CUSTOM_SIGNAL(signal_eth_event_flags, eh_event_flags_t, {});


static void eth_event_slot_function(eh_event_t *e, void *slot_param);

EH_DEFINE_SLOT(slot_eth_event, eth_event_slot_function, NULL);

/* clang-format off */
/*
 * TEXT BELOW IS USED AS SETTING FOR TOOLS *************************************
BOARD_Eth_Pins:
- options: {callFromInitBoot: 'true', coreID: cm33_core0, enableClock: 'true'}
- pin_list:
  - {pin_num: K5, peripheral: ENET0, signal: enet_mdc, pin_signal: PIO1_20/TRIG_IN2/FC5_P4/FC4_P0/CT3_MAT2/SCT0_OUT8/FLEXIO0_D28/PLU_OUT6/ENET0_MDC/CAN1_TXD/ADC1_A20/CMP1_IN3}
  - {pin_num: L5, peripheral: ENET0, signal: enet_mdio, pin_signal: PIO1_21/TRIG_OUT2/FC5_P5/FC4_P1/CT3_MAT3/SCT0_OUT9/FLEXIO0_D29/PLU_OUT7/ENET0_MDIO/SAI1_MCLK/CAN1_RXD/ADC1_A21/CMP2_IN3}
  - {pin_num: D4, peripheral: ENET0, signal: 'enet_rdata, 0', pin_signal: PIO1_14/FC4_P6/FC3_P2/CT_INP10/SCT0_IN4/FLEXIO0_D22/PLU_IN2/ENET0_RXD0/TSI0_CH23/ADC1_A14}
  - {pin_num: E4, peripheral: ENET0, signal: 'enet_rdata, 1', pin_signal: PIO1_15/WUU0_IN13/FC3_P3/CT_INP11/SCT0_IN5/FLEXIO0_D23/PLU_IN3/ENET0_RXD1/I3C1_PUR/TSI0_CH24/ADC1_A15}
  - {pin_num: A2, peripheral: ENET0, signal: 'enet_tdata, 1', pin_signal: PIO1_7/WUU0_IN9/TRIG_OUT2/FC5_P3/CT_INP7/SCT0_IN1/FLEXIO0_D15/PLU_CLK/ENET0_TXD1/SAI1_RX_FS/CAN1_RXD/TSI0_CH7/ADC0_A23}
  - {pin_num: B2, peripheral: ENET0, signal: 'enet_tdata, 0', pin_signal: PIO1_6/TRIG_IN2/FC3_P6/FC5_P2/CT_INP6/SCT0_IN0/FLEXIO0_D14/ENET0_TXD0/SAI1_RX_BCLK/CAN1_TXD/TSI0_CH6/ADC0_A22}
  - {pin_num: A4, peripheral: ENET0, signal: enet_tx_clk, pin_signal: PIO1_4/WUU0_IN8/FREQME_CLK_IN0/FC3_P4/FC5_P0/CT1_MAT2/SCT0_OUT0/FLEXIO0_D12/ENET0_TX_CLK/SAI0_TXD1/TSI0_CH4/ADC0_A20/CMP0_IN2}
  - {pin_num: B3, peripheral: ENET0, signal: enet_tx_en, pin_signal: PIO1_5/FREQME_CLK_IN1/FC3_P5/FC5_P1/CT1_MAT3/SCT0_OUT1/FLEXIO0_D13/ENET0_TXEN/SAI0_RXD1/TSI0_CH5/ADC0_A21/CMP0_IN3}
  - {pin_num: D1, peripheral: ENET0, signal: enet_rx_dv, pin_signal: PIO1_13/TRIG_IN3/FC4_P5/FC3_P1/CT2_MAT3/SCT0_OUT5/FLEXIO0_D21/PLU_OUT3/ENET0_RXDV/CAN1_TXD/TSI0_CH22/ADC1_A13}
  - {pin_num: L14, peripheral: GPIO5, signal: 'GPIO, 8', pin_signal: PIO5_8/TRIG_OUT7/TAMPER6/ADC1_B16, direction: OUTPUT, gpio_init_state: 'false'}
 * BE CAREFUL MODIFYING THIS COMMENT - IT IS YAML SETTINGS FOR TOOLS ***********
 */
/* clang-format on */

/* FUNCTION ************************************************************************************************************
 *
 * Function Name : BOARD_Eth_Pins
 * Description   : Configures pin routing and optionally pin electrical features.
 *
 * END ****************************************************************************************************************/
static void BOARD_Eth_Pins(void)
{
    /* Enables the clock for PORT1: Enables clock */
    CLOCK_EnableClock(kCLOCK_Port1);

    gpio_pin_config_t ETH_RESET_config = {
        .pinDirection = kGPIO_DigitalOutput,
        .outputLogic = 0U
    };
    /* Initialize GPIO functionality on pin PIO5_8 (pin L14)  */
    GPIO_PinInit(BOARD_ETH_PINS_ETH_RESET_GPIO, BOARD_ETH_PINS_ETH_RESET_PIN, &ETH_RESET_config);

    /* PORT1_13 (pin D1) is configured as ENET0_RXDV */
    PORT_SetPinMux(PORT1, 13U, kPORT_MuxAlt9);

    PORT1->PCR[13] = ((PORT1->PCR[13] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_IBE_MASK)))

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_14 (pin D4) is configured as ENET0_RXD0 */
    PORT_SetPinMux(PORT1, 14U, kPORT_MuxAlt9);

    PORT1->PCR[14] = ((PORT1->PCR[14] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_IBE_MASK)))

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_15 (pin E4) is configured as ENET0_RXD1 */
    PORT_SetPinMux(PORT1, 15U, kPORT_MuxAlt9);

    PORT1->PCR[15] = ((PORT1->PCR[15] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_IBE_MASK)))

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_20 (pin K5) is configured as ENET0_MDC */
    PORT_SetPinMux(PORT1, 20U, kPORT_MuxAlt9);

    PORT1->PCR[20] = ((PORT1->PCR[20] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_IBE_MASK)))

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_21 (pin L5) is configured as ENET0_MDIO */
    PORT_SetPinMux(PORT1, 21U, kPORT_MuxAlt9);

    PORT1->PCR[21] = ((PORT1->PCR[21] &
                       /* Mask bits to zero which are setting */
                       (~(PORT_PCR_IBE_MASK)))

                      /* Input Buffer Enable: Enables. */
                      | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_4 (pin A4) is configured as ENET0_TX_CLK */
    PORT_SetPinMux(PORT1, 4U, kPORT_MuxAlt9);

    PORT1->PCR[4] = ((PORT1->PCR[4] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_5 (pin B3) is configured as ENET0_TXEN */
    PORT_SetPinMux(PORT1, 5U, kPORT_MuxAlt9);

    PORT1->PCR[5] = ((PORT1->PCR[5] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_6 (pin B2) is configured as ENET0_TXD0 */
    PORT_SetPinMux(PORT1, 6U, kPORT_MuxAlt9);

    PORT1->PCR[6] = ((PORT1->PCR[6] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    /* PORT1_7 (pin A2) is configured as ENET0_TXD1 */
    PORT_SetPinMux(PORT1, 7U, kPORT_MuxAlt9);

    PORT1->PCR[7] = ((PORT1->PCR[7] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_IBE_MASK)))

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));

    PORT5->PCR[8] = ((PORT5->PCR[8] &
                      /* Mask bits to zero which are setting */
                      (~(PORT_PCR_MUX_MASK | PORT_PCR_IBE_MASK)))

                     /* Pin Multiplex Control: PORT5_8 (pin L14) is configured as PIO5_8. */
                     | PORT_PCR_MUX(PORT5_PCR_MUX_mux00)

                     /* Input Buffer Enable: Enables. */
                     | PORT_PCR_IBE(PCR_IBE_ibe1));
}


static void *ethernetif_rx_alloc(ENET_Type *base, void *userData, uint8_t ringId){
    (void) base;
    (void) userData;
    (void) ringId;
    eh_debugfl("debug");
    return eh_malloc(BOARD_ETH_ENET_FRAME_MAX_FRAMELEN);
}

static void ethernetif_rx_free(ENET_Type *base, void *buffer, void *userData, uint8_t ringId){
    (void) base;
    (void) userData;
    (void) ringId;
    eh_debugfl("debug");
    eh_free(buffer);
}

static void ethernet_callback(ENET_Type *base,
                              enet_handle_t *handle,
                              enet_event_t event,
                              uint8_t channel,
                              enet_tx_reclaim_info_t *txReclaimInfo,
                              void *userData){
    (void) base;
    (void) handle;
    (void) channel;
    (void) txReclaimInfo;
    (void) userData;
    
    eh_event_flags_set_bits(eh_signal_to_custom_event(&signal_eth_event_flags), 1UL << event);
}


static int phy_mdio_read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData){
    return ENET_MDIORead(ENET0, phyAddr, regAddr, pData);
}
static int phy_mdio_write(uint8_t phyAddr, uint8_t regAddr, uint16_t data){
    return ENET_MDIOWrite(ENET0, phyAddr, regAddr, data);
}

static void eth_event_slot_function(eh_event_t *e, void *slot_param){
    (void) slot_param;
    eh_flags_t reality_flags;
    eh_event_flags_t *ef = (eh_event_flags_t*)e;
    status_t status;
    int ret;
    enet_buffer_struct_t buffers[BOARD_ETH_ENET_RXBD_NUM] = {{0}};
    enet_rx_frame_struct_t rx_frame                      = {.rxBuffArray = &buffers[0]};
    ret = eh_event_flags_wait(ef, BOARD_ETH_EVENT_FLAGS_MASK, BOARD_ETH_EVENT_FLAGS_MASK, &reality_flags, 0);
    if(ret < 0){
        eh_warnfl("eh_event_flags_wait fail ret = %d");
        return ;
    }
    eh_infofl("reality_flags = %08x", reality_flags);
    
    status = ENET_GetRxFrame(ENET0, &s_enet_handle, &rx_frame, 0);
    eh_debugfl("status = %08x", status);
    if(status == kStatus_Success){
        eh_infoln("rx frame len = %d", rx_frame.totLen);
        eh_infoln("rx data :|%.*hhq|", (int)rx_frame.rxBuffArray[0].length, rx_frame.rxBuffArray[0].buffer);
        eh_free(rx_frame.rxBuffArray[0].buffer);
    }
    
}

static void eth_phy_reset(void){
    GPIO_PinWrite(GPIO5, 8, 0);
    eh_usleep(1000*10);
    GPIO_PinWrite(GPIO5, 8, 1);
    eh_usleep(1000*100);
}



static int eth_phy_init_task(void* arg){
    (void)arg;
    uint16_t data;
    uint32_t phy_id;
    int ret;
    eth_phy_reset();

    phy_mdio_read(BOARD_ETH_ENET_PHY_ADDR, PHY_ID1_REG, &data);
    phy_id = (uint32_t)data << 16;
    phy_mdio_read(BOARD_ETH_ENET_PHY_ADDR, PHY_ID2_REG, &data);
    phy_id = phy_id | ((uint32_t)data);

    eh_infoln("lan8741a phy id:0x%08x", phy_id);
    
    phy_mdio_write(BOARD_ETH_ENET_PHY_ADDR, PHY_BASICCONTROL_REG, 0x8000);
    
    do{
        ret = phy_mdio_read(BOARD_ETH_ENET_PHY_ADDR, PHY_BASICCONTROL_REG, &data);
        if(ret != kStatus_Success){
            eh_warnfl("phy_mdio_read fail ret = %d", ret);
            return -1;
        }
    }while(data & 0x8000);
    
    /* 启动自协商 */
    ret = phy_mdio_write(BOARD_ETH_ENET_PHY_ADDR, PHY_AUTONEG_ADVERTISE_REG, 0x1E0U | 0x001U);
    if(ret != kStatus_Success){
        eh_warnfl("phy_mdio_write fail ret = %d", ret);
        return -1;
    }
    
    ret = phy_mdio_write(BOARD_ETH_ENET_PHY_ADDR, PHY_BASICCONTROL_REG, 0x1000U | 0x200U);
    if(ret != kStatus_Success){
        eh_warnfl("phy_mdio_write fail ret = %d", ret);
        return -1;
    }
    return 0;
}
static int eth_phy_init(void){
    eh_task_t * task_init;
    task_init = eh_task_create("phy_init", EH_TASK_FLAGS_DETACH, 1024, NULL, eth_phy_init_task);
    return eh_ptr_to_error(task_init);
}


static int __init eth_lan8741a_en_init(void){
    enet_config_t config;
    enet_buffer_config_t buff_cfg;
    
    BOARD_Eth_Pins();

    CLOCK_AttachClk(MUX_A(CM_ENETRMIICLKSEL, 0));
    CLOCK_EnableClock(kCLOCK_Enet);
    SYSCON0->PRESETCTRL2 = SYSCON_PRESETCTRL2_ENET_RST_MASK;
    SYSCON0->PRESETCTRL2 &= ~SYSCON_PRESETCTRL2_ENET_RST_MASK;

    /* 初始化MDIO */
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(ENET0)]);
    ENET_SetSMI(ENET0, CLOCK_GetCoreSysClkFreq());
    
    /* 初始化ENET */
    buff_cfg.rxRingLen = BOARD_ETH_ENET_RXBD_NUM;
    buff_cfg.txRingLen = BOARD_ETH_ENET_TXBD_NUM;
    buff_cfg.txDescStartAddrAlign = s_tx_buff_descrip;
    buff_cfg.txDescTailAddrAlign = s_tx_buff_descrip + BOARD_ETH_ENET_TXBD_NUM;
    buff_cfg.txDirtyStartAddr = s_tx_dirty;
    buff_cfg.rxDescStartAddrAlign = s_rx_buff_descrip;
    buff_cfg.rxDescTailAddrAlign = s_rx_buff_descrip + BOARD_ETH_ENET_RXBD_NUM;
    buff_cfg.rxBufferStartAddr = rx_buffer_start_addr;
    buff_cfg.rxBuffSizeAlign = BOARD_ETH_ENET_FRAME_MAX_FRAMELEN;
    
    ENET_GetDefaultConfig(&config);
    config.multiqueueCfg = NULL;
    config.rxBuffAlloc   = ethernetif_rx_alloc;
    config.rxBuffFree    = ethernetif_rx_free;
    config.specialControl |= kENET_RxChecksumOffloadEnable;
    
    config.interrupt = kENET_DmaTx | kENET_DmaRx;
    NVIC_SetPriority(ETHERNET_IRQn, BOARD_ETH_ENET_PRIORITY);
    
    ENET_Init(ENET0, &config, (uint8_t*)"\x54\x27\x8d\x12\x34\x56", 50000000);
    
    EH_DBG_ERROR_EXEC(ENET_DescriptorInit(ENET0, &config, &buff_cfg) != kStatus_Success, return -1);

    ENET_CreateHandler(ENET0, &s_enet_handle, &config, &buff_cfg, ethernet_callback, NULL);
    
    EH_DBG_ERROR_EXEC(ENET_RxBufferAllocAll(ENET0, &s_enet_handle) != kStatus_Success, return -1);
    
    ENET_StartRxTx(ENET0, 1, 1);
    
    eth_phy_init();
    
    eh_event_flags_init(eh_signal_to_custom_event(&signal_eth_event_flags));
    eh_signal_register(&signal_eth_event_flags);
    
    eh_signal_slot_connect(&signal_eth_event_flags, &slot_eth_event);
    

    
    return 0;
}


static void __init eth_lan8741a_en_exit(void){
    eh_signal_slot_disconnect(&slot_eth_event);
    eh_signal_unregister(&signal_eth_event_flags);
    eh_event_flags_clean(eh_signal_to_custom_event(&signal_eth_event_flags));
    
    /*  */
}



eh_module_level9_export(eth_lan8741a_en_init, eth_lan8741a_en_exit);

