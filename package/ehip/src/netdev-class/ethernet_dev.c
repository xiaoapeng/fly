/**
 * @file ethernet_dev.c
 * @brief 以太网网络设备特征
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <string.h>

#include <eh_error.h>
#include <ehip_module.h>
#include <ehip_conf.h>
#include <ehip_netdev_trait.h>
#include "ehip-netdev-class/ethernet_dev.h"
#include <ehip-mac/ethernet.h>

struct ethernet_trait{
    ehip_eth_addr_t                             hw_addr;
    ehip_eth_addr_t                             multicast_hw_addr[ETH_MULTICAST_ADDR_NUM];
    struct ipv4_addr                            ipv4_addr[EHIP_NETDEV_MAX_IP_NUM];
};


static int ethernet_dev_trait_get_ipv4_addr(ehip_netdev_trait_t *netdev_trait, int index, struct ipv4_addr *ipv4_addr)
{
    struct ethernet_trait *ethernet_trait = (struct ethernet_trait *)netdev_trait;
    if((unsigned int)index >= EHIP_NETDEV_MAX_IP_NUM) 
        return EH_RET_INVALID_PARAM;
    *ipv4_addr = ethernet_trait->ipv4_addr[index];
    return EH_RET_OK;
}

static int ethernet_dev_trait_set_ipv4_addr(ehip_netdev_trait_t *netdev_trait, int index, struct ipv4_addr *ipv4_addr)
{
    struct ethernet_trait *ethernet_trait = (struct ethernet_trait *)netdev_trait;
    if((unsigned int)index >= EHIP_NETDEV_MAX_IP_NUM) 
        return EH_RET_INVALID_PARAM;
    ethernet_trait->ipv4_addr[index] = *ipv4_addr;
    return EH_RET_OK;
}

static void ethernet_dev_trait_get_hw_addr(ehip_netdev_trait_t *netdev_trait, ehip_hw_addr hw_addr)
{
    struct ethernet_trait *ethernet_trait = (struct ethernet_trait *)netdev_trait;
    memcpy(hw_addr, &ethernet_trait->hw_addr, sizeof(ehip_eth_addr_t));
}

static void ethernet_dev_trait_set_hw_addr(ehip_netdev_trait_t *netdev_trait, ehip_hw_addr hw_addr)
{
    struct ethernet_trait *ethernet_trait = (struct ethernet_trait *)netdev_trait;
    memcpy(&ethernet_trait->hw_addr, hw_addr, sizeof(ehip_eth_addr_t));
}


struct ehip_netdev_trait_ops ethernet_dev_trait_ops = {
    .trait_size = sizeof(struct ethernet_trait),
    .get_hw_addr = ethernet_dev_trait_get_hw_addr,
    .set_hw_addr = ethernet_dev_trait_set_hw_addr,
    .get_ipv4_addr = ethernet_dev_trait_get_ipv4_addr,
    .set_ipv4_addr = ethernet_dev_trait_set_ipv4_addr,
    .reset = NULL,
};


int __init ethernet_dev_trait_init(void)
{
    return ehip_netdev_trait_type_install(EHIP_NETDEV_TYPE_ETHERNET, &ethernet_dev_trait_ops);
}

ehip_preinit_module_export(ethernet_dev_trait_init, NULL);
