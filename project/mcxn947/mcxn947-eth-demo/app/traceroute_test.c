/**
 * @file traceroute_test.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-04-22
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <stdint.h>
#include <string.h>
#include <eh.h>
#include <eh_error.h>
#include <eh_swab.h>
#include <eh_debug.h>
#include <eh_signal.h>
#include <eh_module.h>
#include <eh_timer.h>
#include <ehip_error.h>
#include <ehip-ipv4/ping.h>
#include <ehip-ipv4/ip.h>


#define MAX_CNT 10
#define MAX_ROUTE_DEEP 30
static ping_pcb_t traceroute_pcb_test0;

static ipv4_addr_t route_adder[MAX_ROUTE_DEEP];
static int g_strat_seg = -1;
static uint8_t g_ttl = 1;
static uint8_t g_retry = 0;
EH_DEFINE_STATIC_CUSTOM_SIGNAL(ping_response_sender_timer_signal, eh_event_timer_t, EH_TIMER_INIT(ping_response_sender_timer_signal.custom_event));

void traceroute_error_callback(ping_pcb_t pcb, ipv4_addr_t addr, uint16_t seq, int erron){
    (void)pcb;
    if(erron == EHIP_RET_TTL_EXPIRED && (int)seq > g_strat_seg && g_ttl <= MAX_ROUTE_DEEP){
        eh_mdebugfl(TRACEROUTE_TEST, "");
        eh_mdebugfl(TRACEROUTE_TEST, "seq %d deep %d: no response", seq, g_ttl);
        if( !route_adder[g_ttl-1] )
            route_adder[g_ttl-1] = addr;

        for(uint8_t i=0; i < g_ttl ; i++){
            if(route_adder[i])
                eh_mdebugfl(TRACEROUTE_TEST, "%d :"IPV4_FORMATIO, i, ipv4_formatio(route_adder[i]));
            else
                eh_mdebugfl(TRACEROUTE_TEST, "%d : no response", i);
        }
        g_ttl++;
        g_retry = 0;
        g_strat_seg = INT32_MAX;
    }
}

void traceroute_response_callback(ping_pcb_t pcb, ipv4_addr_t addr, uint16_t seq, uint8_t ttl,  eh_clock_t time_ms){
    (void)pcb;
    (void)addr;
    (void)seq;
    (void)time_ms;
    (void)ttl;
    eh_timer_stop(eh_signal_to_custom_event(&ping_response_sender_timer_signal));
    eh_minfofl(TRACEROUTE_TEST, "%p ping response: ttl %d", pcb, g_ttl);
}

static void ping_sender_timer_handler(eh_event_t *e, void *slot_param){
    (void)e;
    int ret;
    ping_pcb_t pcb = *((ping_pcb_t*)slot_param);

    if(g_retry >= MAX_CNT){
        g_retry = 0;
        g_ttl++;
    }

    if(g_retry == 0)
        ehip_ping_set_ttl(pcb, g_ttl);

    if(g_ttl > MAX_ROUTE_DEEP){
        eh_minfofl(TRACEROUTE_TEST, "%p ping request: deep %d", pcb, g_ttl);
        eh_timer_stop(eh_signal_to_custom_event(&ping_response_sender_timer_signal));
        return ;
    }

    // eh_merrfl(TRACEROUTE_TEST,"ehip_ping_request ttl %d!", g_ttl);
    ret = ehip_ping_request(pcb, 40);
    if(ret < 0){
        eh_merrfl(TRACEROUTE_TEST,"%p ehip_ping_request failed ret = %d", pcb, ret);
        return ;
    }

    if(g_retry == 0){
        g_strat_seg = ret;
    }
    g_retry++;
}

static EH_DEFINE_SLOT(slot_ping_sender_timer_test0, ping_sender_timer_handler, &traceroute_pcb_test0);

static int __init  traceroute_init(void){
    int ret = 0;
    traceroute_pcb_test0 = ehip_ping_any_new(ipv4_make_addr(114,114,114,114));
    if(eh_ptr_to_error(traceroute_pcb_test0) < 0){
        eh_errln("traceroute_init failed...");
        return eh_ptr_to_error(traceroute_pcb_test0);
    }
    ehip_ping_set_error_callback(traceroute_pcb_test0, traceroute_error_callback);
    ehip_ping_set_response_callback(traceroute_pcb_test0, traceroute_response_callback);
    ehip_ping_set_timeout(traceroute_pcb_test0, 255); // 90 * 100ms = 9s

    eh_timer_advanced_init(
        eh_signal_to_custom_event(&ping_response_sender_timer_signal), 
        (eh_sclock_t)eh_msec_to_clock(100),
        EH_TIMER_ATTR_AUTO_CIRCULATION);

    ret = eh_timer_start(eh_signal_to_custom_event(&ping_response_sender_timer_signal));
    if(ret < 0) goto eh_timer_start_error;
    ret = eh_signal_slot_connect(&ping_response_sender_timer_signal, &slot_ping_sender_timer_test0);
    if(ret < 0) goto eh_signal_slot_connect_sender_timer_signal_err;
    memset(route_adder, 0, sizeof(route_adder));
    return 0;
eh_signal_slot_connect_sender_timer_signal_err:
    eh_timer_stop(eh_signal_to_custom_event(&ping_response_sender_timer_signal));
eh_timer_start_error:
    ehip_ping_delete(traceroute_pcb_test0);
    return ret;
}

static void __exit traceroute_exit(void){
    eh_signal_slot_disconnect(&ping_response_sender_timer_signal, &slot_ping_sender_timer_test0);
    eh_timer_stop(eh_signal_to_custom_event(&ping_response_sender_timer_signal));
    ehip_ping_delete(traceroute_pcb_test0);
}


eh_module_level9_export(traceroute_init, traceroute_exit);