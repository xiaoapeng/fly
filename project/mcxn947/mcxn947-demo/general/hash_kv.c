/**
 * @file hash_kv.c
 * @brief 内存KV数据库实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2020-01-20
 * 
 * @copyright Copyright (c) 2020  simon.xiaoapeng@gmail.com
 * 

 */

#include <stdint.h>
#include <string.h>
#include "typedef.h"
#include "ulist.h"
#include "hash_kv.h"


unsigned int APHash(char* str, unsigned int len);
unsigned int BKDRHash(char* str, unsigned int len);
unsigned int BPHash(char* str, unsigned int len);
unsigned int DEKHash(char* str, unsigned int len);
unsigned int DJBHash(char* str, unsigned int len);
unsigned int ELFHash(char* str, unsigned int len);
unsigned int FNVHash(char* str, unsigned int len);


#define HASH_FUNC	RSHash

typedef uint32_t HashVal;

typedef struct _HashItem{
	void        	   *v;		/* 值 */
	uint32_t			v_len;
	struct list_head 	list;
	uint32_t			count;	/* 被引用次数 */
	char  				k[0];	/* 键 */
}HashItem;						/* 哈希表数据项 */

typedef struct _HashTabHead{
	struct list_head head;		/* 链表头 */
}HashTabHead;					/* 哈希表链表头数组 */

typedef struct _HashKv{
	int (*lock)(void);			/* 请提供递归锁 */
	void (*unlock)(void);
	uint32_t hash_tab_len;
	HashTabHead  hash_tab[0];
}HashKv;						/* 哈希表对象 */

typedef struct _HashBlob{
	HashKv 		*ht;
	HashItem	*data_item;
}HashBlob;						/* 哈希表数据特殊访问方式 */

/* ############################## 哈希算法集合 ############################# */

unsigned int RSHash(char* str, unsigned int len) 
{
  unsigned int b  = 378551; 
  unsigned int a  = 63689; 
  unsigned int hash = 0; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = hash * a + (*str); 
   a  = a * b; 
  } 
  
  return hash; 
}
/* End Of RS Hash Function */


unsigned int JSHash(char* str, unsigned int len) 
{ 
  unsigned int hash = 1315423911; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash ^= ((hash << 5) + (*str) + (hash >> 2)); 
  } 
  
  return hash; 
} 
/* End Of JS Hash Function */


unsigned int PJWHash(char* str, unsigned int len) 
{ 
  const unsigned int BitsInUnsignedInt = (unsigned int)(sizeof(unsigned int) * 8); 
  const unsigned int ThreeQuarters   = (unsigned int)((BitsInUnsignedInt * 3) / 4); 
  const unsigned int OneEighth     = (unsigned int)(BitsInUnsignedInt / 8); 
  const unsigned int HighBits     = (unsigned int)(0xFFFFFFFF) << (BitsInUnsignedInt - OneEighth); 
  unsigned int hash       = 0; 
  unsigned int test       = 0; 
  unsigned int i         = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = (hash << OneEighth) + (*str); 
  
   if((test = hash & HighBits) != 0) 
   { 
     hash = (( hash ^ (test >> ThreeQuarters)) & (~HighBits)); 
   } 
  }
  
  return hash; 
} 
/* End Of P. J. Weinberger Hash Function */


unsigned int ELFHash(char* str, unsigned int len) 
{ 
  unsigned int hash = 0; 
  unsigned int x  = 0; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = (hash << 4) + (*str); 
   if((x = hash & 0xF0000000L) != 0) 
   { 
     hash ^= (x >> 24); 
   } 
   hash &= ~x; 
  } 
  
  return hash; 
} 
/* End Of ELF Hash Function */


unsigned int BKDRHash(char* str, unsigned int len) 
{ 
  unsigned int seed = 131; /* 31 131 1313 13131 131313 etc.. */
  unsigned int hash = 0; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = (hash * seed) + (*str); 
  } 
  
  return hash; 
} 
/* End Of BKDR Hash Function */


unsigned int SDBMHash(char* str, unsigned int len) 
{ 
  unsigned int hash = 0; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = (*str) + (hash << 6) + (hash << 16) - hash; 
  } 
  
  return hash; 
} 
/* End Of SDBM Hash Function */


