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
#include <eh_types.h>
#include <ehip_module.h>
#include <ehip_ptype.h>

#include <ehip-netdev-class/ethernet_dev.h>


ehip_netdev_trait_static_assert(struct ethernet_trait);

static void ethernet_dev_trait_reset(ehip_netdev_t *netdev);
static int ethernet_dev_trait_change(ehip_netdev_t *netdev, const void *type_ptr, const void *src_ptr);
const struct ehip_netdev_trait_ops ethernet_dev_trait_ops = {
    .trait_size = sizeof(struct ethernet_trait),
    .reset = ethernet_dev_trait_reset,
    .change = ethernet_dev_trait_change,
    .hw_addr_offset = ehip_netdev_trait_offsetof(struct ethernet_trait, hw_addr),
    .ipv4_dev_offset = ehip_netdev_trait_offsetof(struct ethernet_trait, ipv4_netdev),
    .multicast_hw_offset = ehip_netdev_trait_offsetof(struct ethernet_trait, multicast_hw_addr),
    .mac_ptype_offset = ehip_netdev_trait_offsetof(struct ethernet_trait, mac_ptype),
    .hw_addr_len = sizeof(ehip_eth_addr_t),
};

int ethernet_dev_trait_change(ehip_netdev_t *netdev, const void *type_ptr, const void *src_ptr){
    struct ethernet_trait *netdev_ethernet_trait = (struct ethernet_trait *)ehip_netdev_to_trait(netdev);
    long long_offset = (long)type_ptr-(long)netdev;
    uint16_t offset;
    int ret = 0;
    if(long_offset < (long)sizeof(ehip_netdev_t) || long_offset >= (long)(sizeof(ehip_netdev_t) + sizeof(struct ethernet_trait))){
        return EH_RET_INVALID_PARAM;
    }
    offset = (uint16_t)long_offset;
    switch (offset) {
        case ehip_netdev_trait_offsetof(struct ethernet_trait, hw_head_size):
        case ehip_netdev_trait_offsetof(struct ethernet_trait, hw_tail_size):
            ret = EH_RET_NOT_SUPPORTED;
            break;
        case ehip_netdev_trait_offsetof(struct ethernet_trait, mtu):{
            uint16_t new_mtu = *(uint16_t*)src_ptr;
            if(new_mtu < 46 || (new_mtu + netdev_ethernet_trait->hw_tail_size + netdev_ethernet_trait->hw_head_size) > netdev->param->net_max_frame_size){
                ret = EH_RET_NOT_SUPPORTED;
                break;
            }
            netdev_ethernet_trait->mtu = new_mtu;
            break;
        }
        case ehip_netdev_trait_offsetof(struct ethernet_trait, hw_addr):
            /* mac 地址被修改 */
            memcpy((void*)type_ptr, src_ptr, sizeof(netdev_ethernet_trait->hw_addr));
            break;
        case ehip_netdev_trait_offsetof(struct ethernet_trait, mac_ptype):{
            enum ehip_ptype new_mac_ptype = *(enum ehip_ptype*)src_ptr;
            /* 数据链路层协议修改 */
            /* 只支持此 ETHERNET_II */
            if(new_mac_ptype != EHIP_PTYPE_ETHERNET_II_FRAME){
                ret = EH_RET_NOT_SUPPORTED;
                break;
            }
            netdev_ethernet_trait->mac_ptype = new_mac_ptype;
            netdev_ethernet_trait->hw_head_size = sizeof(struct eth_hdr);
            break;
        }
        default:
            if( offset >= ehip_netdev_trait_offsetof(struct ethernet_trait, multicast_hw_addr) && 
                offset <  (ehip_netdev_trait_offsetof(struct ethernet_trait, 
                    multicast_hw_addr) + (ETH_MULTICAST_ADDR_NUM * sizeof(ehip_eth_addr_t)))){
                /* 多播地址被修改 */
                break;
            }
            ret = EH_RET_INVALID_PARAM;
            break;
    }
    return ret;
}

static void ethernet_dev_trait_reset(ehip_netdev_t *netdev){
    struct ethernet_trait *netdev_ethernet_trait = (struct ethernet_trait *)ehip_netdev_to_trait(netdev);
    memset(netdev_ethernet_trait, 0, sizeof(struct ethernet_trait));
    netdev_ethernet_trait->hw_head_size = sizeof(struct eth_hdr);
    netdev_ethernet_trait->hw_tail_size = 4;   /* crc */
    netdev_ethernet_trait->mtu = (uint16_t)(netdev->param->net_max_frame_size - 
        netdev_ethernet_trait->hw_head_size - netdev_ethernet_trait->hw_tail_size);
    netdev_ethernet_trait->mac_ptype = EHIP_PTYPE_ETHERNET_II_FRAME;
}

static int __init ethernet_dev_trait_init(void)
{
    return ehip_netdev_trait_type_install(EHIP_NETDEV_TYPE_ETHERNET, &ethernet_dev_trait_ops);
}

ehip_preinit_module_export(ethernet_dev_trait_init, NULL);
