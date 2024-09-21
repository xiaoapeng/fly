#include "SEGGER_RTT.h"
#include "eh.h"
#include "eh_event.h"
#include "eh_sleep.h"
#include "eh_timer.h"
#include "eh_debug.h"
#include "eh_platform.h"
#include "eh_signal.h"


#include "n200_func.h"
#include "system_gd32vf103.h"
#include "led.h"
#include "button.h"

extern int init(void);

EH_DEFINE_STATIC_CUSTOM_SIGNAL(timer_1000ms_signal, 
    eh_timer_event_t, 
    {}
);

static void slot_test_function(eh_event_t *e, void *slot_param){
    (void) e;
    led_toggle();
    eh_infoln("slot_test_function %s", (char *)slot_param);
}

/* 定义第一个槽，一个槽同时只能被一个信号所连接 */
EH_DEFINE_SLOT(
    test_signal_timer_slot, 
    slot_test_function,
    "timer-1000ms" /* 用户参数 */
);

/* 定义按键的回调槽 */
EH_DEFINE_SLOT(
    test_signal_button_pb5_slot, 
    slot_test_function,
    "button-pb5" /* 用户参数 */
);

/* 定义按键的回调槽 */
EH_DEFINE_SLOT(
    test_signal_button_pa0_slot, 
    slot_test_function,
    "button-pa0" /* 用户参数 */
);


void run(void){

    eh_signal_slot_connect(&button_pb5_signal, &test_signal_button_pb5_slot);
    eh_signal_slot_connect(&button_pa0_signal, &test_signal_button_pa0_slot);

    eh_timer_advanced_init(
        eh_signal_to_custom_event(&timer_1000ms_signal), 
        (eh_sclock_t)eh_msec_to_clock(1000), 
        EH_TIMER_ATTR_AUTO_CIRCULATION
    );
    
    /* 然后在注册信号 */
    eh_signal_register(&timer_1000ms_signal);
    
    /* 连接信号和槽 */
    eh_signal_slot_connect(&timer_1000ms_signal, &test_signal_timer_slot);
    
    /* 启动定时器 */
    eh_timer_start(eh_signal_to_custom_event(&timer_1000ms_signal));
    

    while(1){
        eh_usleep(1000*1000*20);
    }


    eh_timer_stop(eh_signal_to_custom_event(&timer_1000ms_signal));
    eh_signal_slot_disconnect(&test_signal_timer_slot);

    eh_signal_unregister(&timer_1000ms_signal);

    eh_signal_clean(&timer_1000ms_signal);

    eh_signal_slot_disconnect(&test_signal_button_pa0_slot);
    eh_signal_slot_disconnect(&test_signal_button_pb5_slot);

}






int main(void){
    init();
    // while( eh_clock_to_msec(eh_get_clock_monotonic_time()) < 10000 ){
        
    // }
    eh_infofl("app start!");
    eh_global_init();
    
    run();
    eh_infofl("run exit!!");

    eh_global_exit();
    
    eh_infofl("app exit!! ");

    while(1){

    }

    return 0;
}