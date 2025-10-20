/**
 * @file test.c
 * @brief 测试代码
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-06
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <stdint.h>

#include <eh.h>
#include <eh_swab.h>
#include <eh_module.h>
#include <eh_error.h>
#include <eh_mem_pool.h>
#include <eh_debug.h>
#include <eh_sleep.h>
#include <eh_llist.h>
#include <eh_signal.h>
#include <eh_timer.h>
#include <eh_mem.h>
#include <ehip_chksum.h>
#include <ehip_netdev_tool.h>
#include <ehip_netdev_trait.h>
#include <ehip_buffer.h>
#include <ehip-ipv4/ip.h>
#include <ehip-ipv4/route.h>
#include <ehip-ipv4/arp.h>
#include <ehip-ipv4/udp.h>
#include <ehip-ipv4/ip_message.h>

#include "button.h"

EH_DEFINE_STATIC_CUSTOM_SIGNAL(udp_sender_timer_signal, eh_event_timer_t, EH_TIMER_INIT(udp_sender_timer_signal.custom_event));

void  slot_function_arp_table_dump(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    uint32_t sta = button_sw3_sta();
    if(sta == 0 || e == &signal_arp_table_changed.event){
        eh_infofl("arp table changed:");
        arp_table_dump();
    }
}

EH_DEFINE_SLOT(slot_arp_table_changed, slot_function_arp_table_dump, NULL);
EH_DEFINE_SLOT(slot_button_sw3, slot_function_arp_table_dump, NULL);


static int __init test_init(void){
    eh_signal_slot_connect(&signal_arp_table_changed, &slot_arp_table_changed);
    eh_signal_slot_connect(&button_sw3_signal, &slot_button_sw3);
    return 0;
}

static void __exit test_exit(void){
    eh_signal_slot_disconnect(&button_sw3_signal, &slot_button_sw3);
    eh_signal_slot_disconnect(&signal_arp_table_changed, &slot_arp_table_changed);
}

eh_module_level0_export(test_init, test_exit);
