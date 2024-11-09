/**
 * @file elastic_ringbuffer.h
 * @brief
 *		环形缓冲器特殊实现，可直接在缓冲器内存上做数据解析，无需额外的拷贝和额外的缓冲区消耗。
 *		缺点是需要两倍的内存来运行。
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2021-03-01
 * 
 * @copyright Copyright (c) 2021  simon.xiaoapeng@gmail.com
 * 

 */


#ifndef __ELASTIC_RINGBUFFER_H__
#define __ELASTIC_RINGBUFFER_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef int32_t er_ssize_t;

typedef struct _Erb{
	uint8_t 	*mem;		/* 环形缓冲区使用存储空间上 */
	uint32_t 	mem_size;	/* 环形缓冲区的大小 */
	uint32_t 	write;		/* 环形缓冲区的写指针 */
	uint32_t 	read;		/* 环形缓冲区的写指针 */
	uint8_t 	*mem_tmp;	/* 某些操作使用的临时缓冲区 */
  	int         is_static;   /* 环形缓冲区是否为静态存储 */
}Erb;


extern void erb_Del(Erb* fifo);
extern Erb* erb_New(uint32_t size);
extern int erb_StaticNew(Erb* fifo, uint8_t* buf, uint32_t size);
extern uint32_t erb_Read(Erb* fifo, uint8_t *buf, uint32_t buf_size);
extern uint32_t erb_Write(Erb* fifo, uint8_t *buf, uint32_t buf_size);
extern uint32_t erb_ReadAir(Erb* fifo, uint32_t buf_size);
extern const uint8_t* erb_Peep(Erb* fifo, uint32_t expect_peep_size, uint32_t* reality_peep_size);
extern const uint8_t* erb_PeepAll(Erb* fifo, uint32_t* peep_size);
extern void erb_Clear(Erb* fifo);
extern uint32_t erb_Size(Erb *fifo);
extern uint32_t erb_FreeSize(Erb* fifo);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __ELASTIC_RINGBUFFER_H__ */
