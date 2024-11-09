/**
 * @file loop_pool.h
 * @brief while(1)中定时循环的实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-04-07
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */


#ifndef _LOOP_POOL_H_
#define _LOOP_POOL_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#include "typedef.h"

#define _looppool_get_tickms() GET_TICK()


typedef enum LoopPoolTimerSta{
    LPTS_IDIE,              /* 空闲状态 */
    LPTS_WORK,              /* 工作状态 */
}LoopPoolTimerSta;

typedef struct _LoopPoolTimer{
    LoopPoolTimerSta sta;
    uint32_t start_time;
    uint32_t time_out;
}LoopPoolTimer;

/**
 * @brief  配置一个轮询式定时器
 * @param  t                句柄
 * @param  timeout          超时时间
 */
static inline void LoopPoolTimer_Config(LoopPoolTimer* t, uint32_t timeout){
    t->sta = LPTS_IDIE;
    t->time_out = timeout;
    t->start_time = 0;
}

/**
 * @brief  重新开始进行计时
 * @param  t                句柄
 */
static inline void LoopPoolTimer_Reset(LoopPoolTimer* t){
    t->sta = LPTS_WORK;
    t->start_time = _looppool_get_tickms();
}

/**
 * @brief  是否超时
 * @param  t                句柄
 * @return int 
 */
static inline int LoopPoolTimer_IsTimeout(LoopPoolTimer* t){
    if(t->sta != LPTS_WORK) return 0;
    return (_looppool_get_tickms() - t->start_time) >= t->time_out;
}

/**
 * @brief  停止计时
 * @param  t                句柄
 * @return int 
 */
static inline void LoopPoolTimer_Stop(LoopPoolTimer* t){
    t->sta = LPTS_IDIE;
}

/**
 * @brief  返回工作状态
 * @param  t                句柄
 * @return int 
 */
static inline LoopPoolTimerSta LoopPoolTimer_GetSta(LoopPoolTimer* t){
    return t->sta;
}

/**
 * @brief 强行使定时器超时
 * @param  t                句柄
 */
static inline void LoopPoolTimer_ForceTimeout(LoopPoolTimer* t){
    t->sta = LPTS_WORK;
    t->start_time = _looppool_get_tickms() - t->time_out;
}

/**
 * @brief 在循环中进行定时调用
 * @param phase_ms          相位，第一次运行将会偏相，此值应该小于等于cycle_ms，当为0时与LOOPPOOL_CALL_MS一样
 * @param cycle_ms          定时时间
 * @param action            需要被定时调用的代码块
 */
#define LOOPPOOL_PHASE_CALL_MS(phase_ms, cycle_ms, action) ({      \
        static uint32_t __looppool_last_time = 0;   \
        uint32_t __looppool_current_time = _looppool_get_tickms();\
        uint32_t __looppool_diff_time;\
        __looppool_last_time = __looppool_last_time == 0 ? __looppool_current_time-cycle_ms+phase_ms : __looppool_last_time;  \
        __looppool_diff_time = __looppool_current_time - __looppool_last_time;\
        if(cycle_ms == 0 || (__looppool_diff_time >= cycle_ms)) {   \
            if(cycle_ms)                                            \
                __looppool_last_time += __looppool_diff_time/cycle_ms*cycle_ms;   \
            {action;} \
        }\
    })


/**
 * @brief 在循环中进行定时调用
 * @param cycle_ms          定时时间
 * @param action            需要被定时调用的代码块
 */
#define LOOPPOOL_CALL_MS(cycle_ms, action) LOOPPOOL_PHASE_CALL_MS(0, cycle_ms, action)


/**
 * @brief 在循环中调用，但只运行一次，一次只有再也不允许
 */
#define LOOPPOOL_ONCE_CALL(action) ({              \
        static uint32_t __looppool_is_run_once = 1;     \
        if(__looppool_is_run_once){                     \
            __looppool_is_run_once = 0;                 \
            action;                                     \
        }                                               \
    })


static __attribute__ ((__used__)) int __looppool_bool_debounce(uint32_t debounce_ms, int new_bool_state, uint32_t *last_time, 
                                        int *last_state, int *last_last_state, uint32_t *last_call_time){
    if(debounce_ms == 0) return new_bool_state;
    if(*last_state == -1 || (_looppool_get_tickms() - *last_call_time) > (debounce_ms/2))
    {
        *last_time = _looppool_get_tickms();
        *last_call_time = _looppool_get_tickms();
        *last_last_state = *last_state = new_bool_state;
        return new_bool_state;
    }
    *last_call_time = _looppool_get_tickms();
    if(*last_state == new_bool_state){
        if((_looppool_get_tickms() - *last_time) > debounce_ms){
            *last_last_state = *last_state;
        }
        return *last_last_state;
    }
    *last_time = _looppool_get_tickms();
    *last_state = !(*last_state);
    return *last_last_state;
}

/**
 * @brief 自动防抖，此宏必须要被定时调用，且调用频率必须远大于防抖的时间
          否则会产生bug
 * @param debounce_ms       防抖时间
 * @param current_bool      当前bool值
 */
#define LOOPPOOL_BOOL_DEBOUNCE(debounce_ms, current_bool)       ({\
    static uint32_t __last_time__ = 0;                                  \
    static int      __last_state__ = -1;                                \
    static int      __last_last_state__ = 0;                            \
    static uint32_t __last_call_time__ = 0;                             \
    __looppool_bool_debounce(debounce_ms, !!(current_bool), &__last_time__, &__last_state__, &__last_last_state__, &__last_call_time__);  \
})

/**
 * @brief 对于任意一个变量，若前一次调用和后一次调用值发生了改变则调用 action
 */
#define LOOPPOOL_TYPE_VARIABLE_CHANGE_CALL(type, variable_init, variable, action) ({      \
        static type _last_variable_ = (variable_init);                          \
        type _variable_tmp = (variable);                                        \
        if(_variable_tmp != _last_variable_){                                   \
            _last_variable_ = _variable_tmp;                                    \
            action;                                                             \
        }                                                                       \
    })


/**
 * @brief 对于一个U32的变量，若前一次调用和后一次调用值发生了改变则调用 action
 */
#define LOOPPOOL_U32_VARIABLE_CHANGE_CALL(variable_init, variable, action) \
    LOOPPOOL_TYPE_VARIABLE_CHANGE_CALL(uint32_t, variable_init, variable, action)


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _LOOP_POOL_H_


