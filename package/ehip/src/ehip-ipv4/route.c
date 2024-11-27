/**
 * @file route.c
 * @brief 路由表实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-18
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
 

#include "ehip_netdev_trait.h"
#include <eh.h>
#include <eh_error.h>
#include <eh_list.h>
#include <eh_signal.h>
#include <eh_event_flags.h>
#include <eh_mem.h>

#include <ehip_netdev.h>
#include <ehip_module.h>
#include <ehip-ipv4/route.h>
#include <ehip-ipv4/ip.h>
#include <stdint.h>

static struct eh_list_head      route_head;
static size_t                   route_cnt;

struct route_table_entry{
    struct eh_list_head     node;                               /* 链表节点 */
    eh_signal_slot_t        slot_netdev_status_change;          /* 网卡状态变化信号槽 */
    struct route_info       route;                              /* 路由表项 */
};


static void _ehip_ipv4_route_delete(struct route_table_entry *entry){
    eh_signal_slot_disconnect(&entry->slot_netdev_status_change);
    eh_list_del(&entry->node);
    eh_free(entry);
    route_cnt--;
    /* 如果有哈希表缓存，则需要清掉所有缓存 */
}


static void netdev_status_change(eh_event_t *e, void *slot_param){
    (void)e;
    struct route_table_entry *entry = slot_param;
    if(eh_event_flags_get(eh_signal_to_custom_event(&entry->route.netdev->signal)) & EHIP_NETDEV_STATUS_UP)
        return ;
    
    /* 删除此条路由 */
    _ehip_ipv4_route_delete(entry);
}

static struct route_table_entry * ehip_ipv4_route_find(const struct route_info *route){
    struct route_table_entry *pos;
    eh_list_for_each_entry(pos, &route_head, node){
        if( pos->route.dst_addr == route->dst_addr &&
            (pos->route.gateway == 0 || pos->route.gateway == route->gateway) &&
            (pos->route.mask_len == 0 || pos->route.mask_len == route->mask_len) &&
            (pos->route.netdev == NULL || pos->route.netdev == route->netdev) &&
            (pos->route.src_addr == IPV4_ADDR_ANY || pos->route.src_addr == route->src_addr) && 
            (pos->route.metric == 0 || pos->route.metric == route->metric)
        )
            return pos;
    }
    return NULL;
}

static uint32_t ehip_ipv4_route_entry_match_level(ipv4_addr_t dst_addr, struct route_table_entry *entry){
    uint32_t level = 0;
    uint32_t mask = ipv4_mask_len_to_mask(entry->route.mask_len);
    if((entry->route.dst_addr & mask) != (dst_addr & mask))
        return 0;
    /* 目标匹配 */

    /* 
     * 1. 根据最长匹配原则， mask越长，匹配等级越高
     * 2. 如果mask相同，优先匹配直连路由
     * 3. 如果都是直连路由，则根据metric值从小到大排序
     * 由此方法生成一个优先级别
     */
    level = (uint32_t)entry->route.mask_len << 24;

    if(!entry->route.gateway)
        level |= 1 << 16;
    level |= (uint32_t)(UINT16_MAX - entry->route.metric);
    return level;
}

int ipv4_route_add(const struct route_info *route){
    struct route_table_entry *entry;
    
    if(ehip_ipv4_route_find(route))
        return EH_RET_EXISTS;
    
    entry = eh_malloc(sizeof(struct route_table_entry));
    if(entry == NULL)
        return  EH_RET_MALLOC_ERROR;
    eh_list_head_init(&entry->node);
    entry->route = *route;
    
    eh_list_add_tail(&entry->node, &route_head);
    eh_signal_slot_init(&entry->slot_netdev_status_change, netdev_status_change, entry);
    /* 注册当网络状态 DOWN时删除改路由 */
    eh_signal_slot_connect(&entry->route.netdev->signal, &entry->slot_netdev_status_change);
    route_cnt ++;
    return 0;
}


int ipv4_route_del(const struct route_info *route){
    struct route_table_entry *entry;
    entry = ehip_ipv4_route_find(route);
    if(entry == NULL)
        return EH_RET_INVALID_PARAM;
    _ehip_ipv4_route_delete(entry);
    return 0;
}

enum route_table_type ipv4_route_lookup(ipv4_addr_t dst_addr, struct route_info *route){
    struct route_table_entry *pos;
    struct route_table_entry *best = NULL;
    uint32_t match_level = 0;               /* match level 越高则匹配优先级越高 */
    uint32_t match_level_tmp = 0;           /* match level 越高则匹配优先级越高 */

    if(ipv4_is_global_bcast(dst_addr) || ipv4_is_zeronet(dst_addr)){
        return ROUTE_TABLE_BROADCAST;
    }

    if(ipv4_is_multicast(dst_addr))
        return ROUTE_TABLE_MULTICAST;

    if(ipv4_netdev_is_local_addr(dst_addr))
        return ROUTE_TABLE_LOCAL;

    /* 如果设计了哈希表，先查哈希表，没有就遍历路由表 */

    eh_list_for_each_entry(pos, &route_head, node){
        match_level_tmp = ehip_ipv4_route_entry_match_level(dst_addr, pos);
        if(match_level_tmp > match_level){
            match_level = match_level_tmp;
            best = pos;
        }
    }

    if(!best)
        return ROUTE_TABLE_UNREACHABLE;
    *route = best->route;
    if(route->mask_len < 32 && ipv4_is_local_broadcast(dst_addr, route->mask_len))
        return ROUTE_TABLE_ANYCAST;
    return ROUTE_TABLE_UNICAST;
}

ipv4_addr_t ipv4_route_best_src_ip(const struct route_info *route){
    ipv4_addr_t src_ip;
    struct ipv4_netdev* ipv4_dev = ehip_netdev_trait_ipv4_dev(route->netdev);
    if(eh_ptr_to_error(ipv4_dev) < 0)
        return IPV4_ADDR_ANY;
    if(ipv4_netdev_is_ipv4_addr_valid(ipv4_dev, route->src_addr))
        return route->src_addr;
    if(route->mask_len && (src_ip = ipv4_netdev_get_best_ipv4_addr(ipv4_dev, route->dst_addr, route->mask_len)))
        return src_ip;
    return ipv4_netdev_get_addr(ipv4_dev);
}

static int __init ehip_ipv4_route_init(void)
{
    route_cnt = 0;
    eh_list_head_init(&route_head);
    return 0;
}

ehip_preinit_module_export(ehip_ipv4_route_init, NULL);

