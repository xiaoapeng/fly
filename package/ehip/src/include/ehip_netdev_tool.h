/**
 * @file ehip_netdev_tool.h
 * @brief 网络设备控制工具
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-05
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_NETDEV_TOOL_H_
#define _EHIP_NETDEV_TOOL_H_


#include <ehip_netdev.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * @brief                   网络设备 UP
 * @param  netdev
 * @return int 
 */
static inline int ehip_netdev_tool_up(ehip_netdev_t *netdev){
    return ehip_netdev_up(netdev);
}

/**
 * @brief                   网络设备 DOWN
 * @param  netdev
 * @return int 
 */
static inline void ehip_netdev_tool_down(ehip_netdev_t *netdev){
    ehip_netdev_down(netdev);
}


/**
 * @brief                   网络设备控制
 * @param  netdev
 * @param  ctrl
 * @param  arg
 * @return int 
 */
extern int ehip_netdev_tool_ctrl(ehip_netdev_t *netdev, uint32_t ctrl, void *arg);

/**
 * @brief                   根据网卡名称获取网卡句柄
 * @param  netdev_name      网卡注册时名称
 * @return ehip_netdev_t* 
 */
static inline ehip_netdev_t * ehip_netdev_tool_find(char *netdev_name){
    return ehip_netdev_find(netdev_name);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_TOOL_H_
