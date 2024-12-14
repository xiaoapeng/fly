/**
 * @file ehip_core.h
 * @brief ehip 核心
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_CORE_H_
#define _EHIP_CORE_H_

#include <eh.h>
#include <eh_types.h>
#include <eh_event.h>
#include <ehip_buffer.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

extern int  _ehip_core_netdev_init(ehip_netdev_t *netdev);
extern void _ehip_core_netdev_exit(ehip_netdev_t *netdev);
extern int  _ehip_core_netdev_up(ehip_netdev_t *netdev);
extern void _ehip_core_netdev_down(ehip_netdev_t *netdev);

/**
 * @brief                   ehip 网络数据的入口，
 *                          这里将视为buf所有权的移交，
 *                          无论函数是否执行成功，所有权都由ehip_rx所有
 *                          若调用该函数还要继续使用Buf，请dup/ref一个新buf/新引用
 * @param  netdev_buf       网络数据Buf
 * @return int
 */
extern int ehip_rx(ehip_buffer_t *netdev_buf);

/**
 * @brief                   ehip网络数据的出口，将buf挂在队列，等待发送
 *                          这里将视为buf所有权的移交，
 *                          无论函数是否执行成功，所有权都由ehip_queue_tx所有
 *                          若调用该函数还要继续使用Buf，请dup/ref一个新buf/新引用
 * @param  netdev_buf       网络数据Buf
 * @return int
 */
extern int ehip_queue_tx(ehip_buffer_t *netdev_buf);

/**
 * @brief                   清空网络设备的发送队列
 * @param  netdev           网络设备
 * @return int
 */
extern void ehip_queue_tx_clean(ehip_netdev_t *netdev);

/**
 * @brief                   唤醒网络设备的发送队列,一般在忙时队列会进入阻塞状态，在不忙后应该唤醒队列
 * @param  netdev           网络设备
 */
extern __safety void ehip_queue_tx_wakeup(ehip_netdev_t *netdev);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_CORE_H_
