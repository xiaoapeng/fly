/**
 * @file key.c
 * @brief 板级按键适配：GPIO 配置 + nsbtn 注册（扫描与信号在 package/nsbtn）
 *
 * 链路：nsbtn 周期定时器（主 loop 线程）-> on_get_state()（本文件读引脚）
 *      -> nsbtn_event 信号（业务在主 loop 内订阅）。
 *
 * 引脚配置不走 BOARD_InitBootPins()：本工程从未调用它，各驱动自行初始化。
 */
/* fsl 头必须在 key.h（经 nsbtn/nanosig 引入 FreeRTOSConfig.h）之前：
 * 设备头与 FreeRTOSConfig.h 都定义 __NVIC_PRIO_BITS，设备头先来则后者 #ifndef 跳过。 */
#include "fsl_clock.h"
#include "fsl_gpio.h"
#include "fsl_port.h"

#include "key.h"

#include "runtime.h"

#include <stdbool.h>

key_t key_sw1 = { .name = "P0_6",  .pin = 6U  };  /* 板载 SW3 */
key_t key_sw2 = { .name = "P0_23", .pin = 23U };  /* 板载 SW2 */

static key_t *const keys[] = { &key_sw1, &key_sw2 };
#define KEY_COUNT ((uint32_t)(sizeof(keys) / sizeof(keys[0])))

/* ---- 引脚（当前两键都在 GPIO0/PORT0，约束见 key.h 的 pin 字段注释） -------- */

static int key_pins_init(void)
{
    gpio_pin_config_t gpio_cfg = {
        .pinDirection = kGPIO_DigitalInput,
        .outputLogic = 0U,
    };
    port_pin_config_t port_cfg = { 0 };
    uint32_t i;

    port_cfg.mux = (uint16_t)kPORT_MuxAlt0;
    port_cfg.pullSelect = (uint16_t)kPORT_PullUp;
    port_cfg.pullValueSelect = (uint16_t)kPORT_HighPullResistor;
    port_cfg.inputBuffer = (uint16_t)kPORT_InputBufferEnable;

    CLOCK_EnableClock(kCLOCK_Gpio0);
    CLOCK_EnableClock(kCLOCK_Port0);

    for(i = 0U; i < KEY_COUNT; i++) {
        PORT_SetPinConfig(PORT0, keys[i]->pin, &port_cfg);
        GPIO_PinInit(GPIO0, keys[i]->pin, &gpio_cfg);
        /* 防御：万一别处（如 pin_mux）配过边沿触发，这里显式关掉。 */
        GPIO_SetPinInterruptConfig(GPIO0, keys[i]->pin, kGPIO_InterruptStatusFlagDisabled);
    }
    return 0;
}

static void key_pins_deinit(void)
{
    uint32_t i;

    for(i = 0U; i < KEY_COUNT; i++) {
        GPIO_SetPinInterruptConfig(GPIO0, keys[i]->pin, kGPIO_InterruptStatusFlagDisabled);
        GPIO_PinClearInterruptFlag(GPIO0, keys[i]->pin);
    }
}

/* 按下为低：内部上拉保证松开时为高。 */
static bool key_pin_pressed(const key_t *k)
{
    return GPIO_PinRead(GPIO0, k->pin) == 0U;
}

/* nsbtn 状态回调：user_arg 就是注册时传入的 key_t*。 */
static uint8_t on_get_state(void *user_arg)
{
    return key_pin_pressed((const key_t *)user_arg) ? 1U : 0U;
}

int key_init(void)
{
    ns_loop_t *loop = runtime_get();
    uint32_t i;
    int rc;

    if(loop == NULL) {
        rc = NS_E_INVAL;
        goto out;
    }

    rc = nsbtn_init(loop);
    if(rc != NS_OK) {
        goto out;
    }

    if(key_pins_init() != 0) {
        rc = NS_E_NOMEM;
        goto out_nsbtn;
    }

    for(i = 0U; i < KEY_COUNT; i++) {
        if(nsbtn_register(on_get_state, (void *)keys[i]) == NULL) {
            rc = NS_E_NOMEM;
            goto out_pins;
        }
    }

    rc = nsbtn_scan_start();
    if(rc != NS_OK) {
        goto out_pins;
    }
    return NS_OK;

out_pins:
    key_pins_deinit();
out_nsbtn:
    (void)nsbtn_deinit();
out:
    return rc;
}

int key_deinit(void)
{
    int rc;

    rc = nsbtn_scan_stop();
    rc = (rc == NS_OK) ? nsbtn_deinit() : rc;
    key_pins_deinit();

    return rc;
}
