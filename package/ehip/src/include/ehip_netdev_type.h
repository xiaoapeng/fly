/**
 * @file ehip_netdev_type.h
 * @brief  网络设备类型
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _EHIP_NETDEV_TYPE_H_
#define _EHIP_NETDEV_TYPE_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

enum ehip_netdev_type{
    EHIP_NETDEV_TYPE_ETHERNET,
    EHIP_NETDEV_TYPE_PPP,
    EHIP_NETDEV_TYPE_LOOPBACK,
    EHIP_NETDEV_TYPE_MAX,
};




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_TYPE_H_