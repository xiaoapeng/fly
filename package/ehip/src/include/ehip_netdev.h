/**
 * @file ehip_netdev.h
 * @brief 提供网卡注册接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-04
 * 
* @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_NETDEV_H_
#define _EHIP_NETDEV_H_

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#include <eh.h>
#include <eh_error.h>
#include <eh_list.h>
#include <eh_llist.h>
#include <eh_signal.h>
#include <eh_timer.h>
#include <eh_event_flags.h>
#include <ehip_conf.h>
#include <ehip_netdev_type.h>
#include <ehip_buffer_type.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct ehip_netdev ehip_netdev_t;
typedef struct ehip_buffer ehip_buffer_t;
struct ehip_protocol_handle;

#define EHIP_NETDEV_STATUS_UP                           0x00000001    /* 网卡是否运行 */
#define EHIP_NETDEV_STATUS_TX_BUSY                      0x00000002    /* 网卡是否忙碌中，忙碌中无法调用ndo_start_xmit */


struct ehip_netdev{
    struct eh_list_head                                 node;
    const struct ehip_netdev_param                     *param;
    struct eh_list_head                                 protocol_handle_head;
    void                                               *userdata;
    enum ehip_netdev_type                               type; /* 类型决定trait使用哪种定义 */
    EH_STRUCT_CUSTOM_SIGNAL(eh_event_flags_t)           signal_status;
    struct {
        uint16_t                                        hw_head_size;
        uint16_t                                        hw_tail_size;
        uint16_t                                        mtu;
        uint8_t                                         hw_addr_len;
        uint8_t                                         buffer_type;
    }attr;

    /* ehip_core.c  _ehip_core_netdev_init 和 _ehip_core_netdev_exit 函数中初始化或释放下面的成员 */
    struct eh_llist_head                                tx_queue;
    EH_STRUCT_SIGNAL                                    signal_tx_wakeup;
    EH_STRUCT_CUSTOM_SIGNAL(eh_event_timer_t)           signal_watchdog;
    eh_signal_slot_t                                    slot_tx_wakeup;
    eh_signal_slot_t                                    slot_watchdog;

};


struct ehip_netdev_ops{
    int (*ndo_up)(ehip_netdev_t *netdev);
    void (*ndo_down)(ehip_netdev_t *netdev);
    int (*ndo_start_xmit)(ehip_netdev_t *netdev, ehip_buffer_t *buf);
    void (*ndo_tx_timeout)(ehip_netdev_t *netdev);
    int (*ndo_ctrl)(ehip_netdev_t *netdev, uint32_t cmd, void *arg);
};

struct ehip_netdev_param{
    char                    *name;
    struct ehip_netdev_ops  *ops;
    void                    *userdata;
    uint16_t                 net_max_frame_size;
};


/**
 * @brief                    注册网卡
 * @param  param             驱动使用static 声明的结构体，需保持全生命周期
 * @return ehip_netdev_t*    返回值使用eh_ptr_to_error判断
 */
extern ehip_netdev_t* ehip_netdev_register(enum ehip_netdev_type, const struct ehip_netdev_param *param);

/**
 * @brief                    注销网卡
 * @param  netdev            由ehip_netdev_register 返回的指针
 */
extern void ehip_netdev_unregister(ehip_netdev_t *netdev);


/**
 * @brief                   为特定网卡注册协议处理函数
 * @param  netdev           网卡句柄
 * @param  handle           协议处理函数描述句柄
 */
extern int ehip_netdev_protocol_handle_register(ehip_netdev_t *netdev, struct ehip_protocol_handle *handle);

/**
 * @brief                   为特定网卡注销协议处理函数
 * @param  netdev           网卡句柄
 * @param  handle           协议处理函数描述句柄
 */
extern void ehip_netdev_protocol_handle_unregister(ehip_netdev_t *netdev, struct ehip_protocol_handle *handle);


/**
 * @brief                   根据网卡名称获取网卡句柄
 * @param  netdev_name      网卡注册时名称
 * @return ehip_netdev_t* 
 */
extern ehip_netdev_t * ehip_netdev_find(char *netdev_name);

/**
 * @brief                   启动网卡
 * @param  netdev           网卡句柄
 * @return int 
 */
extern int ehip_netdev_up(ehip_netdev_t *netdev);

/**
 * @brief                   关闭网卡
 * @param  netdev           网卡句柄
 */
extern void ehip_netdev_down(ehip_netdev_t *netdev);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_H_