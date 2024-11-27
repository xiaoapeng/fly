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
#include <eh_error.h>
#include <eh_types.h>
#include <ehip_netdev_trait.h>

static struct  ehip_netdev_trait_ops const *type_tab[EHIP_NETDEV_TYPE_MAX];

static void* ehip_netdev_offset_to_trait_addr(ehip_netdev_t * netdev, uint16_t offset){
    if(offset < sizeof(ehip_netdev_t))
        return eh_error_to_ptr(EH_RET_NOT_SUPPORTED);
    return (void*)(((char*)netdev) + offset);
}

#define EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(name, return_type_ptr)                                                    \
return_type_ptr ehip_netdev_trait_##name(ehip_netdev_t *netdev)                                                         \
{                                                                                                                       \
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];                                                  \
    if((uint32_t)netdev->type >= EH_ARRAY_SIZE(type_tab))                                                               \
        return eh_error_to_ptr(EH_RET_INVALID_STATE);                                                                   \
    return (return_type_ptr)ehip_netdev_offset_to_trait_addr(netdev, ops->name##_offset);                               \
}

#define EHIP_NETDEV_ARRAY_TRAIT_DEFINE_FUNC(name, return_type_ptr, step_len)                                            \
return_type_ptr ehip_netdev_trait_##name(ehip_netdev_t *netdev, int index)                                              \
{                                                                                                                       \
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];                                                  \
    if((uint32_t)netdev->type >= EH_ARRAY_SIZE(type_tab) || (unsigned long)step_len <= 0)                               \
        return eh_error_to_ptr(EH_RET_INVALID_STATE);                                                                   \
    return (return_type_ptr)                                                                                            \
        (((char*)ehip_netdev_offset_to_trait_addr(netdev, ops->name##_offset)) +                                        \
         ((size_t)step_len * (size_t)index));                                                                           \
}

#define EHIP_NETDEV_CONST_INT_VAL_TRAIT_DEFINE_FUNC(name)                                                               \
int ehip_netdev_trait_##name(const ehip_netdev_t *netdev)                                                               \
{                                                                                                                       \
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];                                                  \
    if((uint32_t)netdev->type >= EH_ARRAY_SIZE(type_tab))                                                               \
        return EH_RET_INVALID_STATE;                                                                                    \
    return (int)ops->name;                                                                                              \
}


size_t ehip_netdev_trait_size_get(enum ehip_netdev_type type){
    return type_tab[type]->trait_size;
}

int ehip_netdev_trait_type_install(enum ehip_netdev_type type, const struct ehip_netdev_trait_ops *ops){
    eh_param_assert(type < EHIP_NETDEV_TYPE_MAX);
    type_tab[type] = ops;
    return 0;
}

int ehip_netdev_trait_reset(ehip_netdev_t *netdev){
    ehip_netdev_trait_t * trait;
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->reset){
        ops->reset(netdev);
        return 0;
    }
    trait = ehip_netdev_to_trait(netdev);
    memset(trait, 0, ops->trait_size);
    return 0;
}

int ehip_netdev_trait_up(ehip_netdev_t *netdev){
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->up)
        return ops->up(netdev);
    return 0;
}

extern void ehip_netdev_trait_down(ehip_netdev_t *netdev){
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->down)
        ops->down(netdev);
}

int ehip_netdev_trait_change(ehip_netdev_t *netdev, const void *type_ptr, const void *src_ptr){
    const struct ehip_netdev_trait_ops * ops = type_tab[netdev->type];
    if(ops && ops->change){
        return ops->change(netdev, type_ptr, src_ptr);
    }
    return EH_RET_NOT_SUPPORTED;
}


EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(hw_head_size, const uint16_t*)
EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(hw_tail_size, const uint16_t*)
EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(mtu, const uint16_t*)
EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(hw_addr, const ehip_hw_addr_t*)
EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(mac_ptype, const enum ehip_ptype*)
EHIP_NETDEV_VAR_PTR_TRAIT_DEFINE_FUNC(ipv4_dev, struct ipv4_netdev*)
EHIP_NETDEV_CONST_INT_VAL_TRAIT_DEFINE_FUNC(hw_addr_len);
EHIP_NETDEV_ARRAY_TRAIT_DEFINE_FUNC(multicast_hw, const ehip_hw_addr_t*,  ehip_netdev_trait_hw_addr_len(netdev))
