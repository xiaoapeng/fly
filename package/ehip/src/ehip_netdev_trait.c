/**
 * @file ehip_netdev_trait.c
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#include <string.h>
#include "ehip_netdev_trait.h"
#include "eh_error.h"

static struct ehip_netdev_trait_ops *type_tab[EHIP_NETDEV_TYPE_MAX];

#define ehip_netdev_get_trait(netdev) (((ehip_netdev_trait_t)(netdev + 1)))


size_t ehip_netdev_trait_size_get(enum ehip_netdev_type type){
    return type_tab[type]->trait_size;
}

int ehip_netdev_trait_type_install(enum ehip_netdev_type type, struct ehip_netdev_trait_ops *ops){
    eh_param_assert(type < EHIP_NETDEV_TYPE_MAX);
    type_tab[type] = ops;
    return 0;
}

int ehip_netdev_trait_reset(ehip_netdev_t *netdev){
    ehip_netdev_trait_t * trait = ehip_netdev_get_trait(netdev);
    struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->reset){
        ops->reset(trait);
        return 0;
    }
    memset(trait, 0, ops->trait_size);
    return 0;
}

int ehip_netdev_trait_get_hw_addr(ehip_netdev_t *netdev, ehip_hw_addr hw_addr){
    ehip_netdev_trait_t * trait = ehip_netdev_get_trait(netdev);
    struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->get_hw_addr){
        ops->get_hw_addr(trait, hw_addr);
        return 0;
    }
    return EH_RET_NOT_SUPPORTED;
}

int ehip_netdev_trait_set_hw_addr(ehip_netdev_t *netdev, ehip_hw_addr hw_addr){
    ehip_netdev_trait_t * trait = ehip_netdev_get_trait(netdev);
    struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->set_hw_addr){
        ops->set_hw_addr(trait, hw_addr);
        return 0;
    }
    return EH_RET_NOT_SUPPORTED;
}


int ehip_netdev_trait_get_ipv4_addr(ehip_netdev_t *netdev, int index, struct ipv4_addr *ipv4_addr){
    ehip_netdev_trait_t * trait = ehip_netdev_get_trait(netdev);
    struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->get_ipv4_addr)
        return ops->get_ipv4_addr(trait, index, ipv4_addr);
    return EH_RET_NOT_SUPPORTED;

}

int ehip_netdev_trait_set_ipv4_addr(ehip_netdev_t *netdev, int index, struct ipv4_addr *ipv4_addr){
    ehip_netdev_trait_t * trait = ehip_netdev_get_trait(netdev);
    struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->set_ipv4_addr)
        return ops->set_ipv4_addr(trait, index, ipv4_addr);
    return EH_RET_NOT_SUPPORTED;
}