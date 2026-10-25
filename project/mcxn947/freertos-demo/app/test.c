/**
 * @file test.c
 * @brief Demo 业务：按键驱动灯效（SW1 换闪烁颜色、双击换频率、SW2 总开关）
 *
 * 输入全部来自 nsbtn_event（单击/双击判定由 lwbtn 的连击窗口完成）：
 *   - SW1 单击        -> 闪烁颜色循环：红/绿/蓝/黄/品红/青/白
 *   - SW1 双击        -> 闪烁频率循环：1 / 2 / 5 Hz
 *   - SW2 单击        -> 总开关：关=全灭，开=恢复当前颜色/频率继续闪
 * 上电默认：使能 + 红色 1Hz 闪烁。
 *
 * 闪烁用固定 10ms repeat timer + 软件分频，换频率只改阈值，不重建 timer。
 */
#include "test.h"

#include "key.h"
#include "led.h"
#include "runtime.h"

#include <stdbool.h>

#include <nanosig/nanosig.h>
#include <nanosig/ns_debug.h>
#include <nanosig/nanosig_port.h>
#include <nanosig/nanosig_signal.h>
#include <nanosig/nanosig_timer.h>

#define BLINK_TICK_US 10000u

/* 本 demo 的输入全部来自单击/双击；关掉 lwbtn 的 click 支持会让功能静默失效。 */
#if !LWBTN_CFG_USE_CLICK
#error "LED demo requires CONFIG_PACKAGE_LWBTN_USE_CLICK=y"
#endif

/* 单色 -> 混色；bit0=红, bit1=绿, bit2=蓝（见 board/led.c） */
static const uint8_t s_colors[] = {
    0x01u, /* 红 */
    0x02u, /* 绿 */
    0x04u, /* 蓝 */
    0x03u, /* 黄   = 红 + 绿 */
    0x05u, /* 品红 = 红 + 蓝 */
    0x06u, /* 青   = 绿 + 蓝 */
    0x07u, /* 白   = 红 + 绿 + 蓝 */
};

/* 半周期（单位：10ms tick）：500ms / 250ms / 100ms -> 1 / 2 / 5 Hz */
static const uint16_t s_half_ticks[] = { 50u, 25u, 10u };

#define LED_COLOR_COUNT ((uint8_t)(sizeof(s_colors) / sizeof(s_colors[0])))
#define LED_FREQ_COUNT ((uint8_t)(sizeof(s_half_ticks) / sizeof(s_half_ticks[0])))

static ns_timer_t blink_timer;
static ns_connection_t blink_conn;
static ns_connection_t key_conn;

static bool s_enabled = true;
static bool s_on_phase;
static uint8_t s_color;
static uint8_t s_freq;
static uint16_t s_tick;

/* 按当前状态刷新灯与 timer（关=灭灯停表；开=立即点亮并重启节奏）。 */
static int blink_apply(void)
{
    s_tick = 0u;

    if (!s_enabled) {
        (void)ns_timer_cancel(&blink_timer);
        s_on_phase = false;
        led_set(0u);
        return NS_OK;
    }

    s_on_phase = true;
    led_set(s_colors[s_color]);
    return ns_timer_restart(&blink_timer);
}

/* 10ms 一拍；累计到半周期就翻转一次，翻转时才改灯。 */
static void on_blink_tick(void *user_data, const ns_no_payload_t *payload)
{
    (void)user_data;
    (void)payload;

    if (!s_enabled) {
        return;
    }

    s_tick++;
    if (s_tick < s_half_ticks[s_freq]) {
        return;
    }

    s_tick = 0u;
    s_on_phase = !s_on_phase;
    led_set(s_on_phase ? s_colors[s_color] : 0u);
}

static const char *evt_name(lwbtn_evt_t evt)
{
    switch(evt) {
        case LWBTN_EVT_ONPRESS:
            return "ONPRESS";
        case LWBTN_EVT_ONRELEASE:
            return "ONRELEASE";
#if LWBTN_CFG_USE_CLICK
        case LWBTN_EVT_ONCLICK:
            return "ONCLICK";
#endif
#if LWBTN_CFG_USE_KEEPALIVE
        case LWBTN_EVT_KEEPALIVE:
            return "KEEPALIVE";
#endif
        default:
            return "?";
    }
}

/* Runs in the main loop task, dispatched from nsbtn_event. */
static void on_key_event(void *user_data, const nsbtn_event_payload_t *payload)
{
    const key_t *key = (const key_t *)payload->handle->user_arg;

    (void)user_data;

    ns_sysln("key: key=%s evt=%s clicks=%u keepalive=%u t=%lums",
             key->name, evt_name(payload->evt),
             (unsigned)payload->click_count,
             (unsigned)payload->keepalive_count,
             (unsigned long)payload->time_ms);

#if LWBTN_CFG_USE_CLICK
    if (payload->evt != LWBTN_EVT_ONCLICK) {
        return;
    }

    if (key == &key_sw2 && payload->click_count == 1u) {
        s_enabled = !s_enabled;
    } else if (key == &key_sw1 && payload->click_count == 1u) {
        s_color = (uint8_t)((s_color + 1u) % LED_COLOR_COUNT);
    } else if (key == &key_sw1 && payload->click_count == 2u) {
        s_freq = (uint8_t)((s_freq + 1u) % LED_FREQ_COUNT);
    } else {
        return; /* 三连击及以上不响应 */
    }

    (void)blink_apply();
    ns_sysln("led: en=%u color=%u freq=%u",
             (unsigned)s_enabled, (unsigned)s_color, (unsigned)s_freq);
#endif
}

int test_init(void)
{
    ns_loop_t *loop = runtime_get();
    int rc;

    if(loop == NULL) {
        rc = NS_E_INVAL;
        goto out;
    }

    rc = ns_signal_connect_typed(nsbtn_event, on_key_event, nsbtn_event_payload_t,
                                 loop, NULL, &key_conn);
    if(rc != NS_OK) {
        goto out;
    }

    rc = ns_timer_init(&blink_timer, BLINK_TICK_US, NS_TIMER_ATTR_REPEAT);
    if(rc != NS_OK) {
        goto out_key_conn;
    }

    rc = ns_signal_connect_typed(blink_timer.signal, on_blink_tick, ns_no_payload_t,
                                 loop, NULL, &blink_conn);
    if(rc != NS_OK) {
        goto out_timer;
    }

    /* 上电即"开灯"：使能 + 红色 1Hz 闪烁。 */
    rc = blink_apply();
    if(rc != NS_OK) {
        goto out_blink_conn;
    }

    ns_sysln("led: en=1 color=0 freq=0 (boot)");
    return NS_OK;

out_blink_conn:
    (void)ns_signal_disconnect(&blink_conn);
out_timer:
    (void)ns_timer_deinit(&blink_timer);
out_key_conn:
    (void)ns_signal_disconnect(&key_conn);
out:
    return rc;
}

int test_deinit(void)
{
    int rc;

    /* 先摘业务自己的连接，key_deinit() 之后才能安全 deinit nsbtn_event。 */
    rc = ns_signal_disconnect(&key_conn);
    rc = (rc == NS_OK) ? ns_timer_cancel(&blink_timer) : rc;
    rc = (rc == NS_OK) ? ns_signal_disconnect(&blink_conn) : rc;
    rc = (rc == NS_OK) ? ns_timer_deinit(&blink_timer) : rc;
    led_set(0u);

    return rc;
}
