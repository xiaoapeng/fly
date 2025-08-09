/**
 * @file tcp_test_echo.c
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

#include "eh_ringbuf.h"
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

static void tcp_test_send_callback(tcp_pcb_t pcb){
    int data = (int)(intptr_t)ehip_tcp_client_get_userdata(pcb);
    eh_ringbuf_t *tx_ringbuf = ehip_tcp_client_get_send_ringbuf(pcb);
    
    if(data >= 10*1024*1024){
        ehip_tcp_client_delete(pcb);
        return ;
    }

    while(eh_ringbuf_write(tx_ringbuf, (uint8_t*)&data, sizeof(data)) > 0){
        data++;
    }
    ehip_tcp_client_request_send(pcb);
    ehip_tcp_client_set_userdata(pcb, (void*)(intptr_t)data);
}

static void tcp_test_echo_recv_callback(tcp_pcb_t pcb, enum tcp_event state){
    (void) pcb;
    // tcp_client_info_t info;
    // uint8_t *data_ptr;
    // int32_t len;
    // int32_t offset;
    // int32_t wl = 0;
    // eh_ringbuf_t *tx_ringbuf = ehip_tcp_client_get_send_ringbuf(pcb);
    eh_ringbuf_t *rx_ringbuf = ehip_tcp_client_get_recv_ringbuf(pcb);
    // ehip_tcp_client_get_info(pcb, &info);

    eh_ringbuf_clear(rx_ringbuf);
    // eh_mdebugfl(TCP_TEST_ECHO,"("IPV4_FORMATIO":%d)->("IPV4_FORMATIO":%d) size:%d", 
    //     ipv4_formatio(info.remote_addr),
    //     info.remote_port,
    //     ipv4_formatio(info.local_addr), 
    //     info.local_port,
    //     eh_ringbuf_size(rx_ringbuf));
    
    // len = 0;
    // data_ptr = (uint8_t *)eh_ringbuf_peek(rx_ringbuf, 0, NULL, &len);
    // wl += eh_ringbuf_write(tx_ringbuf, data_ptr, len);
    // offset = len;
    // len = 0;
    // data_ptr = (uint8_t *)eh_ringbuf_peek(rx_ringbuf, offset, NULL, &len);
    // wl += eh_ringbuf_write(tx_ringbuf, data_ptr, len);
    // eh_ringbuf_read_skip(rx_ringbuf, wl);
    // if(state == TCP_RECV_DATA || eh_ringbuf_free_size(tx_ringbuf) == 0 || eh_ringbuf_size(rx_ringbuf) == 0){
    //     ehip_tcp_client_request_send(pcb);
    // }
}


static void tcp_test_echo_connect_change_callback(tcp_pcb_t pcb, enum tcp_event state){
    (void) pcb;
    switch (state) {
        case TCP_CONNECT_TIMEOUT:
        case TCP_ERROR:
        case TCP_RECV_FIN:
        case TCP_RECV_RST:
        case TCP_SEND_TIMEOUT:
        case TCP_DISCONNECTED:
            eh_debugfl("tcp_test_echo_connect_change_callback state = %s", 
                state == TCP_CONNECT_TIMEOUT ? "TCP_CONNECT_TIMEOUT" :
                state == TCP_CONNECTED ? "TCP_CONNECTED" :
                state == TCP_DISCONNECTED ? "TCP_DISCONNECTED" :
                state == TCP_ERROR ? "TCP_ERROR" :
                state == TCP_RECV_FIN ? "TCP_RECV_FIN" :
                state == TCP_RECV_RST ? "TCP_RECV_RST" : 
                state == TCP_RECV_DATA ? "TCP_RECV_DATA" :
                state == TCP_RECV_ACK ? "TCP_RECV_ACK" : "UNKNOWN"
            );
            ehip_tcp_client_delete(pcb);
            break;
        case TCP_CONNECTED:
            tcp_test_send_callback(pcb);
            break;
        case TCP_RECV_DATA:
            tcp_test_echo_recv_callback(pcb, state);
            break;
        case TCP_RECV_ACK:
            tcp_test_send_callback(pcb);
            break;
    }
}


static void tcp_new_connect(tcp_pcb_t new_client){
    eh_debugfl("tcp_new_connect");
    ehip_tcp_set_events_callback(new_client, tcp_test_echo_connect_change_callback);
    ehip_tcp_client_set_userdata(new_client, (void*)0);
}

static int __init tcp_test_echo_init(void){

    tcp_server = ehip_tcp_server_any_new(eh_hton16(8888), 1024*10 , 1024*10);
    ehip_tcp_server_set_new_connect_callback(tcp_server, tcp_new_connect);

    eh_signal_slot_connect(&sig_eth0_ip_add, &slot_ip_setting_ok);

    return 0;
}

static void __exit tcp_test_echo_exit(void){
    eh_signal_slot_disconnect(&slot_ip_setting_ok);
}


eh_module_level9_export(tcp_test_echo_init, tcp_test_echo_exit);