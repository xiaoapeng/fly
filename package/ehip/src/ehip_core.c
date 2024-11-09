/**
 * @file ehip_core.c
 * @brief ehip 核心程序
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include "eh.h"
#include "eh_debug.h"
#include "eh_error.h"
#include "eh_module.h"
#include "eh_signal.h"
#include "eh_ringbuf.h"
#include "eh_mem_pool.h"

#include "ehip_module.h"
#include "ehip_netdev.h"
#include "ehip_conf.h"
#include "ehip_core_mbox_msg_type.h"

/* 邮箱RX信号 */
EH_STATIC_SIGNAL(signal_mbox_rx);
/* 邮箱实体 */
static eh_ringbuf_t *mbox_rx;
/* 网卡RX消息句柄内存池 */
static eh_mem_pool_t pool_netdev_rx;

void  slot_function_mbox_rx(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    struct ehip_mbox_msg_base *mbox_msg_base;
    if(eh_ringbuf_read(mbox_rx, (uint8_t*)&mbox_msg_base, sizeof(void*)) == 0)
        return ;
    switch(mbox_msg_base->type) {
        case EHIP_MBOX_MSG_TYPE_NETDEV_RX:
            break;
    }
}
EH_DEFINE_SLOT(slot_mbox_rx, slot_function_mbox_rx, NULL);


int ehip_rx(ehip_buffer_t *netdev_buf){
    (void) netdev_buf;
    return 0;
}


static int __init ehip_core_init(void){
    int ret;
    /* 注册连接邮箱RX信号和槽 */
    eh_signal_register(&signal_mbox_rx);
    eh_signal_slot_connect(&signal_mbox_rx, &slot_mbox_rx);

    /* 申请一个ringbuf作为邮箱 */
    mbox_rx = eh_ringbuf_create((int32_t)(EHIP_CORE_MBOX_MSG_SIZE * sizeof(void*)), NULL);
    ret = eh_ptr_to_error(mbox_rx);
    if(ret < 0)
        goto eh_ringbuf_create_msg_mbox_error;

    /* 申请一个内存池作为网络数据的邮件 */
    pool_netdev_rx = eh_mem_pool_create(EHIP_POOL_BASE_ALIGN, sizeof(struct ehip_mbox_msg_netdev_rx), EHIP_CORE_MBOX_NETDEV_MSG_BUFFER_NUM);
    ret = eh_ptr_to_error(mbox_rx);
    if(ret < 0)
        goto eh_mem_pool_create_mbox_msg_netdev_rx_error;

    return 0;
eh_mem_pool_create_mbox_msg_netdev_rx_error:
    eh_ringbuf_destroy(mbox_rx);
eh_ringbuf_create_msg_mbox_error:
    eh_signal_slot_disconnect(&slot_mbox_rx);
    eh_signal_unregister(&signal_mbox_rx);
    return ret;
}

static void __exit ehip_core_exit(void){
    eh_mem_pool_destroy(pool_netdev_rx);
    eh_ringbuf_destroy(mbox_rx);
    eh_signal_slot_disconnect(&slot_mbox_rx);
    eh_signal_unregister(&signal_mbox_rx);
}

ehip_core_module_export(ehip_core_init, ehip_core_exit);