#include <eh.h>
#include <eh_event.h>
#include <eh_timer.h>
#include <eh_signal.h>
#include <eh_sleep.h>
#include <eh_debug.h>
#include <eh_mem.h>

#include "SEGGER_RTT.h"

void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    SEGGER_RTT_Write(0, buf, size);
}
EH_DEFINE_STATIC_CUSTOM_SIGNAL(timer_signal, eh_event_timer_t, EH_TIMER_INIT(timer_signal.custom_event));

static eh_clock_t s_timer_clock_interval;
static eh_clock_t s_last_idle_time = 0;
static void timer_slot_function(eh_event_t *e, void *slot_param){
    (void)slot_param;
    (void)e;
    eh_clock_t idle_now = eh_task_idle_time();
    eh_clock_t idle_diff = idle_now  - s_last_idle_time;
    struct eh_mem_heap_info heap_info;
    double cpu_usage, mem_usage;
    s_last_idle_time = idle_now;

    // static uint8_t sta = 0;
    // led_test_out_set_val(sta);
    // sta = !sta;

    cpu_usage = (double)idle_diff / (double)s_timer_clock_interval;
    cpu_usage = 1.0 - cpu_usage;
    eh_mem_get_heap_info(&heap_info);
    mem_usage = (double)(heap_info.total_size - heap_info.free_size)/(double)heap_info.total_size;
    eh_infofl("run!!! cpu_usage: %.2f%% mem_usage:%.2f%% mem_free: %d mem_total: %d", 
        cpu_usage * 100.0, 
        mem_usage * 100.0, heap_info.free_size, heap_info.total_size );
}

static EH_DEFINE_SLOT(timer_slot, timer_slot_function, NULL);

int main(void){
    

    EH_DBG_ERROR_EXEC(eh_global_init() != 0 , goto error);

    /* 初始化一个定时器信号，并连接信号与槽 */
    s_timer_clock_interval = eh_msec_to_clock(1000*5);
    eh_timer_advanced_init(eh_signal_to_custom_event(&timer_signal), (eh_sclock_t)s_timer_clock_interval, EH_TIMER_ATTR_AUTO_CIRCULATION);

    eh_signal_slot_connect(&timer_signal, &timer_slot);
    eh_timer_start(eh_signal_to_custom_event(&timer_signal));

    eh_signal_dispatch_loop();

    eh_timer_stop(eh_signal_to_custom_event(&timer_signal));
    eh_signal_slot_disconnect(&timer_signal, &timer_slot);

    eh_global_exit();
    eh_debugfl("exit!!");
error:
    while(1){    }
    return 0;
}