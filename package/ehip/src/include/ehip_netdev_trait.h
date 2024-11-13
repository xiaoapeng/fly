/**
 * @file ehip_netdev_trait.h
 * @brief 获取网卡特征接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _NETDEV_TRAIT_H_
#define _NETDEV_TRAIT_H_

#include "ehip-mac/hw_addr.h"
#include <stddef.h>

#include <ehip_netdev.h>
#include <ehip_netdev_type.h>
#include <ehip-mac/ethernet.h>
#include <ehip-ipv4/ip.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct ehip_netdev ehip_netdev_t;
typedef void *ehip_netdev_trait_t;

struct ehip_netdev_trait_ops{
    size_t  trait_size;
    void (*reset)(ehip_netdev_trait_t *netdev_trait);
    const ehip_hw_addr_t* (*get_hw_addr)(const ehip_netdev_trait_t *netdev_trait);
    void (*set_hw_addr)(ehip_netdev_trait_t *netdev_trait, ehip_hw_addr_t *hw_addr);
    const struct ipv4_addr * (*get_ipv4_addr)(const ehip_netdev_trait_t *netdev_trait, int index);
    int (*set_ipv4_addr)(ehip_netdev_trait_t *netdev_trait, int index, struct ipv4_addr *ipv4_addr);
    int (*set_multicast_hw_addr)(ehip_netdev_trait_t *netdev_trait, int index, ehip_hw_addr_t *hw_addr);
    const ehip_hw_addr_t* (*get_multicast_hw_addr)(const ehip_netdev_trait_t *netdev_trait, int index);
    uint8_t  hw_addr_len;
};


extern size_t ehip_netdev_trait_size_get(enum ehip_netdev_type type);

/**
 * @brief 安装网卡特征属性
 * @param type 网卡类型
 * @param ops 网卡特征操作接口
 * @return int 
 */
extern int ehip_netdev_trait_type_install(enum ehip_netdev_type type, const struct ehip_netdev_trait_ops *ops);

/**
 * @brief               网卡特征属性重置
 * @param               netdev           
 * @return              int 
 */
extern int ehip_netdev_trait_reset(ehip_netdev_t *netdev);

/**
 * @brief               获取网卡MAC地址
 * @param  netdev
 * @param  hw_addr      网卡MAC地址
 * @return int 
 */
extern const ehip_hw_addr_t* ehip_netdev_trait_get_hw_addr(const ehip_netdev_t *netdev);

/**
 * @brief               设置网卡MAC地址
 * @param  netdev
 * @param  hw_addr      网卡MAC地址
 * @return int 
 */
extern int ehip_netdev_trait_set_hw_addr(ehip_netdev_t *netdev, ehip_hw_addr_t *hw_addr);

/**
 * @brief               获取网卡IPv4地址
 * @param  netdev
 * @param  index        网卡IPv4地址索引,一般小于EHIP_NETDEV_MAX_IP_NUM
 * @param  ipv4_addr    网卡IPv4地址
 * @return int 
 */
extern const struct ipv4_addr * ehip_netdev_trait_get_ipv4_addr(const ehip_netdev_t *netdev, int index);


/**
 * @brief               设置网卡IPv4地址
 * @param  netdev       
 * @param  index        网卡IPv4地址索引,一般小于EHIP_NETDEV_MAX_IP_NUM
 * @param  ipv4_addr    网卡IPv4地址
 * @return int 
 */
extern int ehip_netdev_trait_set_ipv4_addr(ehip_netdev_t *netdev, int index, struct ipv4_addr *ipv4_addr);


/**
 * @brief               获取网卡物理地址长度
 * @param  netdev
 * @return int          返回网卡物理地址长度
 */
extern int ehip_netdev_trait_get_hw_addr_len(const ehip_netdev_t *netdev);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _NETDEV_TRAIT_H_