unsigned int DJBHash(char* str, unsigned int len) 
{ 
  unsigned int hash = 5381; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = ((hash << 5) + hash) + (*str); 
  } 
  
  return hash; 
} 
/* End Of DJB Hash Function */


unsigned int DEKHash(char* str, unsigned int len) 
{ 
  unsigned int hash = len; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash = ((hash << 5) ^ (hash >> 27)) ^ (*str); 
  } 
  return hash; 
} 
/* End Of DEK Hash Function */
  
  
unsigned int BPHash(char* str, unsigned int len) 
{ 
  unsigned int hash = 0; 
  unsigned int i  = 0; 
  for(i = 0; i < len; str++, i++) 
  { 
   hash = hash << 7 ^ (*str); 
  } 
  
  return hash; 
} 
/* End Of BP Hash Function */
  
  
unsigned int FNVHash(char* str, unsigned int len) 
{ 
  const unsigned int fnv_prime = 0x811C9DC5; 
  unsigned int hash   = 0; 
  unsigned int i     = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash *= fnv_prime; 
   hash ^= (*str); 
  } 
  
  return hash; 
} 
/* End Of FNV Hash Function */
  
  
unsigned int APHash(char* str, unsigned int len) 
{ 
  unsigned int hash = 0xAAAAAAAA; 
  unsigned int i  = 0; 
  
  for(i = 0; i < len; str++, i++) 
  { 
   hash ^= ((i & 1) == 0) ? ( (hash << 7) ^ (*str) * (hash >> 3)) : 
                (~((hash << 11) + ((*str) ^ (hash >> 5)))); 
  } 
  
  return hash; 
} 
/* End Of AP Hash Function */

/* ######################################################################### */




static inline HashTabHead* _HashSearch(HashKv *ht,const char* k)
{
	uint32_t hash_val = HASH_FUNC((char*)k,strlen(k)) % ht->hash_tab_len;
	return ht->hash_tab + hash_val;
}

static inline HashItem* _TabSearch(HashTabHead *item,const char* k)
{
	HashItem	*data_item;
	list_for_each_entry(data_item, &item->head , list)
	{
		if(!strcmp(data_item->k,k))
			return data_item;
	}
	return NULL;
}

/* 整体:返回值为成功字节数 */
static int _SetItemData(HashItem *data_item,const void* data,uint32_t data_len)
{
	void *v = NULL;
	if(data_item->v_len == data_len)
	{
		memcpy(data_item->v,data,data_len);
		return (int)data_len;
	}else{
		v = MALLOC(data_len);
		if(v){
			memcpy(v,data,data_len);
			FREE(data_item->v);
			data_item->v = v;
			data_item->v_len = data_len;
			return (int)data_len;
		}else{
			return -1;
		}
	}
}

/* 整体:返回值为成功字节数 */
static int _GetItemData(HashItem *data_item,void* buf,uint32_t read_size)
{
	int rb = read_size > data_item->v_len ? data_item->v_len : read_size;
	memcpy(buf, ((uint8_t*)data_item->v),rb);
	return rb;
}

#if 0
/* 分段:返回值为成功字节数 */
static int _SetItemOffsetData(HashItem *data_item,const void* data,
				uint32_t data_len,uint32_t offset)
{
	int wb = data_item->v_len - offset;	/* 可写空间 */
	if(wb <= 0) return 0;
	wb = wb > data_len? data_len : wb;
	memcpy(((uint8_t*)data_item->v)+offset,data,wb);
	return wb;
}
/* 分段:返回值为成功字节数 */
static int _GetItemOffsetData(HashItem *data_item, void* buf,
	uint32_t read_len,uint32_t offset)
{
	int rb = data_item->v_len - offset;	/* 可读数量 */
	if(rb <= 0) return 0;
	rb = rb > read_len? read_len : rb;	
	memcpy(buf, ((uint8_t*)data_item->v)+offset,rb);
	return rb;
}
#endif

static inline void _DelItemData(HashItem *data_item)
{
	list_del(&data_item->list);
	FREE(data_item->v);
	FREE(data_item);
}

