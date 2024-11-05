/**
 * @file ethernet_dev.h
 * @brief 以太网设备CONFIG接口
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-11-05
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */
#ifndef _ETHERNET_DEV_H_
#define _ETHERNET_DEV_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define ETHERNET_CTRL_CMD_SET_RX_FILTER_MODE   0x00000001
#define ETHERNET_CTRL_CMD_SET_MAC_ADDR         0x00000002
#define ETHERNET_CTRL_CMD_SET_MULTICAST_ADDR   0x00000003




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _ETHERNET_DEV_H_