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


extern int ehip_netdev_tool_up(ehip_netdev_t *netdev);
extern int ehip_netdev_tool_down(ehip_netdev_t *netdev);
extern int ehip_netdev_tool_ctrl(ehip_netdev_t *netdev, uint32_t ctrl, void *arg);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_NETDEV_TOOL_H_

