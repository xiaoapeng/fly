/**
 * @file common_ringbuffer.c
 * @brief 比较常见的环形缓冲区实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-08-28
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */
#include <string.h>

#include "typedef.h"

#include "common_ringbuffer.h"

#define _er_malloc MALLOC
#define _er_free   FREE

#define crb_fix(ptr, size)	((ptr)%(size))

static inline bool crb_full(Crb *fifo)
{
	if(crb_fix(fifo->write+1, fifo->mem_size) == fifo->read)
		return true;
	return false;
}

static inline bool crb_empty(Crb *fifo)
{
	if(fifo->write == fifo->read)
		return true;
	return false;
}

/**
 * @brief 计算已经使用的缓冲区大小
 * @param  fifo             句柄
 * @return uint32_t 
 */
uint32_t crb_Size(Crb *fifo)
{
	uint32_t write = fifo->write;
	uint32_t read = fifo->read;
	write = read > write ? write + fifo->mem_size : write;
	return write - read;
}

/**
 * @brief 计算可用缓冲区的大小
 * @param  fifo            句柄
 * @return uint32_t 
 */
uint32_t crb_FreeSize(Crb* fifo)
{
	return fifo->mem_size - crb_Size(fifo) - 1;
}

/**
 * @brief  完全清空环形缓冲区
 * @param  fifo             句柄
 */
void crb_Clear(Crb* fifo)
{
	fifo->read = fifo->write;
}

/**
 * @brief  读了个空气，只对读指针产生偏移，不拿出数据
 * @param  fifo             句柄
 * @param  buf_size         读出空气的大小
 * @return uint32_t 
 */
uint32_t crb_ReadAir(Crb* fifo, uint32_t buf_size)
{
	uint32_t br=0;
	br = crb_Size(fifo);
	br = br > buf_size ? buf_size : br ;
	fifo->read = crb_fix(fifo->read + br, fifo->mem_size);
	return br;
}

/**
 * @brief 写环形缓冲区
 * @param  fifo             句柄
 * @param  buf              要写的数据
 * @param  buf_size         要写的数据大小
 * @return uint32_t 
 */
uint32_t crb_Write(Crb* fifo,const uint8_t *buf, uint32_t buf_size)
{
	uint32_t wr = 0;
	uint32_t wr_first=0;

	if(buf == NULL || buf_size == 0) return 0;
	if(crb_full(fifo))	return 0;
	
	wr = crb_FreeSize(fifo);
	if(wr == 0) return 0;
	wr = wr > buf_size ? buf_size : wr;
	wr_first = wr > fifo->mem_size-fifo->write ? fifo->mem_size-fifo->write : wr;
	memcpy(fifo->mem+fifo->write, buf, wr_first);
	if(wr-wr_first)
		memcpy(fifo->mem+0, buf+wr_first, wr-wr_first);
	/* 移动 */
	fifo->write = crb_fix(fifo->write + wr, fifo->mem_size);
	return wr;
}


/**
 * @brief  读缓冲区的数据,放入buf缓冲区中
 * @param  fifo             句柄
 * @param  buf              存放字节流的指针
 * @param  buf_size         缓冲区大小
 * @return uint32_t 		返回读到的缓冲区大小
 */
uint32_t crb_Read(Crb* fifo, uint8_t *buf, uint32_t buf_size)
{
	uint32_t br=0;
	uint32_t br_first=0;
	/* 为空 */
	if(crb_empty(fifo)) return 0;
	
	br = crb_Size(fifo);
	br = br > buf_size ? buf_size : br ;
	br_first = br > fifo->mem_size-fifo->read ? fifo->mem_size-fifo->read : br;
	memcpy(buf, fifo->mem+fifo->read, br_first);
	if(br-br_first)
		memcpy(buf+br_first, fifo->mem+0, br-br_first);
	/* 移动 */
	fifo->read = crb_fix(fifo->read + br, fifo->mem_size);
	return br;
}

/**
 * @brief  偷看环形缓冲区的数据，但不进行偏移
 * @param  fifo             句柄
 * @param  buf              存放字节流的指针
 * @param  buf_size         缓冲区大小
 * @return uint32_t 		返回读到的缓冲区大小
 */
uint32_t crb_Peep(Crb* fifo, uint8_t *buf, uint32_t buf_size)
{
	uint32_t br=0;
	uint32_t br_first=0;
	/* 为空 */
	if(crb_empty(fifo)) return 0;
	
	br = crb_Size(fifo);
	br = br > buf_size ? buf_size : br ;
	br_first = br > fifo->mem_size-fifo->read ? fifo->mem_size-fifo->read : br;
	memcpy(buf, fifo->mem+fifo->read, br_first);
	if(br-br_first)
		memcpy(buf+br_first, fifo->mem+0, br-br_first);
	return br;
}



/**
 * @brief 新建一个环形缓冲区
 * @param  size             环形缓冲区的容量
 * @return Crb* 			返回一个实例
 */
Crb* crb_New(uint32_t size)
{
	Crb *crb_new;
	crb_new = (Crb *)_er_malloc(sizeof(Crb)+size * 2);
	if( crb_new == NULL )
		return NULL;
	crb_new->mem 	  	= (uint8_t *)(crb_new+1);	
	crb_new->mem_size 	= size;
	crb_new->read 		= crb_new->write = 0;
	crb_new->is_static  = 0;
	return crb_new;
}

/**
 * @brief 使用静态的方式创建crb
 * @param  crb             	外部实例化
 * @param  buf              静态缓冲区（生命周期长的栈上缓冲区）
 * @param  size             缓冲区大小
 * @return int 
 */
int crb_StaticNew(Crb* crb, uint8_t* buf, uint32_t size){
	if(buf == NULL || crb == NULL || size <= 1) 
		return -1;
	crb->mem 	  	= buf;
	crb->mem_size 	= size;
	crb->read 		= crb->write = 0;
	crb->is_static  = 1;
	return 0;
}



/**
 * @brief 删除环形缓冲区
 * @param  fifo             句柄
 */
void crb_Del(Crb* fifo)
{
    if(!fifo->is_static)
	    _er_free(fifo);
}
