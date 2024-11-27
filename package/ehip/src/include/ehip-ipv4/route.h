/**
 * @file route.h
 * @brief ipv4 route实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _ROUTE_H_
#define _ROUTE_H_

#include <ehip-ipv4/ip.h>
#include <eh_list.h>
#include <eh_llist.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum route_table_type{
    ROUTE_TABLE_UNREACHABLE,          /* 网络不可达 */
    ROUTE_TABLE_MULTICAST,            /* 多播 */
    ROUTE_TABLE_BROADCAST,            /* 广播 */
    ROUTE_TABLE_UNICAST,              /* 单播 */
    ROUTE_TABLE_LOCAL,                /* 本地地址 */
    ROUTE_TABLE_ANYCAST,              /* 发送时单播，接收时作为广播  */
};

struct route_info{
    struct ehip_netdev     *netdev;                 /* 路由项指向的网卡 */
    ipv4_addr_t             dst_addr;               /* 目标 */
    ipv4_addr_t             src_addr;               /* 网关 */
    ipv4_addr_t             gateway;                /* 网关 */
    uint16_t                metric;                 /* 路由条目的优先级 */
    uint8_t                 mask_len;               /* 目标掩码长度 */
};

/**
 * @brief                   添加一条路由
 * @param  route            路由信息
 * @return int              成功返回0
 */
extern int ipv4_route_add(const struct route_info *route);

/**
 * @brief                   删除一条路由表项
 * @param  route            路由信息
 * @return int              成功返回0
 */
extern int ipv4_route_del(const struct route_info *route);

/**
 * @brief                   将路由表转成数组项
 * @param  route_array      使用完后需要释放
 * @return int 
 */
extern int ipv4_route_to_array(struct route_info **route_array);

/**
 * @brief                   查找路由
 * @param  dst_addr         目标地址
 * @param  route            路由表项信息
 * @return int              成功返回  ROUTE_TABLE_XXX
 */
extern enum route_table_type ipv4_route_lookup(ipv4_addr_t dst_addr, struct route_info *route);

/**
 * @brief                   查找路由表项的源ip
 * @param  route            路由表项信息，由 ipv4_route_lookup 参数 返回的结果
 * @return ipv4_addr_t      成功返回源ip地址 失败返回0
 */
extern ipv4_addr_t ipv4_route_best_src_ip(const struct route_info *route);





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ROUTE_H_