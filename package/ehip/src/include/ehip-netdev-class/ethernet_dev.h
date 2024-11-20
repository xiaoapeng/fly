/**
 * @file ethernet_dev.h
 * @brief 以太网设备CONFIG接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-05
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _ETHERNET_DEV_H_
#define _ETHERNET_DEV_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdint.h>
#include <ehip-mac/ethernet.h>
#include <ehip_conf.h>
#include <ehip-ipv4/ip.h>
#include <ehip_netdev_trait.h>

/**
 * @brief 定义的是ehip_netdev->signal成员的位图定义，在以太网设备中的含义
 */
#define EHIP_NETDEV_ETHERNET_STATUS_LINK                   0x00000002    /* LINK 状态 */
#define EHIP_NETDEV_ETHERNET_STATUS_PROMISC                0x00000004    /* PROMISC(混杂)模式，接收全部的包 */
#define EHIP_NETDEV_ETHERNET_STATUS_ALLMULTI               0x00000008    /* 接收全部的组播包 */
#define EHIP_NETDEV_ETHERNET_STATUS_BROADCAST              0x00000010    /* 接收广播包 */
#define EHIP_NETDEV_ETHERNET_STATUS_MULTICAST              0x00000020    /* 接收指定组播包 */

/**
 * @brief 定义ndo_ctrl函数的命令参数
 */
#define ETHERNET_CTRL_CMD_SET_RX_FILTER_MODE                0x00000001
#define ETHERNET_CTRL_CMD_SET_MAC_ADDR                      0x00000002
#define ETHERNET_CTRL_CMD_SET_MULTICAST_ADDR                0x00000003

struct ethernet_trait{
    uint16_t                                    hw_head_size;                  /* 一般是 14*/
    uint16_t                                    hw_tail_size;                  /* 一般是 4 */
    uint16_t                                    mtu;                           /* 一般是 1500 */
    ehip_eth_addr_t                             hw_addr;
    enum ehip_ptype                             mac_ptype;
    ehip_eth_addr_t                             multicast_hw_addr[ETH_MULTICAST_ADDR_NUM];
    struct ipv4_netdev                          ipv4_netdev;
};


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ETHERNET_DEV_H_