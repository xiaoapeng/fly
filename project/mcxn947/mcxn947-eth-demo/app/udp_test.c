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
#include <ehip-ipv4/udp.h>
#include <ehip-ipv4/ip.h>



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
    eh_infofl("udp sender timer handler");
    if(!ehip_udp_sender_is_init(&udp_sender)){
        ret = ehip_udp_sender_init_ready(udp_pcb, &udp_sender, ipv4_make_addr(192, 168, 9, 44), eh_hton16(9000));
        if(ret < 0){
            eh_errln("udp sender init fail %d", ret);
            return ;
        }
    }

    ehip_udp_sender_buffer_clean(&udp_sender);
    EH_DBG_ERROR_EXEC(ehip_udp_sender_add_buffer(&udp_sender, &out_buffer, &out_buffer_capacity_size) < 0, return );

    if(out_buffer_capacity_size < 4){
        eh_errln("buffer size %d", out_buffer_capacity_size);
        ehip_udp_sender_buffer_clean(&udp_sender);
        return ;
    }
    *(uint32_t *)ehip_buffer_payload_append(out_buffer, 4) = eh_swab32(count++);

    ret = ehip_udp_send(udp_pcb, &udp_sender);
    if(ret < 0){
        eh_errln("udp send fail %d", ret);
        return ;
    }
}

static EH_DEFINE_SLOT(slot_udp_sender_timer, udp_sender_timer_handler, NULL);

static void udp_error_callback(udp_pcb_t pcb, ipv4_addr_t addr, uint16_be_t port, int err){
    (void) pcb;
    eh_errln("udp error callback ip:" IPV4_FORMATIO ":%d err:%d", 
        ipv4_formatio(addr), eh_ntoh16(port), err);
}

static int __init udp_test_init(void)
{
    int ret;
    udp_pcb = ehip_udp_any_new(eh_hton16(9000));
    if (eh_ptr_to_error(udp_pcb) < 0) {
        eh_errln("udp pcb create fail %d", eh_ptr_to_error(udp_pcb));
        return -1;
    }

    ehip_udp_set_error_callback(udp_pcb, udp_error_callback);


    eh_timer_advanced_init(
        eh_signal_to_custom_event(&udp_sender_timer_signal), 
        (eh_sclock_t)eh_msec_to_clock(1000*1), 
        EH_TIMER_ATTR_AUTO_CIRCULATION);
    ret = eh_signal_register(&udp_sender_timer_signal);
    if(ret < 0) goto eh_signal_register_error;
    ret = eh_timer_start(eh_signal_to_custom_event(&udp_sender_timer_signal));
    if(ret < 0) goto eh_timer_start_error;
    eh_signal_slot_connect(&udp_sender_timer_signal, &slot_udp_sender_timer);
    
    return 0;   
eh_timer_start_error:
    eh_signal_unregister(&udp_sender_timer_signal);
eh_signal_register_error:
    ehip_udp_delete(udp_pcb);
    return ret;
}

static void __exit udp_test_exit(void)
{
    if(ehip_udp_sender_is_init(&udp_sender)){
        ehip_udp_sender_buffer_clean(&udp_sender);
    }
    eh_signal_slot_disconnect(&slot_udp_sender_timer);
    eh_timer_stop(eh_signal_to_custom_event(&udp_sender_timer_signal));
    eh_signal_unregister(&udp_sender_timer_signal);
    ehip_udp_delete(udp_pcb);

}
eh_module_level9_export(udp_test_init, udp_test_exit);