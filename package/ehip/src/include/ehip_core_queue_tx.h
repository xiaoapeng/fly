/**
 * @file ehip_core_queue_tx.h
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-12-02
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_CORE_QUEUE_TX_H_
#define _EHIP_CORE_QUEUE_TX_H_

#include <eh_llist.h>
#include <ehip_buffer.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

struct ehip_tx_msg{
    struct eh_llist_node node;
    ehip_buffer_t *netdev_buf;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_CORE_QUEUE_TX_H_