static inline int _MutexLock(HashKv *ht)
{
	if(ht->lock) 
		return ht->lock();
	return 0;
}

static inline void _MutexUnLock(HashKv *ht)
{
	if(ht->unlock) 
		ht->unlock();
}

const char* hash_BlobGetKey(HashBlob_t *b)
{
	HashBlob *hb = (HashBlob*)b;
	if(!b) return NULL;
    return hb->data_item->k;
}

/*****************************************************************************
 函 数 名  : hash_BlobSetData
 功能描述  : 
 	使用特殊方式设置数据
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
 	data		数据缓冲区
 	data_len	数据大小
  返回值：
 	成功返回写入的字节数,失败返回负数
*****************************************************************************/
int hash_BlobSetData(HashBlob_t *b,const void* data,uint32_t data_len)
{
	HashBlob	*hb = (HashBlob*)b;
	int ret;

	if(!b) return -1;

	if(_MutexLock(hb->ht)) return -2;
	ret = _SetItemData(hb->data_item,data,data_len);
	_MutexUnLock(hb->ht);
	return ret;
}

/*****************************************************************************
 函 数 名  : hash_BlobGetData
 功能描述  : 
 	使用特殊方式读数据
  参数：
 	b 			由hash_AcquireBlob生成
 	buf			数据缓冲区
 	read_len		数据大小
  返回值：
 	成功返回获取的字节数,失败返回负数
*****************************************************************************/
int hash_BlobGetData(HashBlob_t * b, void * buf, uint32_t read_len)
{
	HashBlob	*hb = (HashBlob*)b;
	int ret;
	if(!b) return -1;
	if(!hb->data_item || !hb->ht ) return -1;
	if(_MutexLock(hb->ht)) return -2;
	ret = _GetItemData(hb->data_item,buf,read_len);
	_MutexUnLock(hb->ht);
	return ret;
}


/*****************************************************************************
 函 数 名  : hash_AcquireBlob
 功能描述  : 
 	获取特殊方式对象	HashBlob_t
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
 	b			输出参数，请到上层实例化
  返回值：
 	成功返回0,失败返回负数
*****************************************************************************/
int hash_AcquireBlob(HashKv_t ht, HashBlob_t *b,char* k)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *tab_item;
	HashItem	*data_item;
	HashBlob	*hb = (HashBlob*)b;
	if(!obj || !b) return -1;
	
	if(_MutexLock(obj)) return -2;
	tab_item = _HashSearch(obj,k);
	data_item = _TabSearch(tab_item,k);
	
	if(!data_item){
		_MutexUnLock(obj);
		return -1;
	}
	hb->data_item = data_item;
	hb->ht = obj;
	data_item->count++;
	_MutexUnLock(obj);
	return 0;
}

/*****************************************************************************
 函 数 名  : hash_ReleaseBlob
 功能描述  : 
 	释放HashBlob_t引用减1
  参数：
 	b			由hash_AcquireBlob生成
  返回值：
 	成功返回0,失败返回负数
*****************************************************************************/
int hash_ReleaseBlob(HashBlob_t *b)
{
	HashBlob	*hb = (HashBlob*)b;
	HashKv 		*ht;
	if(!b) return -1;

	if(!hb->data_item || !hb->ht ) return 0;
	ht = hb->ht;
	if(_MutexLock(ht)) return -2;
	if(hb->data_item->count > 0)
		hb->data_item->count--;
	hb->data_item = NULL;
	hb->ht = NULL;
	_MutexUnLock(ht);	
	return 0;
}

/*****************************************************************************
 函 数 名  : hash_DelKeyFormBlob
 功能描述  : 
 	利用Blob删除键值对,只有引用为0时才真正删除
  参数：
 	b			由hash_AcquireBlob生成
  返回值：
 	成功返回0,失败返回负数
*****************************************************************************/
int hash_DelKeyFormBlob(HashBlob_t *b)
{
	HashBlob	*hb = (HashBlob*)b;
	if(!b) return -1;

	if(!hb->data_item || !hb->ht ) return -1;

	if(_MutexLock(hb->ht)) return -2;
	if(hb->data_item->count > 0)
		hb->data_item->count--;
	hb->data_item = NULL;
	hb->ht = NULL;
	if(hb->data_item->count == 0)
		_DelItemData(hb->data_item);
	_MutexUnLock(hb->ht);
	return 0;
}



