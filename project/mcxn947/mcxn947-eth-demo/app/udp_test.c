/**
 * @file udp_test.c
 * @brief udp 测试程序
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-03-31
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include <eh_swab.h>
#include <eh_debug.h>
#include <eh_module.h>
#include <ehip_error.h>
#include <ehip_buffer.h>
#include <ehip_netdev_tool.h>
#include <ehip-ipv4/udp.h>
#include <ehip-ipv4/ip.h>
#include <ehip-ipv4/ip_message.h>



static udp_pcb_t udp_pcb;
static struct udp_sender udp_sender;

EH_DEFINE_STATIC_CUSTOM_SIGNAL(udp_sender_timer_signal, eh_event_timer_t, EH_TIMER_INIT(udp_sender_timer_signal.custom_event));

static void udp_sender_timer_handler(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    int ret;
    static uint32_t count = 0;
    ehip_buffer_t *out_buffer;
    ehip_buffer_size_t out_buffer_capacity_size;
    eh_mdebugfl(UDP_TEST, "udp sender timer handler");
    ret = ehip_udp_sender_route_ready(&udp_sender);
    if(ret < 0){
        eh_merrfl(UDP_TEST, "udp sender route fail %d", ret);
        return ;
    }
    
    ret = ehip_udp_sender_add_buffer(&udp_sender, &out_buffer, &out_buffer_capacity_size);
    if(ret < 0){
        eh_merrfl(UDP_TEST, "udp sender add buffer fail %d", ret);
        return ;
    }

    if(out_buffer_capacity_size < 4){
        eh_merrfl(UDP_TEST, "buffer size %d", out_buffer_capacity_size);
        return ;
    }
    *(uint32_t *)ehip_buffer_payload_append(out_buffer, 4) = eh_swab32(count++);

    ret = ehip_udp_send(udp_pcb, &udp_sender);
    if(ret < 0){
        eh_merrfl(UDP_TEST, "udp send fail %d", ret);
        return ;
    }
}

static EH_DEFINE_SLOT(slot_udp_sender_timer, udp_sender_timer_handler, NULL);

static void udp_error_callback(udp_pcb_t pcb, ipv4_addr_t addr, uint16_be_t port, int err){
    (void) pcb;
    eh_merrfl(UDP_TEST, "udp error callback ip:" IPV4_FORMATIO ":%d err:%d", 
        ipv4_formatio(addr), eh_ntoh16(port), err);
}


static void udp_recv_callback(udp_pcb_t pcb, ipv4_addr_t addr, uint16_be_t port, struct ip_message *udp_rx_meg){
    (void) pcb;
    static uint32_t rx_byte = 0;
    eh_minfofl(UDP_TEST, "udp recv callback ip:" IPV4_FORMATIO ":%d", 
        ipv4_formatio(addr), eh_ntoh16(port));
    if(ip_message_flag_is_fragment(udp_rx_meg)){
        ehip_buffer_t *pos_buffer;
        int int_tmp_i, int_tmp_sort_i;
        ip_message_rx_fragment_for_each(pos_buffer, int_tmp_i, int_tmp_sort_i, udp_rx_meg){
            eh_minfofl(UDP_TEST, "fragment[%03d]:%04d|%.*hhq ...|", int_tmp_i, ehip_buffer_get_payload_size(pos_buffer), 
                ehip_buffer_get_payload_size(pos_buffer) > 32 ? 32 : ehip_buffer_get_payload_size(pos_buffer) ,
                 ehip_buffer_get_payload_ptr(pos_buffer));
        }
    }else{
        eh_minfofl(UDP_TEST, "payload %.*hhq", ehip_buffer_get_payload_size(udp_rx_meg->buffer), 
            ehip_buffer_get_payload_ptr(udp_rx_meg->buffer));
    }
    rx_byte += (uint32_t)ip_message_rx_data_size(udp_rx_meg);
    eh_minfofl(UDP_TEST, "rx total size %d \n", rx_byte);
}

static int __init udp_test_init(void)
{
    int ret;
    udp_pcb = ehip_udp_any_new(eh_hton16(800));
    if (eh_ptr_to_error(udp_pcb) < 0) {
        eh_merrfl(UDP_TEST, "udp pcb create fail %d", eh_ptr_to_error(udp_pcb));
        return -1;
    }

    ehip_udp_set_error_callback(udp_pcb, udp_error_callback);
    ehip_udp_set_recv_callback(udp_pcb, udp_recv_callback);

    eh_timer_advanced_init(
        eh_signal_to_custom_event(&udp_sender_timer_signal), 
        (eh_sclock_t)eh_msec_to_clock(1000*1), 
        EH_TIMER_ATTR_AUTO_CIRCULATION);
    ret = eh_signal_register(&udp_sender_timer_signal);
    if(ret < 0) goto eh_signal_register_error;
    ret = eh_timer_start(eh_signal_to_custom_event(&udp_sender_timer_signal));
    if(ret < 0) goto eh_timer_start_error;
    eh_signal_slot_connect(&udp_sender_timer_signal, &slot_udp_sender_timer);
    
    ehip_udp_sender_init(udp_pcb, &udp_sender, ipv4_make_addr(192,168,12,88), eh_hton16(9000));
    return 0;   
eh_timer_start_error:
    eh_signal_unregister(&udp_sender_timer_signal);
eh_signal_register_error:
    ehip_udp_delete(udp_pcb);
    return ret;
}

static void __exit udp_test_exit(void)
{
    ehip_udp_sender_deinit(&udp_sender);
    eh_signal_slot_disconnect(&slot_udp_sender_timer);
    eh_timer_stop(eh_signal_to_custom_event(&udp_sender_timer_signal));
    eh_signal_unregister(&udp_sender_timer_signal);
    ehip_udp_delete(udp_pcb);

}
eh_module_level9_export(udp_test_init, udp_test_exit);