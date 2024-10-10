/**
 * @file ehip_netdev.h
 * @brief 提供网卡注册接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */

#ifndef _EHIP_NETDEV_H_
#define _EHIP_NETDEV_H_

#include <stddef.h>
#include <stdint.h>
typedef void * ehip_netdev_t;

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


enum ehip_netdev_type{
    EHIP_NETDEV_TYPE_ETHERNET,
};

struct ehip_netdev_ops{
};

struct ehip_netdev_param{
    char                    *name;
    enum ehip_netdev_type    netdev_type;
    struct ehip_netdev_ops  *ops;
    void                    *userdata;
    uint16_t                 net_frame_size;
    
};


ehip_netdev_t ehip_netdev_register(struct ehip_netdev_param *param);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_H_