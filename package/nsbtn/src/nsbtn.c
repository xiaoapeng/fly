/**
 * @file nsbtn.c
 * @brief lwbtn 包装：共享单实例 + 静态按钮池 + 单一 nsbtn_event 信号
 *
 * 扫描链：周期 ns_timer（loop 线程）-> lwbtn_process_ex(now_ms)
 *        -> lwbtn 事件回调 -> nsbtn_event 信号（值拷贝 payload）。
 */
#include <autoconf.h>

#include <stdbool.h>
#include <stddef.h>

#include <nsbtn/nsbtn.h>

#include <nanosig/nanosig_port.h>
#include <nanosig/nanosig_timer.h>

#define NSBTN_MAX_BUTTONS ((uint16_t)CONFIG_PACKAGE_NSBTN_MAX_BUTTONS)
#define NSBTN_SCAN_PERIOD_US \
    ((ns_time_us_t)CONFIG_PACKAGE_NSBTN_SCAN_PERIOD_MS * 1000U)

/* 全局单例信号：定义在包内，声明（extern）在 nsbtn.h。 */
ns_signal_t nsbtn_event;

static ns_timer_t s_timer;
static ns_connection_t s_conn;
static lwbtn_t s_inst;
static lwbtn_btn_t s_btns[NSBTN_MAX_BUTTONS];
static nsbtn_button_t s_buttons[NSBTN_MAX_BUTTONS];
static uint16_t s_count;
static bool s_inited;
static bool s_running;

static lwbtn_time_t now_ms(void)
{
    ns_platform_time_us_t now_us = 0u;

    (void)ns_platform_clock_monotonic_us(&now_us);
    return (lwbtn_time_t)(now_us / 1000U);
}

/* lwbtn 回调只给出 btn，包内用线性查找取回句柄（池很小，避免指针差转换告警）。 */
static nsbtn_button_t *button_from_lwbtn(const lwbtn_btn_t *btn)
{
    uint16_t i;

    for (i = 0U; i < s_count; i++) {
        if (s_buttons[i].btn == btn) {
            return &s_buttons[i];
        }
    }
    return NULL;
}

static uint8_t on_get_state(struct lwbtn *lwobj, struct lwbtn_btn *btn)
{
    nsbtn_button_t *button = button_from_lwbtn(btn);

    (void)lwobj;
    if (button == NULL || button->get_state == NULL) {
        return 0U;
    }
    return button->get_state(button->user_arg);
}

static void on_event(struct lwbtn *lwobj, struct lwbtn_btn *btn, lwbtn_evt_t evt)
{
    nsbtn_button_t *button = button_from_lwbtn(btn);
    nsbtn_event_payload_t p = {0};

    (void)lwobj;
    if (button == NULL) {
        return;
    }

    p.handle = button;
    p.evt = evt;
    p.time_ms = now_ms();

    switch (evt) {
#if LWBTN_CFG_USE_CLICK
        case LWBTN_EVT_ONCLICK:
            p.click_count = lwbtn_click_get_count(btn);
            break;
#endif
#if LWBTN_CFG_USE_KEEPALIVE
        case LWBTN_EVT_KEEPALIVE:
            p.keepalive_count = lwbtn_keepalive_get_count(btn);
            break;
#endif
        default:
            break;
    }

    /* 队列满时丢事件（nanosig emit 非阻塞），与旧应用层胶水行为一致。 */
    (void)ns_signal_emit(nsbtn_event, &p);
}

static void on_poll(void *user_data, const ns_no_payload_t *payload)
{
    (void)user_data;
    (void)payload;

    /* timer 取消不撤销已入队的 slot 调用（nanosig 语义），停止后可能还有
     * 一拍到达；这里挡住，避免 scan_stop 之后又冒出事件。 */
    if (!s_running) {
        return;
    }

    (void)lwbtn_process_ex(&s_inst, now_ms());
}

/* 释放路径只保留第一个错误码，但绝不跳步。 */
static int keep_first(int rc, int sub)
{
    return (rc == NS_OK) ? sub : rc;
}

int nsbtn_init(ns_loop_t *loop)
{
    int rc;

    if (loop == NULL) {
        return NS_E_INVAL;
    }
    if (s_inited) {
        return NS_OK;
    }

    s_count = 0U;
    s_running = false;

    rc = ns_signal_init(&nsbtn_event, nsbtn_event_payload_t);
    if (rc != NS_OK) {
        return rc;
    }

    rc = ns_timer_init(&s_timer, NSBTN_SCAN_PERIOD_US, NS_TIMER_ATTR_REPEAT);
    if (rc != NS_OK) {
        (void)ns_signal_deinit(&nsbtn_event);
        return rc;
    }

    rc = ns_signal_connect_typed(s_timer.signal, on_poll, ns_no_payload_t,
                                 loop, NULL, &s_conn);
    if (rc != NS_OK) {
        (void)ns_timer_deinit(&s_timer);
        (void)ns_signal_deinit(&nsbtn_event);
        return rc;
    }

    s_inited = true;
    return NS_OK;
}

int nsbtn_deinit(void)
{
    int rc;

    if (!s_inited) {
        return NS_E_INVAL;
    }

    rc = nsbtn_scan_stop();
    rc = keep_first(rc, ns_signal_disconnect(&s_conn));
    rc = keep_first(rc, ns_timer_deinit(&s_timer));
    /* 契约：调用者必须先断开自己的 nsbtn_event 连接（同 nanosig 的 signal
     * 生命周期约定）。库不代劳，否则会越权改动用户的连接对象。 */
    rc = keep_first(rc, ns_signal_deinit(&nsbtn_event));

    s_inited = false;
    s_running = false;
    s_count = 0U;
    return rc;
}

nsbtn_button_t *nsbtn_register(nsbtn_get_state_fn get_state, void *user_arg)
{
    nsbtn_button_t *button;

    if (!s_inited || s_running || get_state == NULL) {
        return NULL;
    }
    if (s_count >= NSBTN_MAX_BUTTONS) {
        return NULL;
    }

    /* 只入池、不绑定：绑定统一在 nsbtn_scan_start() 里按 s_count 调
     * lwbtn_init_ex() 完成。运行态拒绝注册，避免“注册成功但不被扫描”。 */
    button = &s_buttons[s_count];
    button->btn = &s_btns[s_count];
    button->get_state = get_state;
    button->user_arg = user_arg;
    s_count++;

    return button;
}

int nsbtn_scan_start(void)
{
    int rc;

    if (!s_inited) {
        return NS_E_INVAL;
    }
    if (s_running) {
        return NS_OK;
    }
    if (s_count == 0U) {
        return NS_E_INVAL;
    }

    /* 绑定点：用当前 s_count 重新初始化共享实例，停止后新增的按钮在此生效。
     * 停止态重绑定是安全的：lwbtn_init_ex 会 memset 实例与按钮数组。
     * 返回 0 表示参数非法（上游约定），必须检查，否则永远不会有事件。 */
    if (lwbtn_init_ex(&s_inst, s_btns, s_count, on_get_state, on_event) == 0U) {
        return NS_E_INVAL;
    }

    rc = ns_timer_start(&s_timer);
    if (rc != NS_OK) {
        return rc;
    }

    s_running = true;
    return NS_OK;
}

int nsbtn_scan_stop(void)
{
    int rc;

    if (!s_inited) {
        return NS_E_INVAL;
    }
    if (!s_running) {
        return NS_OK;
    }

    rc = ns_timer_cancel(&s_timer);
    if (rc == NS_OK) {
        s_running = false;
    }
    return rc;
}
