/**
 * @file main.c
 * @brief FreeRTOS demo: RTT tick + LED blink + nanosig signal/watcher on mcxn947
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "SEGGER_RTT.h"
#include "fsl_clock.h"
#include "fsl_ctimer.h"
#include "led.h"
#include <stdio.h>

#include <nanosig/nanosig.h>
#include <nanosig/nanosig_signal.h>
#include <nanosig/nanosig_loop.h>
#include <nanosig/nanosig_broker.h>
#include <nanosig/ns_debug.h>

extern void init(void);

/* nanosig funnels all debug output (ns_debug / ns_printf) through the weak
 * stdout_write() hook. The FreeRTOS platform backend (platform/freertos/port.c)
 * provides the strong stdout_write() that forwards to the weak
 * platform_stdout_write() below; overriding it here routes every nanosig
 * internal log line (broker / platform / timer) to the same SEGGER RTT channel
 * the demo uses. */
void platform_stdout_write(void *stream, const uint8_t *buf, size_t size)
{
    (void)stream;
    (void)SEGGER_RTT_Write(0, buf, (unsigned)size);
}

static ns_loop_t *demo_loop = NULL;
static ns_signal_t demo_signal;
static ns_connection_t demo_conn;

/* Watcher backend: a caller-owned binary semaphore registered as a queue-set
 * member. The CTIMER0 ISR gives it via xSemaphoreGiveFromISR; the broker
 * waitset reports readiness and consume_fn drains it (queue-set contract:
 * ready-not-consumed). */
static SemaphoreHandle_t watcher_member = NULL;

/* Hardware-interrupt trigger, started once the watcher is registered. */
static void isr_trigger_start(void);
static ns_watcher_t demo_watcher;
static ns_connection_t watcher_conn;
static volatile uint32_t watcher_hits = 0u;
static volatile int loop_exited = 0;

static void on_signal(void *user_data, const ns_no_payload_t *payload)
{
    (void)user_data;
    (void)payload;
    /* Runs in loop_task context (the task driving the ns loop). */
    SEGGER_RTT_Write(0, "nanosig: callback received\n", 27);
}

static int watcher_consume(ns_watcher_t *watcher)
{
    (void)watcher;
    /* Drain the readiness member in the broker thread. Return 1 so the
     * broker emits the watcher signal; any value <= 0 would skip the emit. */
    (void)xSemaphoreTake(watcher_member, 0);
    return 1;
}

static void on_watcher(void *user_data, const ns_watcher_event_t *event)
{
    (void)user_data;
    (void)event;
    watcher_hits++;
    SEGGER_RTT_Write(0, "nanosig: watcher callback\n", 26);
}

/* Runs the single nanosig main loop; subscription callbacks execute here. */
static void loop_task(void *p)
{
    (void)p;
    ns_init();
    /* nanosig debug output is routed to RTT via the stdout_write override
     * above; keep the level at SYS (errors/warnings/sys) to avoid DEBUG flood. */
    /* Global debug level is seeded from Kconfig
     * (CONFIG_PACKAGE_NANOSIG_LOG_LEVEL -> NS_CONFIG_DEFAULT_DEBUG_LEVEL);
     * no runtime override here so the Kconfig knob stays authoritative. */
    ns_sysln("nanosig debug bridge -> SEGGER RTT");
    if(ns_loop_init(&demo_loop, NULL) != NS_OK) {
        SEGGER_RTT_Write(0, "nanosig: loop init failed\n", 26);
        vTaskDelete(NULL);
        return;
    }
    ns_signal_init(&demo_signal, ns_no_payload_t);
    ns_signal_connect(&demo_signal, (ns_slot_fn)on_signal, demo_loop, NULL, &demo_conn);

    /* Register a watcher backed by a caller-owned binary semaphore member. */
    watcher_member = xSemaphoreCreateBinary();
    if(watcher_member == NULL) {
        SEGGER_RTT_Write(0, "nanosig: member sem create failed\n", 34);
    } else if(ns_watcher_init(&demo_watcher,
                    (ns_waitable_handle_t){.handle = watcher_member},
                    NS_WAITABLE_EVENT_IN, 0, watcher_consume) != NS_OK) {
        SEGGER_RTT_Write(0, "nanosig: watcher init failed\n", 29);
    } else {
        ns_signal_connect_typed(demo_watcher.signal, on_watcher, ns_watcher_event_t,
                                demo_loop, NULL, &watcher_conn);
        if(ns_broker_add(&demo_watcher) != NS_OK) {
            SEGGER_RTT_Write(0, "nanosig: broker add failed\n", 27);
        } else {
            SEGGER_RTT_WriteString(0, "nanosig: watcher registered (CTIMER0 ISR)\n");
            isr_trigger_start();
        }
    }

    /* Blocks in ns_platform_wakeup_wait (task notification) until a signal
     * is emitted from another task; callback dispatch happens here.
     * ns_loop_run only returns on ns_loop_quit (see shutdown_task). */
    (void)ns_loop_run(demo_loop);
    SEGGER_RTT_Write(0, "nanosig: loop exited\n", 21);
    loop_exited = 1;
    vTaskDelete(NULL);
}

