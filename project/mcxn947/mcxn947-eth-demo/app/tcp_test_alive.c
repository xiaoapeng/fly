/**
 * @file tcp_test_keepalive.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-05-23
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
#include <stdint.h>

#include <eh_ringbuf.h>
#include "global_signal.h"

static tcp_server_pcb_t tcp_server = NULL;

static void slot_function_ip_setting_ok(eh_event_t  *e, void*  arg){
    (void)e;
    (void)arg;
    int ret;
    ret = ehip_tcp_server_listen(tcp_server);
    if(ret < 0){
        eh_errfl("ehip_tcp_server_listen failed\n");
    }
}

static EH_DEFINE_SLOT(slot_ip_setting_ok, slot_function_ip_setting_ok, NULL);


static void tcp_test_recv_callback(tcp_pcb_t pcb, enum tcp_event state){
    (void) pcb;
    (void) state;
    eh_ringbuf_t *rx_ringbuf = ehip_tcp_client_get_recv_ringbuf(pcb);
    eh_ringbuf_clear(rx_ringbuf);
}

static void tcp_test_keepalive_connect_change_callback(tcp_pcb_t pcb, enum tcp_event state){
    (void) pcb;
    switch (state) {
        case TCP_CONNECT_TIMEOUT:
        case TCP_ERROR:
        case TCP_RECV_FIN:
        case TCP_RECV_RST:
        case TCP_SEND_TIMEOUT:
        case TCP_KEEPALIVE_TIMEOUT:
        case TCP_DISCONNECTED:
            eh_debugfl("tcp_test_keepalive_connect_change_callback state = %s", 
                state == TCP_CONNECT_TIMEOUT ? "TCP_CONNECT_TIMEOUT" :
                state == TCP_CONNECTED ? "TCP_CONNECTED" :
                state == TCP_DISCONNECTED ? "TCP_DISCONNECTED" :
                state == TCP_ERROR ? "TCP_ERROR" :
                state == TCP_RECV_FIN ? "TCP_RECV_FIN" :
                state == TCP_RECV_RST ? "TCP_RECV_RST" : 
                state == TCP_RECV_DATA ? "TCP_RECV_DATA" :
                state == TCP_SEND_TIMEOUT ? "TCP_SEND_TIMEOUT" :
                state == TCP_KEEPALIVE_TIMEOUT ? "TCP_KEEPALIVE_TIMEOUT" :
                state == TCP_RECV_ACK ? "TCP_RECV_ACK" : "UNKNOWN"
            );
            ehip_tcp_client_delete(pcb);
            break;
        case TCP_RECV_DATA:
            tcp_test_recv_callback(pcb, state);
            break;
        case TCP_RECV_ACK:
        case TCP_CONNECTED:
            break;

    }
}


static void tcp_new_connect(tcp_pcb_t new_client){
    uint16_t keepalive_time = 8;
    uint16_t keepalive_intvl = 4;
    uint8_t keepalive_retry_cnt = 4;
    eh_debugfl("tcp_new_connect");
    ehip_tcp_set_events_callback(new_client, tcp_test_keepalive_connect_change_callback);
    ehip_tcp_client_set_userdata(new_client, (void*)0);
    tcp_keepalive_time_config(new_client, &keepalive_time, &keepalive_intvl, &keepalive_retry_cnt);
    eh_debugfl("keepalive_time = %d, keepalive_intvl = %d, keepalive_retry_cnt = %d", keepalive_time, keepalive_intvl, keepalive_retry_cnt);
}

static int __init tcp_test_keepalive_init(void){

    tcp_server = ehip_tcp_server_any_new(eh_hton16(5555), 1024*1 , 1024*1);
    ehip_tcp_server_set_new_connect_callback(tcp_server, tcp_new_connect);

    eh_signal_slot_connect(&sig_eth0_ip_add, &slot_ip_setting_ok);

    return 0;
}

static void __exit tcp_test_keepalive_exit(void){
    eh_signal_slot_disconnect(&sig_eth0_ip_add, &slot_ip_setting_ok);
    ehip_tcp_server_delete(tcp_server);
}


eh_module_level9_export(tcp_test_keepalive_init, tcp_test_keepalive_exit);