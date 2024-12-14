/**
 * @file ehip_core.c
 * @brief ehip 核心程序
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <eh.h>
#include <eh_debug.h>
#include <eh_error.h>
#include <eh_module.h>
#include <eh_signal.h>
#include <eh_mem_pool.h>
#include <eh_llist.h>
#include <eh_timer.h>

#include <ehip_module.h>
#include <ehip_netdev.h>
#include <ehip_conf.h>
#include <ehip_buffer.h>
#include <ehip_core_mbox_msg_type.h>
#include <ehip_core_queue_tx.h>
#include <ehip_protocol_handle.h>

/* 邮箱RX信号 */
EH_STATIC_SIGNAL(signal_mbox_rx);
/* 邮箱实体 */
static struct eh_llist_head mbox_rx;
/* 网卡RX消息句柄内存池 */
static eh_mem_pool_t pool_mbox_msg_rx;
static eh_mem_pool_t pool_queue_entry_tx;

static void  slot_function_mbox_rx(eh_event_t *e, void *slot_param){
    (void)e;
    (void)slot_param;
    struct ehip_mbox_msg_base *mbox_msg_base;
    struct eh_llist_node *pos;
    while((pos = eh_llist_dequeue(&mbox_rx))){
        mbox_msg_base = eh_llist_entry(pos, struct ehip_mbox_msg_base, node);
        switch(mbox_msg_base->type) {
            case EHIP_MBOX_MSG_TYPE_NETDEV_RX:{
                struct ehip_mbox_msg_rx *mbox_msg_netdev_rx = (struct ehip_mbox_msg_rx *)mbox_msg_base;
                ehip_protocol_handle_dispatch(mbox_msg_netdev_rx->netdev_buffer);
                break;
            }
        }
        eh_mem_pool_free(pool_mbox_msg_rx, mbox_msg_base);
    }
}
EH_DEFINE_SLOT(slot_mbox_rx, slot_function_mbox_rx, NULL);

static void slot_function_start_xmit(eh_event_t *e, void *slot_param){
    (void)e;
    ehip_netdev_t *netdev = slot_param;
    struct eh_llist_node *pos;
    struct ehip_tx_msg *tx_msg;
    ehip_buffer_t * tx_buf;
    int ret;
    while((pos = eh_llist_peek(&netdev->tx_queue))){
        tx_msg = eh_llist_entry(pos, struct ehip_tx_msg, node);

        if(netdev->param->ops && netdev->param->ops->ndo_start_xmit){
            tx_buf = ehip_buffer_ref_dup(tx_msg->netdev_buf);
            if((ret = eh_ptr_to_error(tx_buf)) < 0)
                goto busy_and_error;
            /* 这里视为tx_buf的所有权转让,ndo_start_xmit应该承担释放它的责任*/
            ret = netdev->param->ops->ndo_start_xmit(netdev, tx_buf);
            if(ret != 0)
                goto busy_and_error;
        }
        eh_llist_dequeue(&netdev->tx_queue);
        ehip_buffer_free(tx_msg->netdev_buf);
        eh_mem_pool_free(pool_queue_entry_tx, tx_msg);
    }
    eh_event_flags_clear_bits_change_notify(eh_signal_to_custom_event(&netdev->signal_status), EHIP_NETDEV_STATUS_TX_BUSY);
    eh_timer_stop(eh_signal_to_custom_event(&netdev->signal_watchdog));
    return ;
busy_and_error:
    eh_event_flags_set_bits_change_notify(eh_signal_to_custom_event(&netdev->signal_status), EHIP_NETDEV_STATUS_TX_BUSY);
    eh_timer_start(eh_signal_to_custom_event(&netdev->signal_watchdog));
}

static void slot_function_watchdog_timeout(eh_event_t *e, void *slot_param){
    (void)e;
    ehip_netdev_t *netdev = slot_param;
    if(netdev->param->ops && netdev->param->ops->ndo_tx_timeout){
        netdev->param->ops->ndo_tx_timeout(netdev);
    }
}


int ehip_queue_tx(ehip_buffer_t *netdev_buf){
    struct ehip_tx_msg *tx_msg;
    int ret;
    tx_msg = eh_mem_pool_alloc(pool_queue_entry_tx);
    if(tx_msg == NULL){
        ret = EH_RET_MEM_POOL_EMPTY;
        goto eh_mem_pool_alloc_error;
    }
    tx_msg->netdev_buf = netdev_buf;
    eh_llist_enqueue(&tx_msg->node, &netdev_buf->netdev->tx_queue);

    /* 如果TX_BUSY,则直接返回，否则触发wake_up信号 */
    
    if(eh_event_flags_get(eh_signal_to_custom_event(&netdev_buf->netdev->signal_status)) & EHIP_NETDEV_STATUS_TX_BUSY)
        return 0;
    /* wake_up_tx */
    eh_signal_notify(&netdev_buf->netdev->signal_tx_wakeup);
    return 0;
eh_mem_pool_alloc_error:
    ehip_buffer_free(netdev_buf);
    return ret;
}


void ehip_queue_tx_clean(ehip_netdev_t *netdev){
    struct eh_llist_node *pos;
    struct ehip_tx_msg *tx_msg;

    while((pos = eh_llist_dequeue(&netdev->tx_queue))){
        tx_msg = eh_llist_entry(pos, struct ehip_tx_msg, node);
        ehip_buffer_free(tx_msg->netdev_buf);
        eh_mem_pool_free(pool_queue_entry_tx, tx_msg);
    }
}

