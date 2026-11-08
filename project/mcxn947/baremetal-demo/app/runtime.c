/**
 * @file runtime.c
 * @brief SysTick 时钟与 nanosig 裸机主循环。
 */

#include "fsl_common.h"
#include "SEGGER_RTT.h"

#include <nanosig_baremetal_api.h>
#include <nanosig/ns_debug.h>

#include "runtime.h"

static ns_loop_t *main_loop;
static volatile uint64_t elapsed_us;

/* 仅维护时基；所有 timer、按键和 LED 回调都在主循环执行。 */
void SysTick_Handler(void)
{
    elapsed_us += UINT64_C(1000);
}

static uint64_t now_us(void)
{
    uint32_t primask = __get_PRIMASK();
    uint64_t value;

    /* Cortex-M33 的 64 位读取需避免与 ISR 更新交错。 */
    __disable_irq();
    value = elapsed_us;
    __set_PRIMASK(primask);
    return value;
}

void platform_stdout_write(void *stream, const uint8_t *buf, size_t size)
{
    (void)stream;
    (void)SEGGER_RTT_Write(0, buf, (unsigned)size);
}

ns_loop_t *runtime_get(void)
{
    return main_loop;
}

int runtime_init(void)
{
    int rc;

    elapsed_us = 0u;
    ns_baremetal_set_now_us(now_us);
    rc = ns_init();
    if(rc != NS_OK) {
        goto out_clock;
    }

    rc = ns_loop_create(&main_loop, NULL);
    if(rc != NS_OK) {
        goto out_shutdown;
    }

    if(SysTick_Config(SystemCoreClock / 1000u) != 0u) {
        rc = NS_E_INVAL;
        goto out_loop;
    }
    ns_sysln("nanosig bare-metal loop -> SEGGER RTT");
    return NS_OK;

out_loop:
    (void)ns_loop_destroy(main_loop);
    main_loop = NULL;
out_shutdown:
    (void)ns_shutdown();
out_clock:
    ns_baremetal_set_now_us(NULL);
    return rc;
}

int runtime_deinit(void)
{
    int rc;
    int shutdown_rc;

    SysTick->CTRL = 0u;
    SCB->ICSR = SCB_ICSR_PENDSTCLR_Msk;
    rc = ns_loop_destroy(main_loop);
    main_loop = NULL;
    shutdown_rc = ns_shutdown();
    ns_baremetal_set_now_us(NULL);
    return (rc == NS_OK) ? shutdown_rc : rc;
}

int runtime_run(void)
{
    int rc;

    if(main_loop == NULL) {
        return NS_E_INVAL;
    }
    for(;;) {
        rc = ns_loop_poll(main_loop, 1000u);
        if(rc != NS_OK) {
            return rc;
        }
        __WFI();
    }
}
