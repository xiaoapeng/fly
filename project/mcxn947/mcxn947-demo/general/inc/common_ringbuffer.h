/**
 * @file common_ringbuffer.h
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-08-28
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */

#ifndef _COMMON_RINGBUFFER_H_
#define _COMMON_RINGBUFFER_H_


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>

typedef int32_t er_ssize_t;

typedef struct _Crb{
	uint8_t 	*mem;		/* 环形缓冲区使用存储空间上 */
	uint32_t 	mem_size;	/* 环形缓冲区的大小 */
	uint32_t 	write;		/* 环形缓冲区的写指针 */
	uint32_t 	read;		/* 环形缓冲区的写指针 */
    int         is_static;   /* 环形缓冲区是否为静态存储 */
}Crb;


extern void crb_Del(Crb* fifo);
extern Crb* crb_New(uint32_t size);
extern int crb_StaticNew(Crb* crb, uint8_t* buf, uint32_t size);
extern uint32_t crb_Read(Crb* fifo, uint8_t *buf, uint32_t buf_size);
extern uint32_t crb_Peep(Crb* fifo, uint8_t *buf, uint32_t buf_size);
extern uint32_t crb_Write(Crb* fifo, const uint8_t *buf, uint32_t buf_size);
extern uint32_t crb_ReadAir(Crb* fifo, uint32_t buf_size);
extern void crb_Clear(Crb* fifo);
extern uint32_t crb_Size(Crb *fifo);
extern uint32_t crb_FreeSize(Crb* fifo);



#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _COMMON_RINGBUFFER_H_