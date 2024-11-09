/**
 * @file elastic_ringbuffer.c
 * @brief 
 *		环形缓冲器特殊实现，可直接在缓冲器内存上做数据解析，无需额外的拷贝和额外的缓冲区消耗。
 *		缺点是需要两倍的内存来运行。
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2021-03-01
 * 
 * @copyright Copyright (c) 2021  simon.xiaoapeng@gmail.com
 * 

 */
#include <string.h>

#include "typedef.h"

#include "elastic_ringbuffer.h"

#define _er_malloc MALLOC
#define _er_free   FREE


#define erb_fix(ptr, size)	((ptr)%(size))

static inline bool erb_full(Erb *fifo)
{
	if(erb_fix(fifo->write+1, fifo->mem_size) == fifo->read)
		return true;
	return false;
}

static inline bool erb_empty(Erb *fifo)
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
uint32_t erb_Size(Erb *fifo)
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
uint32_t erb_FreeSize(Erb* fifo)
{
	return fifo->mem_size - erb_Size(fifo) - 1;
}

/**
 * @brief  读了个空气，只对读指针产生偏移，不拿出数据
 * @param  fifo             句柄
 * @param  buf_size         读出空气的大小
 * @return uint32_t 
 */
uint32_t erb_ReadAir(Erb* fifo, uint32_t buf_size)
{
	uint32_t br=0;
	br = erb_Size(fifo);
	br = br > buf_size ? buf_size : br ;
	fifo->read = erb_fix(fifo->read + br, fifo->mem_size);
	return br;
}

/**
 * @brief  完全清空环形缓冲区
 * @param  fifo             句柄
 */
void erb_Clear(Erb* fifo)
{
	fifo->read = fifo->write;
}

/**
 * @brief 	偷偷观看，观看缓冲区可观看的数据,
 * @param  fifo             		句柄
 * @param  expect_peep_size 		想偷看数据的大小
 * @param  reality_peep_sizeMy 		想偷看数据的大小
 * @return const uint8_t* 			返回观看字节流数据的指针，注意该指针是只读的
 */
const uint8_t* erb_Peep(Erb* fifo, uint32_t expect_peep_size, uint32_t* reality_peep_size)
{
	uint32_t br=0;
	br = erb_Size(fifo);
	br = br > expect_peep_size ? expect_peep_size : br ;
	if(reality_peep_size)
		*reality_peep_size = br;
	return fifo->mem+fifo->read;
}

/**
 * @brief 偷偷观看所有数据
 * @param  fifo             句柄
 * @param  peep_size        偷看数据大大小
 * @return const uint8_t* 
 */
const uint8_t* erb_PeepAll(Erb* fifo, uint32_t* peep_size)
{
	uint32_t br=0;
	br = erb_Size(fifo);
	if(peep_size)
		*peep_size = br;
	return fifo->mem+fifo->read;
}

/**
 * @brief  读缓冲区的数据,放入buf缓冲区中
 * @param  fifo             句柄
 * @param  buf              存放字节流的指针
 * @param  buf_size         缓冲区大小
 * @return uint32_t 		返回读到的缓冲区大小
 */
uint32_t erb_Read(Erb* fifo, uint8_t *buf, uint32_t buf_size)
{
	uint32_t br=0;
	/* 为空 */
	if(erb_empty(fifo)) return 0;
	
	br = erb_Size(fifo);
	br = br > buf_size ? buf_size : br ;
	memcpy(buf, fifo->mem+fifo->read, br);
	/* 移动 */
	fifo->read = erb_fix(fifo->read + br, fifo->mem_size);
	return br;
}

/**
 * @brief 写环形缓冲区
 * @param  fifo             句柄
 * @param  buf              要写的数据
 * @param  buf_size         要写的数据大小
 * @return uint32_t 
 */
uint32_t erb_Write(Erb* fifo, uint8_t *buf, uint32_t buf_size)
{
	uint32_t wr = 0;

	if(buf == NULL || buf_size == 0) return 0;
	if(erb_full(fifo))	return 0;
	
	wr = erb_FreeSize(fifo);
	if(wr == 0) return 0;
	wr = wr > buf_size ? buf_size : wr;
	memcpy(fifo->mem+fifo->write, buf, wr);

	/* 进行环回 */
	if(fifo->write + wr > (uint32_t)fifo->mem_size)
		memcpy(fifo->mem, fifo->mem_tmp, (fifo->write + wr) - fifo->mem_size);
	else
		memcpy(fifo->mem_tmp + fifo->write, buf, wr);
	/* 移动 */
	fifo->write = erb_fix(fifo->write + wr, fifo->mem_size);
	return wr;
}

/**
 * @brief 新建一个环形缓冲区
 * @param  size             环形缓冲区的容量
 * @return Erb* 			返回一个实例
 */
Erb* erb_New(uint32_t size)
{
	Erb *erb_new;
	erb_new = (Erb *)_er_malloc(sizeof(Erb)+size * 2);
	if( erb_new == NULL )
	{
		return NULL;
	}
	erb_new->mem 	  	= (uint8_t *)(erb_new+1);	
	erb_new->mem_tmp 	= erb_new->mem + size;
	erb_new->mem_size 	= size;
	erb_new->read 		= erb_new->write = 0;
	erb_new->is_static  = 0;
	return erb_new;
}

/**
 * @brief 静态方式创建环形缓冲区
 * @param  fifo             实例化后的指针
 * @param  buf              环形缓冲区的缓冲区指针，实际大小必须是size的两倍
 * @param  size             环形缓冲区大小，必须是实际使用buf大小的一半
 * @return int 
 */
int erb_StaticNew(Erb* fifo, uint8_t* buf, uint32_t size)
{
	fifo->mem 	  	= buf;	
	fifo->mem_tmp 	= buf + size;
	fifo->mem_size 	= size;
	fifo->read 		= fifo->write = 0;
	fifo->is_static  = 1;
	return 0;
}


/**
 * @brief 删除环形缓冲区
 * @param  fifo             句柄
 */
void erb_Del(Erb* fifo)
{
	if(!fifo->is_static)
		_er_free(fifo);
}
