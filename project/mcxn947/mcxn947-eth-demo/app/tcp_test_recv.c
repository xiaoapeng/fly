/**
 * @file tcp_test.c
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


static void tcp_test_recv_callback(tcp_pcb_t pcb){
    (void) pcb;
    tcp_client_info_t info;
    uint8_t data = (uint8_t)(uintptr_t)ehip_tcp_client_get_userdata(pcb);
    uint8_t *data_ptr;
    int32_t len;
    int32_t offset;
    eh_ringbuf_t *rx_ringbuf = ehip_tcp_client_get_recv_ringbuf(pcb);
    ehip_tcp_client_get_info(pcb, &info);

    eh_debugfl("("IPV4_FORMATIO":%d)->("IPV4_FORMATIO":%d) size:%d", 
        ipv4_formatio(info.remote_addr),
        info.remote_port,
        ipv4_formatio(info.local_addr), 
        info.local_port,
        eh_ringbuf_size(rx_ringbuf));
    
    len = 0;
    data_ptr = (uint8_t *)eh_ringbuf_peek(rx_ringbuf, 0, NULL, &len);
    for(int32_t i = 0; i < len; i++, data++){
        if(data_ptr[i] != data){
            eh_warnfl("tcp_test_recv_callback data error, expect %d, but got %d", data, data_ptr[i]);
            goto quit;
        }
    }
    offset = len;
    len = 0;
    data_ptr = (uint8_t *)eh_ringbuf_peek(rx_ringbuf, offset, NULL, &len);
    for(int32_t i = 0; i < len; i++, data++){
        if(data_ptr[i] != data){
            eh_warnfl("tcp_test_recv_callback data error, expect %d, but got %d", data, data_ptr[i]);
            goto quit;
        }
    }
    ehip_tcp_client_set_userdata(pcb, (void*)(uintptr_t)data);


quit:
    eh_ringbuf_clear(rx_ringbuf);

}


static void tcp_test_connect_change_callback(tcp_pcb_t pcb, enum tcp_event state){
    (void) pcb;
    eh_debugfl("tcp_test_connect_change_callback state = %s", 
            state == TCP_CONNECT_TIMEOUT ? "TCP_CONNECT_TIMEOUT" :
            state == TCP_CONNECTED ? "TCP_CONNECTED" :
            state == TCP_DISCONNECTED ? "TCP_DISCONNECTED" :
            state == TCP_ERROR ? "TCP_ERROR" :
            state == TCP_RECV_FIN ? "TCP_RECV_FIN" :
            state == TCP_RECV_RST ? "TCP_RECV_RST" : 
            state == TCP_RECV_DATA ? "TCP_RECV_DATA" :
            state == TCP_RECV_ACK ? "TCP_RECV_ACK" : "UNKNOWN"
        );
    
    switch (state) {
        case TCP_CONNECT_TIMEOUT:
        case TCP_ERROR:
        case TCP_RECV_FIN:
        case TCP_RECV_RST:
        case TCP_DISCONNECTED:
        case TCP_SEND_TIMEOUT:
            ehip_tcp_client_delete(pcb);
            break;
        case TCP_CONNECTED:
            break;
        case TCP_RECV_DATA:
        case TCP_RECV_ACK:
            tcp_test_recv_callback(pcb);
            break;
        }
}


static void tcp_new_connect(tcp_pcb_t new_client){
    eh_debugfl("tcp_new_connect");
    ehip_tcp_set_events_callback(new_client, tcp_test_connect_change_callback);
    ehip_tcp_client_set_userdata(new_client, (void*)0);
}

static int __init tcp_test_init(void){

    tcp_server = ehip_tcp_server_any_new(eh_hton16(7777), 1024*8 , 80);
    ehip_tcp_server_set_new_connect_callback(tcp_server, tcp_new_connect);

    eh_signal_slot_connect(&sig_eth0_ip_add, &slot_ip_setting_ok);

    return 0;
}

static void __exit tcp_test_exit(void){
    eh_signal_slot_disconnect(&sig_eth0_ip_add, &slot_ip_setting_ok);
}


eh_module_level9_export(tcp_test_init, tcp_test_exit);