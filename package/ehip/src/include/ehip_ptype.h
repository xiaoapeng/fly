/**
 * @file ehip_ptype.h
 * @brief 协议类型
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-10-20
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_PTYPE_H_
#define _EHIP_PTYPE_H_

#include <eh_swab.h>
#include <ehip-mac/ethernet_type.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum ehip_ptype{
    /* Non DIX types,MAX = 0x0600 */
    EHIP_PTYPE_ETHERNET_II_FRAME = eh_hton16(0x0000),

    /* These are the defined Ethernet Protocol ID's. */
    EHIP_PTYPE_ETHERNET_IP = eh_hton16(EHIP_ETH_P_IP),
    EHIP_PTYPE_ETHERNET_ARP = eh_hton16(EHIP_ETH_P_ARP),
};





#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_PTYPE_H_