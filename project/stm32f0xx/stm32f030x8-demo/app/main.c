
#include "SEGGER_RTT.h"
#include <eh.h>
#include <eh_sleep.h>
#include <eh_debug.h>
#include <eh_signal.h>
#include <eh_timer.h>
#include "led.h"

extern int init(void);

void stdout_write(void *stream, const uint8_t *buf, size_t size){
    (void)stream;
    SEGGER_RTT_Write(0, buf, size);
}

void task_main(void);

int main(void)
{
	init();
	eh_debugln("hello world!");
	
	
	eh_global_init();
	task_main();
	eh_global_exit();

	
	eh_debugln("hello world! end!");
	while(1){

	}
}


EH_DEFINE_STATIC_CUSTOM_SIGNAL(
	signal_1000ms_timer, 
	eh_timer_event_t, 
	{}
);

void slot_1000ms_timer(eh_event_t *e, void *slot_param){
	(void) e;
	
	if((*(int*)slot_param)%2)
		led_gpio_off();
	else
		led_gpio_on();

	eh_debugln("slot_1000ms_timer %d", (*(int*)slot_param)++);
}
int slot_1000ms_param = 0;
EH_DEFINE_SLOT(slot_1000ms, slot_1000ms_timer, &slot_1000ms_param);



int task_switch_on_test(void *parameters){
    (void)parameters;
    while(1){
		led_gpio_on();
        eh_task_yield();
    }
    return 0;
}

int task_switch_off_test(void *parameters){
    (void)parameters;
    while(1){
		led_gpio_off();
        eh_task_yield();
    }
    return 0;
}


void task_main(void){
	eh_timer_advanced_init(
        eh_signal_to_custom_event(&signal_1000ms_timer), 
        (eh_sclock_t)eh_msec_to_clock(1000), 
        EH_TIMER_ATTR_AUTO_CIRCULATION
    );
	eh_signal_register(&signal_1000ms_timer);
	eh_signal_slot_connect(&signal_1000ms_timer, &slot_1000ms);
	eh_timer_start(eh_signal_to_custom_event(&signal_1000ms_timer));

	// eh_task_create("test_switch0", EH_TASK_FLAGS_DETACH, 512, NULL, task_switch_on_test);
	// eh_task_create("test_switch1", EH_TASK_FLAGS_DETACH, 512, NULL, task_switch_off_test);

	while(1){
		eh_usleep(1000*1000*60);
	}

	eh_timer_stop(eh_signal_to_custom_event(&signal_1000ms_timer));
    eh_signal_slot_disconnect(&slot_1000ms);
    eh_signal_unregister(&signal_1000ms_timer);
    eh_signal_clean(&signal_1000ms_timer);
}

