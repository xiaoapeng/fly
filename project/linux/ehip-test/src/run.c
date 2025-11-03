/**
 * @file run.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-11-03
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */
#include <eh.h>
#include <eh_event.h>
#include <eh_timer.h>
#include <eh_signal.h>
#include <eh_debug.h>
#include <eh_module.h>

#include <unistd.h>

void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    write(1, buf, size);
}


EH_DEFINE_STATIC_CUSTOM_SIGNAL(timer_signal, eh_event_timer_t, EH_TIMER_INIT(timer_signal.custom_event));

static void timer_slot_function(eh_event_t *e, void *slot_param){
    (void)slot_param;
    (void)e;
    static int cnt = 0;
    eh_infofl("timer slot function! cnt: %d", cnt++);
    if(cnt >= 5){
        eh_signal_dispatch_loop_request_quit();
    }
}
static EH_DEFINE_SLOT(timer_slot, timer_slot_function, NULL);

static int __init run_init(void){
    eh_debugfl("run init!");
    eh_timer_advanced_init(eh_signal_to_custom_event(&timer_signal), (eh_sclock_t)eh_msec_to_clock(1000*5), EH_TIMER_ATTR_AUTO_CIRCULATION);
    eh_signal_slot_connect(&timer_signal, &timer_slot);
    eh_timer_start(eh_signal_to_custom_event(&timer_signal));
    return 0;
}

static void __exit run_exit(void){
    eh_timer_stop(eh_signal_to_custom_event(&timer_signal));
    eh_signal_slot_disconnect(&timer_signal, &timer_slot);
}

eh_module_level8_export(run_init, run_exit);
