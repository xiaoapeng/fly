/**
 * @file ping_test.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-04-12
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */



#include <eh.h>
#include <eh_error.h>
#include <eh_swab.h>
#include <eh_debug.h>
#include <eh_signal.h>
#include <eh_module.h>
#include <eh_timer.h>
#include <ehip-ipv4/ping.h>


static ping_pcb_t ping_pcb_test0;
EH_DEFINE_STATIC_CUSTOM_SIGNAL(ping_sender_timer_signal, eh_event_timer_t, EH_TIMER_INIT(ping_sender_timer_signal.custom_event));

static void ping_sender_timer_handler(eh_event_t *e, void *slot_param){
    (void)e;
    int ret;
    ping_pcb_t pcb = *((ping_pcb_t*)slot_param);
    eh_mdebugfl(PING_TEST, "%p ping_sender_timer_handler", pcb);
    if(ehip_ping_has_active_request(pcb))
        return ;

    eh_mdebugfl(PING_TEST, "%p ping request:", pcb);
    ret = ehip_ping_request(pcb, 200);
    if(ret < 0){
        eh_errln("%p ehip_ping_request failed ret %d", pcb, ret);
    }
}

static EH_DEFINE_SLOT(slot_ping_sender_timer_test0, ping_sender_timer_handler, &ping_pcb_test0);

void ping_error_callback(ping_pcb_t pcb, ipv4_addr_t addr, uint16_t seq, int erron){
    int ret;
    eh_errfl("%p ping_error_callback addr: "IPV4_FORMATIO", seq: %d, erron: %d",
        pcb, ipv4_formatio(addr), seq, erron);
    eh_mdebugfl(PING_TEST, "%p ping request:", pcb);
    ret = ehip_ping_request(pcb, 200);
    if(ret < 0){
        eh_errfl("%p ehip_ping_request failed %d", pcb, ret);
    } 
}

void ping_response_callback(ping_pcb_t pcb, ipv4_addr_t addr, uint16_t seq, uint8_t ttl, eh_clock_t time){
    (void) pcb;
    eh_minfofl(PING_TEST, "ping response from addr: "IPV4_FORMATIO", seq=%d, ttl=%d, time=%.3fms",
        ipv4_formatio(addr), seq, ttl, ((double)eh_clock_to_usec(time))/1000.0);
}

static int __init ping_test_init(void)
{
    int ret;
    ping_pcb_test0 = ehip_ping_any_new(ipv4_make_addr(192,168,12,89));
    if(eh_ptr_to_error(ping_pcb_test0) < 0){
        eh_errln("ping_pcb create failed %d", eh_ptr_to_error(ping_pcb_test0));
        return eh_ptr_to_error(ping_pcb_test0);
    }

    ehip_ping_set_error_callback(ping_pcb_test0, ping_error_callback);
    ehip_ping_set_response_callback(ping_pcb_test0, ping_response_callback);
    ehip_ping_set_timeout(ping_pcb_test0, 30); // 30 * 100ms = 3s


    eh_timer_advanced_init(
        eh_signal_to_custom_event(&ping_sender_timer_signal), 
        (eh_sclock_t)eh_msec_to_clock(1000*1), 
        EH_TIMER_ATTR_AUTO_CIRCULATION);
    
    ret = eh_signal_register(&ping_sender_timer_signal);
    if(ret < 0) goto eh_signal_register_error;
    ret = eh_timer_start(eh_signal_to_custom_event(&ping_sender_timer_signal));
    if(ret < 0) goto eh_timer_start_error;
    eh_signal_slot_connect(&ping_sender_timer_signal, &slot_ping_sender_timer_test0);

    return 0;
eh_timer_start_error:
    eh_signal_unregister(&ping_sender_timer_signal);
eh_signal_register_error:
    ehip_ping_delete(ping_pcb_test0);
    return ret;
}

static void __exit ping_test_exit(void)
{
    eh_signal_slot_disconnect(&slot_ping_sender_timer_test0);
    eh_timer_stop(eh_signal_to_custom_event(&ping_sender_timer_signal));
    eh_signal_unregister(&ping_sender_timer_signal);
    ehip_ping_delete(ping_pcb_test0);


}

// eh_module_level9_export(ping_test_init, ping_test_exit);