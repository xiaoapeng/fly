/**
 * @file global_signal.h
 * @brief 用户提供的一些全局信号
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2025-05-23
 * 
 * @copyright Copyright (c) 2025  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _GLOBAL_SIGNAL_H_
#define _GLOBAL_SIGNAL_H_

#include <eh.h>
#include <eh_signal.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

EH_EXTERN_SIGNAL( sig_eth0_ip_add );
EH_EXTERN_SIGNAL( sig_eth0_ip_del );

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _GLOBAL_SIGNAL_H_