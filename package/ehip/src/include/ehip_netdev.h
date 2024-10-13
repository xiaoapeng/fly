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
#include <stdbool.h>

#include "eh_error.h"
#include "eh_list.h"

#include "ehip_buffer.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

typedef struct ehip_netdev ehip_netdev_t;

enum ehip_netdev_type{
    EHIP_NETDEV_TYPE_ETHERNET,
};

struct ehip_netdev{
    struct eh_list_head node;
    struct ehip_netdev_param *param;
};


struct ehip_netdev_ops{
    int (*ndo_open)(ehip_netdev_t *netdev);
    int (*ndo_stop)(ehip_netdev_t *netdev);
    int (*ndo_output)(ehip_netdev_t *netdev, ehip_buffer_t *buf);
    int (*ndo_set_rx_mode)(ehip_netdev_t *netdev);
    int (*ndo_set_mac_addr)(ehip_netdev_t *netdev, const uint8_t *mac);
};

struct ehip_netdev_param{
    char                    *name;
    enum ehip_netdev_type    netdev_type;
    struct ehip_netdev_ops  *ops;
    void                    *userdata;
    uint16_t                 net_frame_size;    
};

/* ########################### 网卡层调用 ################################# */

extern ehip_netdev_t ehip_netdev_register(struct ehip_netdev_param *param);
extern void ehip_netdev_unregister(ehip_netdev_t *netdev);
extern int ehip_netdev_input(ehip_netdev_t *netdev, ehip_buffer_t *buf);
extern void ehip_netdev_link_set_status(ehip_netdev_t *netdev, bool is_up);

/* ####################################################################### */


/* ############################ 数据链路层调用 ############################# */

static inline int ehip_netdev_output(ehip_netdev_t *netdev, ehip_buffer_t *buf){
    eh_param_assert(netdev->param);
    eh_param_assert(netdev->param->ops->ndo_output);
    return netdev->param->ops->ndo_output(netdev, buf);
}

/* ####################################################################### */


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_H_