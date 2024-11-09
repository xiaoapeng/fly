/**
 * @file crc_check.h
 * @brief CRC算法
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */

#ifndef _CRC_CHECK_H_
#define _CRC_CHECK_H_

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */


extern uint16_t crc16(uint16_t init_val,const uint8_t* msg, uint16_t msg_len);
extern uint32_t crc32(uint32_t init_val, const uint8_t* msg, uint32_t msg_len);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _CRC_CHECK_H_