/* Periodically emits the demo signal from a different task context. */
static void emit_task(void *p)
{
    (void)p;
    uint32_t n = 0;
    for(;;) {
        char msg[32];
        int len = snprintf(msg, sizeof(msg), "nanosig: emit #%u\n", (unsigned)n);
        if(len > 0) {
            SEGGER_RTT_Write(0, msg, (unsigned)len);
        }
        ns_signal_emit(demo_signal, NS_NO_PAYLOAD);
        n++;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

/* Hardware-interrupt trigger: CTIMER0 match0 fires the ISR at a fixed rate and
 * the ISR gives the watcher member via xSemaphoreGiveFromISR. This is the real
 * "queue-set member given from an ISR" path the backend exists for — there is
 * deliberately no task-context trigger, so any watcher callback must originate
 * from this ISR. */
#define ISR_TRIGGER_PERIOD_MS 300u
#define ISR_TRIGGER_CLK_HZ    (12UL * 1000UL * 1000UL)

static volatile uint32_t isr_hits = 0u;

void CTIMER0_IRQHandler(void)
{
    BaseType_t woken = pdFALSE;

    if((CTIMER_GetStatusFlags(CTIMER0) & (uint32_t)kCTIMER_Match0Flag) != 0u) {
        CTIMER_ClearStatusFlags(CTIMER0, (uint32_t)kCTIMER_Match0Flag);
        isr_hits++;
        if(watcher_member != NULL) {
            (void)xSemaphoreGiveFromISR(watcher_member, &woken);
        }
    }
    portYIELD_FROM_ISR(woken);
}

static void isr_trigger_start(void)
{
    ctimer_config_t tcfg;
    ctimer_match_config_t mcfg;

    /* CTIMER_Init() gates the peripheral clock itself; only the clock source
     * has to be selected (FRO 12 MHz, so the match value is exact). */
    CLOCK_AttachClk(kFRO12M_to_CTIMER0);
    CTIMER_GetDefaultConfig(&tcfg);
    tcfg.prescale = 0u;
    CTIMER_Init(CTIMER0, &tcfg);

    mcfg.matchValue         = (uint32_t)((ISR_TRIGGER_CLK_HZ / 1000UL) * ISR_TRIGGER_PERIOD_MS);
    mcfg.enableCounterReset = true;
    mcfg.enableCounterStop  = false;
    mcfg.outControl         = kCTIMER_Output_NoAction;
    mcfg.outPinInitState    = false;
    mcfg.enableInterrupt    = true;
    CTIMER_SetupMatch(CTIMER0, kCTIMER_Match_0, &mcfg);
    CTIMER_StartTimer(CTIMER0);

    /* Priority 5 == configMAX_SYSCALL_INTERRUPT_PRIORITY (5 << (8-3)); calling
     * the FromISR API from a higher-priority (numerically lower) vector trips
     * configASSERT. */
    NVIC_SetPriority(CTIMER0_IRQn, 5);
    NVIC_EnableIRQ(CTIMER0_IRQn);
}

/* Shutdown evidence: after >= 3 watcher callbacks, quit the loop and shut
 * nanosig down while the watcher is still registered (exercises the broker's
 * member drain on remove), then report the shutdown return code over RTT.
 * A non-zero rc, or a "shutdown: ... failed" log line, signals a leak. */
static void shutdown_task(void *p)
{
    (void)p;
    while(watcher_hits < 3u) {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    SEGGER_RTT_Write(0, "nanosig: 3 watcher hits, shutting down\n", 39);

    (void)ns_loop_quit(demo_loop);
    while(!loop_exited) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    int shutdown_rc = ns_shutdown();
    char msg[80];
    int len = snprintf(msg, sizeof(msg),
        "nanosig: ns_shutdown rc=%d isr_hits=%u\n",
        shutdown_rc, (unsigned)isr_hits);
    if(len > 0) {
        SEGGER_RTT_Write(0, msg, (unsigned)len);
    }
    vTaskDelete(NULL);
}

static void tick_task(void *p)
{
    (void)p;
    /* n counts half-second slots: one full second every 2 lines. */
    uint32_t n = 0;
    for (;;) {
        char msg[32];
        int len = snprintf(msg, sizeof(msg), "tick @%u\n", (unsigned)n);
        if (len > 0) {
            SEGGER_RTT_Write(0, msg, (unsigned)len);
        }
        vTaskDelay(pdMS_TO_TICKS(500));
        n++;
    }
}

static void led_task(void *p)
{
    (void)p;
    uint8_t v = 0;
    for (;;) {
        led_set(v ^= 0x01u);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

int main(void)
{
    init();
    SEGGER_RTT_Write(0, "freertos-demo\n", 13);

    xTaskCreate(tick_task, "tick", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
    xTaskCreate(led_task,  "led",  configMINIMAL_STACK_SIZE, NULL, 2, NULL);
    xTaskCreate(loop_task, "nsloop", configMINIMAL_STACK_SIZE * 8, NULL, 2, NULL);
    xTaskCreate(emit_task, "nsemit", configMINIMAL_STACK_SIZE * 4, NULL, 2, NULL);
    xTaskCreate(shutdown_task, "nsoff", configMINIMAL_STACK_SIZE * 2, NULL, 2, NULL);

    vTaskStartScheduler();

    for (;;) {
    }
    return 0;
}
