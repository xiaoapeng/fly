/**
 * @file task_main.c
 * @brief 主要任务
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-06-30
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <stdio.h>
#include <stdarg.h>
#include "SEGGER_RTT.h"
#include "eh.h"
#include "eh_co.h"
#include "eh_debug.h"
#include "eh_mem.h"
#include "eh_event.h"
#include "eh_event_cb.h"
#include "eh_signal.h"
#include "eh_sleep.h"
#include "eh_platform.h"
#include "eh_timer.h"
#include "eh_sleep.h"
#include "eh_types.h"
#include "system_MCXN947_cm33_core0.h"

#include "fsl_lpuart.h"

#include "button.h"
#include "led.h"

void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    //LPUART_WriteBlocking(LPUART4, buf, size);
    SEGGER_RTT_Write(0, buf, size);
}

EH_DEFINE_STATIC_CUSTOM_SIGNAL(timer_signal, eh_timer_event_t, EH_TIMER_INIT(timer_signal.custom_event));

#define TEST_CNT        500 
int task_run(void *parameters){
    // float a = 0.0f;
    eh_timer_event_t timer1;
    eh_timer_init(&timer1);
    eh_timer_config_interval(&timer1, eh_usec_to_clock(1000));
    eh_timer_set_attr(&timer1, EH_TIMER_ATTR_AUTO_CIRCULATION);

    eh_timer_start(&timer1);
    eh_clock_t *test_clock = eh_malloc(sizeof(eh_clock_t)*TEST_CNT);

    for(int i=0; i<TEST_CNT; i++){
        test_clock[i] = eh_get_clock_monotonic_time();
        eh_task_yield();
    }
    eh_clock_t diff_sum = 0;
    double avg_diff = 0.0;
    for(int i=0; i<TEST_CNT-1; i++){
        diff_sum += test_clock[i+1] - test_clock[i]; 
        //eh_debugln("test_clock[%d] = %u", i, test_clock[i]);
    }
    avg_diff = (double)diff_sum;
    avg_diff = avg_diff/(TEST_CNT-1)/2;
    eh_debugfl("%s: avg_diff=%f",parameters, avg_diff);
    eh_free(test_clock);
    eh_timer_clean(&timer1);
    return 0;
}

int task_auto_destroy_test0(void *parameters){
    eh_infoln("auto destroy task %s", parameters);
    return 0;
}

int task_auto_destroy_test1(void *parameters){
    eh_infoln("auto destroy task %s", parameters);
    eh_usleep(1000*1000*4);
    eh_infoln("auto destroy task %s", parameters);
    return 0;
}



void sw3_slot_function(eh_event_t *e, void *slot_param){
    (void)slot_param;
    (void)e;
    static int cnt = 0;
    uint32_t sta = button_sw3_sta();
    led_test_out_set_val((uint8_t)sta);
    
    eh_infoln("sw3 %s", sta ? "up":"down");
    if(sta == 0){
        cnt++;
        switch (cnt%3) {
            case 0:
                led_rgb_r_set_val(1);
                break;
            case 1:
                led_rgb_g_set_val(1);
                break;
            case 2:
                led_rgb_b_set_val(1);
                break;
            default:
                break;
        }
    }else{
        led_rgb_r_set_val(0);
        led_rgb_g_set_val(0);
        led_rgb_b_set_val(0);
    }
}

void timer_slot_function(eh_event_t *e, void *slot_param){
    (void)slot_param;
    (void)e;
    // static uint8_t sta = 0;
    // led_test_out_set_val(sta);
    // sta = !sta;
}

int task_switch_test(void *parameters){
    (void)parameters;
    while(1){
        led_test_out_set_val((uint8_t)(uint32_t)parameters);
        eh_task_yield();
    }
    return 0;
}



int task_main(void)
{
    //int ret;
    EH_DEFINE_SLOT(sw3_slot, sw3_slot_function, NULL);
    EH_DEFINE_SLOT(timer_slot, timer_slot_function, NULL);

    eh_infofl("start! 1234");
    
    eh_timer_advanced_init(eh_signal_to_custom_event(&timer_signal), (eh_sclock_t)eh_usec_to_clock(500), EH_TIMER_ATTR_AUTO_CIRCULATION);
    
    eh_signal_register(&timer_signal);
    eh_signal_slot_connect(&timer_signal, &timer_slot);
    eh_signal_slot_connect(&button_sw3_signal, &sw3_slot);

    eh_timer_start(eh_signal_to_custom_event(&timer_signal));

    // eh_task_create("test_switch0", EH_TASK_FLAGS_DETACH, 1024, (void*)1, task_switch_test);
    // eh_task_create("test_switch1", EH_TASK_FLAGS_DETACH, 1024, (void*)0, task_switch_test);
    while(1){
        eh_infofl("run!!!");
        __await eh_usleep(1000*1000*10);
    }
    
    eh_signal_slot_disconnect(&sw3_slot);
    eh_signal_slot_disconnect(&timer_slot);
    eh_signal_unregister(&timer_signal);
    return 0;
}
