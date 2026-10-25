/**
 * @file nsbtn.h
 * @brief lwbtn 按钮管理器 -> 单一 nanosig 信号（nsbtn_event）的包装包
 *
 * 所有已注册按钮共享一个 lwbtn 实例；包内用静态池保存按钮与句柄。
 *
 * 调用契约（顺序即约定，包内零锁）：
 *   nsbtn_init(loop)
 *     -> nsbtn_register(...)  x N       （只能在此处增；池满返回 NULL）
 *     -> nsbtn_scan_start()             （绑定实例 + 启动周期定时器）
 *     -> 订阅 nsbtn_event 处理按键事件
 *     -> nsbtn_scan_stop()              （停定时器；之后才允许再 register）
 *     -> 断开自己的 nsbtn_event 订阅     （调用者责任，库不代劳）
 *     -> nsbtn_deinit()                 （之后全部句柄失效）
 *
 * 线程契约：所有 nsbtn_* 只能在 nsbtn_init() 传入的 loop 所在线程调用
 * （包括信号槽内）。ISR 与其它线程均不支持。
 *
 * 状态契约：nsbtn_scan_start() 会重新初始化 lwbtn 实例与按钮数组
 * （lwbtn 上游行为：memset lwobj 与 btns[]），因此通过 handle->btn 直接写入
 * lwbtn 的属性（如 btn->arg、动态时间参数）会在下一次 scan_start 后丢失。
 * 透传句柄请只用于查询/只读配置。
 */
#ifndef NSBTN_HDR_H
#define NSBTN_HDR_H

#include <stdint.h>

#include <lwbtn/lwbtn.h>
#include <nanosig/nanosig.h>
#include <nanosig/nanosig_loop.h>
#include <nanosig/nanosig_signal.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 注册句柄。透明结构体（定义见下），用户可直接读 handle->btn 调 lwbtn API。 */
typedef struct nsbtn_button nsbtn_button_t;

/* 按键状态回调：1 = 按下（active），0 = 松开；user_arg 为注册时传入的指针。 */
typedef uint8_t (*nsbtn_get_state_fn)(void *user_arg);

/* nsbtn_event 的 payload：字段与 lwbtn 事件能力对齐。 */
typedef struct {
    nsbtn_button_t *handle;          /* 身份：nsbtn_register() 返回的句柄 */
    lwbtn_evt_t evt;                 /* ONPRESS / ONRELEASE / ONCLICK / KEEPALIVE */
    uint8_t click_count;             /* 仅 ONCLICK 非 0 */
    uint16_t keepalive_count;        /* 仅 KEEPALIVE 非 0 */
    lwbtn_time_t time_ms;            /* 事件时刻（单调毫秒） */
} nsbtn_event_payload_t;

/* 所有按钮共用同一个事件信号，按钮身份在 payload->handle 里区分。 */
NS_SIGNAL_DECLARE(nsbtn_event);
NS_DEFINE_SLOT(nsbtn_event_slot, nsbtn_event_payload_t);

/**
 * @brief 按键句柄（透明）：btn 指向包内连续数组，可直接用于 lwbtn API。
 */
struct nsbtn_button {
    lwbtn_btn_t *btn;                /* 透传：lwbtn 按钮对象（只读用途） */
    nsbtn_get_state_fn get_state;    /* 注册时提供的状态回调 */
    void *user_arg;                  /* 注册时提供的用户参数 */
};

/**
 * @brief 初始化包：保存 loop，创建信号与周期定时器（不启动扫描）。
 * @param loop 调用者持有的 nanosig loop，必须非 NULL
 * @return NS_OK / NS_E_INVAL / 其它 nanosig 错误码
 */
int nsbtn_init(ns_loop_t *loop);

/**
 * @brief 停扫描、拆定时器/连接/信号，复位按钮池。
 *
 * 调用者契约：**必须先断开自己所有 nsbtn_event 订阅**（`ns_signal_disconnect`），
 * 再调用本函数——与 nanosig 的 signal 生命周期约定一致，库不代劳。
 * 之后全部句柄失效；再次使用前必须重新 init + register + connect。
 *
 * @return NS_OK 或首个错误码
 */
int nsbtn_deinit(void);

/**
 * @brief 注册一个按钮。
 *
 * 与 nsbtn_scan_start() 的关系：
 *   - register 只把按钮放进池子，**不会**让 lwbtn 立刻看到它；
 *   - 真正生效是在下一次 nsbtn_scan_start()：那里用当时的按钮数调用
 *     lwbtn_init_ex() 完成绑定；
 *   - 因此 register 只在扫描停止态合法（init 后、scan_start 前，或 scan_stop 后），
 *     扫描运行中调用返回 NULL；无 unregister，句柄终身有效至 deinit。
 *
 * @param get_state 状态回调（非 NULL）
 * @param user_arg 传给回调的用户参数
 * @return 句柄；未 init、扫描运行中或池满时返回 NULL
 */
nsbtn_button_t *nsbtn_register(nsbtn_get_state_fn get_state, void *user_arg);

/**
 * @brief 启动扫描：把当前已注册按钮绑定到共享 lwbtn 实例并启动周期定时器。幂等。
 *
 * 与 nsbtn_register() 的关系：
 *   - 绑定发生在**本函数**：以此刻的按钮数调用 lwbtn_init_ex()，此后再 register
 *     的按钮不会被本实例处理；
 *   - 停止（nsbtn_scan_stop）后追加注册的按钮，要在**下一次** scan_start 才生效；
 *   - 没有注册任何按钮时返回 NS_E_INVAL。
 *
 * @return NS_OK / NS_E_INVAL（未 init 或没有按钮）/ 定时器错误码
 */
int nsbtn_scan_start(void);

/**
 * @brief 停止扫描（取消定时器）。幂等；停止后才允许再注册。
 *
 * 与 nsbtn_register() 的关系：停止只是关掉定时器，lwbtn 实例保持已绑定状态；
 * 停止期间新注册的按钮要等下一次 nsbtn_scan_start() 重新绑定后才参与扫描。
 *
 * @return NS_OK / NS_E_INVAL（未 init）
 */
int nsbtn_scan_stop(void);

#ifdef __cplusplus
}
#endif

#endif /* NSBTN_HDR_H */
