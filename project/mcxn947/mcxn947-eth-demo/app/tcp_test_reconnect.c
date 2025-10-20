/**
 * @file tcp_test_reconnect.c
 * @brief TCP 重连测试
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-09-28
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */


#include <eh.h>
#include <eh_swab.h>
#include <eh_module.h>
#include <eh_debug.h>
#include <ehip-ipv4/tcp.h>
#include <ehip-ipv4/ip.h>
#include <eh_event.h>
#include <eh_signal.h>
#include <eh_timer.h>

#include <eh_ringbuf.h>
#include "global_signal.h"

static tcp_pcb_t test_client;
EH_DEFINE_STATIC_CUSTOM_SIGNAL(timeout_signal, eh_event_timer_t, EH_TIMER_INIT(timeout_signal.custom_event));



static void slot_function_ip_setting_ok(eh_event_t  *e, void*  arg){
    (void)e;
    (void)arg;
    int ret;
    ret = ehip_tcp_client_connect(test_client);
    if(ret < 0){
        eh_merrfl(TCP_TEST_RECONNECT, "connect failed ret = %d!!", ret);
        return ;
    }
}

static EH_DEFINE_SLOT(slot_ip_setting_ok, slot_function_ip_setting_ok, NULL);


static void timeout_slot_function(eh_event_t *e, void *slot_param){
    (void)slot_param;
    (void)e;
    ehip_tcp_client_disconnect(test_client);
}


static EH_DEFINE_SLOT(timeout_slot, timeout_slot_function, NULL);

void tcp_test_reconnect_events_callback(tcp_pcb_t pcb, enum tcp_event state){
    (void) pcb;
    int ret;
    switch (state) {
        case TCP_CONNECT_TIMEOUT:
        case TCP_ERROR:
        case TCP_RECV_RST:
        case TCP_SEND_TIMEOUT:
        case TCP_DISCONNECTED:
            eh_timer_stop(eh_signal_to_custom_event(&timeout_signal));
            eh_debugfl("tcp_test_send_connect_change_callback state = %s", 
                state == TCP_CONNECT_TIMEOUT ? "TCP_CONNECT_TIMEOUT" :
                state == TCP_DISCONNECTED ? "TCP_DISCONNECTED" :
                state == TCP_ERROR ? "TCP_ERROR" :
                state == TCP_RECV_RST ? "TCP_RECV_RST" : "UNKNOWN"
            );
            eh_minfofl(TCP_TEST_RECONNECT, "Auto reconnect...");
            ret = ehip_tcp_client_connect(test_client);
            if(ret < 0){
                eh_merrfl(TCP_TEST_RECONNECT, "connect failed ret = %d!!", ret);
            }
            break;
        case TCP_CONNECTED:
            /* 可以开一个定时器，自动断连 */
            eh_minfofl(TCP_TEST_RECONNECT, "Connected, Auto disconnect after 10s...");
            eh_timer_start(eh_signal_to_custom_event(&timeout_signal));
            break;
        case TCP_RECV_DATA:
        case TCP_RECV_ACK:
        case TCP_RECV_FIN:
            break;
    }

}

static int __init tcp_test_reconnect_init(void){
    test_client = ehip_tcp_client_any_new(0, ipv4_make_addr(192, 168, 12, 12), eh_hton16(9999), 1024*2, 1024*2);
    ehip_tcp_set_events_callback(test_client, tcp_test_reconnect_events_callback);
    
    eh_signal_slot_connect(&sig_eth0_ip_add, &slot_ip_setting_ok);

    eh_timer_advanced_init(eh_signal_to_custom_event(&timeout_signal), (eh_sclock_t)eh_msec_to_clock(1000 * 10), 0);

    eh_signal_slot_connect(&timeout_signal, &timeout_slot);
    return 0;
}

static void __exit tcp_test_reonnect_exit(void){
    eh_signal_slot_disconnect(&timeout_signal, &timeout_slot);
    eh_signal_slot_disconnect(&sig_eth0_ip_add, &slot_ip_setting_ok);
    ehip_tcp_client_delete(test_client);
}


eh_module_level9_export(tcp_test_reconnect_init, tcp_test_reonnect_exit);
