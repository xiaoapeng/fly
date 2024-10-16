/**
 * @file ehip_module.h
 * @brief ehip module 初始导出宏定义
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @version 1.0
 * @date 2024-10-14
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 * @par 修改日志:
 */
#ifndef _EHIP_MODULE_H_
#define _EHIP_MODULE_H_

#include "eh_module.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define ehip_preinit_module_export(_init__func_, _exit__func_) _eh_define_modeule_export(_init__func_, _exit__func_, "2.0.0")
#define ehip_core_module_export(_init__func_, _exit__func_) _eh_define_modeule_export(_init__func_, _exit__func_, "2.0.1")
#define ehip_netdev_module_export(_init__func_, _exit__func_) _eh_define_modeule_export(_init__func_, _exit__func_, "2.0.2")


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_MODULE_H_