/*****************************************************************************
 函 数 名  : hash_DelKey
 功能描述  : 
 	获取值
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
  返回值：
 	成功返回0,失败返回-1
*****************************************************************************/
int hash_DelKey(HashKv_t ht,const char* k)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *tab_item;
	HashItem	*data_item;
	if(!obj) return -1;
	
	if(_MutexLock(obj)) return -2;
	tab_item = _HashSearch(obj,k);
	data_item = _TabSearch(tab_item,k);

	if(!data_item || data_item->count){
		_MutexUnLock(obj);
		return -1;
	}
	
	_DelItemData(data_item);
	_MutexUnLock(obj);
	return 0;
}

/*****************************************************************************
 函 数 名  : hash_NewData
 功能描述  : 
 	新建一个数据项
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
  返回值：
 	成功返回0,失败返回负数
*****************************************************************************/
int hash_NewData(HashKv_t ht,const char* k,uint32_t data_len)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *tab_item;
	HashItem	*data_item;
	
	if(!obj)  return -1;
	
	if(_MutexLock(obj)) return -2;
	
	tab_item = _HashSearch(obj,k);
	data_item = _TabSearch(tab_item,k);
	if(data_item)
	{
		_MutexUnLock(obj);	/* 本来就存在 */
		return -3;
	}

	/* 不存在的项,新建 */
	data_item = MALLOC(sizeof(*data_item) + strlen(k)+1);
	if(!data_item)
		goto alloc_di;
	data_item->v = MALLOC(data_len);
	if(!data_item->v)
		goto alloc_v;
	memset(data_item->v,0,data_len);
	data_item->count = 0;
	data_item->v_len = data_len;
	strcpy(data_item->k,k);
	list_add(&data_item->list, &tab_item->head);
	_MutexUnLock(obj);
	
	return 0;
alloc_v:
	FREE(data_item);
alloc_di:
	_MutexUnLock(obj);
	return -1;
}


/*****************************************************************************
 函 数 名  : hash_SetData
 功能描述  : 
 	设置值，对于不存在的值会自动新建
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
 	data		数据缓冲区 
 	data_len	数据大小，必须大于0
  返回值：
 	成功返回设置的字节数,失败返回负数
*****************************************************************************/
int hash_SetData(HashKv_t ht,const char* k,const void* data,uint32_t data_len)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *tab_item;
	HashItem	*data_item;
	int 		ret = 0;
	
	if(!obj || data_len == 0 || !data)  return -1;
	
	if(_MutexLock(obj)) return -2;
	
	tab_item = _HashSearch(obj,k);
	data_item = _TabSearch(tab_item,k);
	if(data_item)
	{
		ret = _SetItemData(data_item, data, data_len);
		_MutexUnLock(obj);
		return ret;
	}	

	/* 不存在的项,新建 */
	data_item = MALLOC(sizeof(*data_item) + strlen(k)+1);
	if(!data_item)
		goto alloc_di;
	data_item->v = MALLOC(data_len);
	if(!data_item->v)
		goto alloc_v;
	memcpy(data_item->v,data,data_len);
	data_item->count = 0;
	data_item->v_len = data_len;
	strcpy(data_item->k,k);
	list_add(&data_item->list, &tab_item->head);
	_MutexUnLock(obj);
	
	return data_len;
	
alloc_v:
	FREE(data_item);
alloc_di:
	_MutexUnLock(obj);
	return -1;
}




/*****************************************************************************
 函 数 名  : hash_GetData
 功能描述  : 
 	获取值
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
 	buf			缓冲区
 	read_size	要读的大小
  返回值：
 	成功返回读到的字节数,失败返回负数
*****************************************************************************/
int hash_GetData(HashKv_t ht,const char* k,void* buf,uint32_t read_size)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *tab_item;
	HashItem	*data_item;
	int 		ret = 0;
	if(!obj) return -1;	
	if(_MutexLock(obj)) return -1;
	
	tab_item = _HashSearch(obj,k);
	data_item = _TabSearch(tab_item,k);
	if(data_item)
	{
		ret = _GetItemData(data_item, buf, read_size);
		goto out;
	}else{
		ret = -1;
	}
	