void ehip_queue_tx_wakeup(ehip_netdev_t *netdev){
    eh_signal_notify(&netdev->signal_tx_wakeup);
}

int ehip_rx(ehip_buffer_t *netdev_buf){
    struct ehip_mbox_msg_rx *mbox_msg_netdev_rx;
    int ret;
    mbox_msg_netdev_rx = eh_mem_pool_alloc(pool_mbox_msg_rx);
    if(mbox_msg_netdev_rx == NULL){
        ret = EH_RET_MEM_POOL_EMPTY;
        goto eh_mem_pool_alloc_error;
    }
    mbox_msg_netdev_rx->base.type = EHIP_MBOX_MSG_TYPE_NETDEV_RX;
    mbox_msg_netdev_rx->netdev_buffer = netdev_buf;
    eh_llist_enqueue(&mbox_msg_netdev_rx->base.node, &mbox_rx);
    eh_signal_notify(&signal_mbox_rx);
    return 0;
eh_mem_pool_alloc_error:
    ehip_buffer_free(netdev_buf);
    return ret;
}


int  _ehip_core_netdev_init(ehip_netdev_t *netdev){
    int ret;
    eh_llist_head_init(&netdev->tx_queue);
    eh_signal_init(&netdev->signal_tx_wakeup);
    ret = eh_signal_register(&netdev->signal_tx_wakeup);
    if(ret < 0){
        goto eh_signal_tx_wakeup_register_error;
    }
    eh_signal_slot_init(&netdev->slot_tx_wakeup, slot_function_start_xmit, netdev);
    eh_signal_slot_connect(&netdev->signal_tx_wakeup, &netdev->slot_tx_wakeup);

    eh_signal_init(&netdev->signal_watchdog);
    eh_timer_advanced_init(
        eh_signal_to_custom_event(&netdev->signal_watchdog),
        (eh_sclock_t)eh_msec_to_clock(EHIP_NETDEV_TX_WATCHDOG_TIMEOUT), 
        0
    );

    ret = eh_signal_register(&netdev->signal_watchdog);
    if(ret < 0){
        goto eh_signal_watchdog_register_error;
    }

    eh_signal_slot_init(&netdev->slot_watchdog, slot_function_watchdog_timeout, netdev);
    eh_signal_slot_connect(&netdev->signal_watchdog, &netdev->slot_watchdog);

    return 0;
eh_signal_watchdog_register_error:
    eh_signal_slot_disconnect(&netdev->slot_tx_wakeup);
    eh_signal_unregister(&netdev->signal_tx_wakeup);
eh_signal_tx_wakeup_register_error:
    /* 无需调用 eh_signal_clean */
    return ret;
}
void _ehip_core_netdev_exit(ehip_netdev_t *netdev){
    eh_signal_slot_disconnect(&netdev->slot_watchdog);
    eh_signal_unregister(&netdev->signal_watchdog);
    eh_timer_clean(eh_signal_to_custom_event(&netdev->signal_watchdog));
    eh_signal_clean(&netdev->signal_watchdog);
    eh_signal_slot_disconnect(&netdev->slot_tx_wakeup);
    eh_signal_unregister(&netdev->signal_tx_wakeup);
    eh_signal_clean(&netdev->signal_tx_wakeup);
}

int  _ehip_core_netdev_up(ehip_netdev_t *netdev){
    eh_llist_head_init(&netdev->tx_queue);
    return 0;
}
void _ehip_core_netdev_down(ehip_netdev_t *netdev){
    ehip_queue_tx_clean(netdev);
    eh_timer_stop(eh_signal_to_custom_event(&netdev->signal_watchdog));
}

static int __init ehip_core_init(void){
    int ret;
    /* 注册连接邮箱RX信号和槽 */
    ret = eh_signal_register(&signal_mbox_rx);
    if(ret < 0) return ret;
    eh_signal_slot_connect(&signal_mbox_rx, &slot_mbox_rx);
    eh_llist_head_init(&mbox_rx);
    /* 申请一个内存池作为网络数据的邮件 */
    pool_mbox_msg_rx = eh_mem_pool_create(EHIP_POOL_BASE_ALIGN, sizeof(struct ehip_mbox_msg_rx), EHIP_CORE_MBOX_NETDEV_RX_MSG_BUFFER_NUM);
    ret = eh_ptr_to_error(pool_mbox_msg_rx);
    if(ret < 0)
        goto eh_mem_pool_create_mbox_msg_netdev_rx_error;
    
    /* 申请一个内存池作为发送队列的项 */
    pool_queue_entry_tx = eh_mem_pool_create(EHIP_POOL_BASE_ALIGN, sizeof(struct ehip_tx_msg), EHIP_CORE_MBOX_NETDEV_TX_MSG_BUFFER_NUM);
    ret = eh_ptr_to_error(pool_queue_entry_tx);
    if(ret < 0)
        goto eh_mem_pool_create_queue_entry_tx_error;
    
    return 0;
eh_mem_pool_create_queue_entry_tx_error:
    eh_mem_pool_destroy(pool_mbox_msg_rx);
eh_mem_pool_create_mbox_msg_netdev_rx_error:
    eh_signal_slot_disconnect(&slot_mbox_rx);
    eh_signal_unregister(&signal_mbox_rx);
    return ret;
}

static void __exit ehip_core_exit(void){
    eh_mem_pool_destroy(pool_queue_entry_tx);
    eh_mem_pool_destroy(pool_mbox_msg_rx);
    eh_signal_slot_disconnect(&slot_mbox_rx);
    eh_signal_unregister(&signal_mbox_rx);
}

ehip_core_module_export(ehip_core_init, ehip_core_exit);