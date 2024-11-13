/**
 * @file hw_addr.h
 * @brief 定义硬件地址结构
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-07
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _HW_ADDR_H_
#define _HW_ADDR_H_

#include <stdint.h>
#include "eh_types.h"

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#define EHIP_GENERAL_HW_ADDR_LEN  6
struct ehip_general_hw_addr{
    uint8_t addr[EHIP_GENERAL_HW_ADDR_LEN];
}eh_aligned(1);

typedef void ehip_hw_addr_t;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _HW_ADDR_H_