out:
	_MutexUnLock(obj);
	return ret;
}



int hash_Iteration(HashKv_t ht, 
	enum HashIterState (*processor)(void* param,HashBlob_t* ),void *param)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *item;
	HashItem	*data_item;
	HashItem	*work_item;
	HashBlob	Blob;
	int			i;
	enum 		HashIterState state;
	
	if(obj == NULL)	return -1;

	if(_MutexLock(obj)) return -2;

	
	item = obj->hash_tab;	
	for(i=0;i<obj->hash_tab_len ;i++)
	{
		list_for_each_entry_safe(data_item, work_item, &item[i].head , list)
		{
			Blob.data_item = data_item;			
			Blob.ht = obj;
			
			data_item->count++;
			if( &work_item->list != &item[i].head)
				work_item->count++;	/* 提前将下一个引用,防止被误删造成bug */
			
			_MutexUnLock(obj);	/* 释放锁 */
			state = processor(param,(HashBlob_t*)&Blob);
			
			if(_MutexLock(obj))
			{
				if(&work_item->list != &item[i].head)
					work_item->count--; /* 即使获取不到锁也要强行去掉引用 */
				return -2;
			}
			if(&work_item->list != &item[i].head)
				work_item->count--; /*去掉引用 */
			if(state == ITER_EXIT)
			{
				_MutexUnLock(obj);	/* 释放锁 */
				return 0;
			}
		}
	}
	_MutexUnLock(obj);	/* 释放锁 */
	return 0;
}

/*****************************************************************************
 函 数 名  : hash_GetData
 功能描述  : 
 	获取值
  参数：
 	ht 			由hash_New生成的HashKv_t
 	k			键值
 	buf			缓冲区
 	read_size	要读的大小
  返回值：
 	成功返回读到的字节数,失败返回负数
*****************************************************************************/
void hash_SetLock(HashKv_t ht,int (*lock)(void),void (*unlock)(void))
{
	HashKv		*obj = (HashKv*)ht;
	if(obj){
		obj->lock 	= lock;
		obj->unlock = unlock;
	}
}

/*****************************************************************************
 函 数 名  : hash_Del
 功能描述  : 
 	删除哈希表
 参数：
 	ht 			由hash_New生成的HashKv_t
 *****************************************************************************/
void hash_Del(HashKv_t ht)
{
	HashKv		*obj = (HashKv*)ht;
	HashTabHead *item;
	HashItem	*data_item;
	HashItem	*work_item;
	
	int			i;
	if(obj == NULL)	return ;
	item = obj->hash_tab;
	/* 先一项一项释放，最后再一起释放 */
	
	for(i=0;i<obj->hash_tab_len ;i++)
	{
		list_for_each_entry_safe(data_item, work_item, &item[i].head , list)
		{
			_DelItemData(data_item);
		}
	}
	FREE(obj);
}



/*****************************************************************************
 函 数 名  : hash_New
 功能描述  : 
 	新建一个hash表
 参数：
 	tab_len hash表的长度，关系到hash表的效率
 返回值：
 	成功返回哈希表对象，失败返回NULL
*****************************************************************************/
HashKv_t hash_New(uint32_t tab_len)
{
	int i;
	HashKv 		*obj;
	HashTabHead *item;
	
	if(tab_len == 0) return NULL;
	
	obj = MALLOC(sizeof(HashKv) + sizeof(HashTabHead)*tab_len);
	if(obj == NULL) return NULL;
	obj->hash_tab_len = tab_len;
	obj->lock = NULL;
	obj->unlock = NULL;
	item = obj->hash_tab;

	for(i=0;i<tab_len;i++)
	{
		/* 初始化每一项 */
		INIT_LIST_HEAD(&item->head);
		item++;
	}
	return (HashKv_t)obj;
}



