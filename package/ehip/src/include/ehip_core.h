/**
 * @file ehip_core.h
 * @brief ehip 核心
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-11-09
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */

#ifndef _EHIP_CORE_H_
#define _EHIP_CORE_H_

#include <ehip_buffer.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/**
 * @brief                   ehip 网络数据的入口，
 *                          这里将视为buf所有权的移交，
 *                          无论函数是否执行成功，所有权都由ehip_rx所有
 *                          若调用该函数还要继续使用Buf，请dup/ref一个新buf/新引用
 * @param  netdev_buf       网络数据Buf
 * @return int
 */
extern int ehip_rx(ehip_buffer_t *netdev_buf);




#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _EHIP_CORE_H_
