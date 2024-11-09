/**
 * @file memctrl.h
 * @brief 非常方便的内存操作函数
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2021-03-16
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 
 *
 */

#ifndef __MEMCTRL_H__
#define __MEMCTRL_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

#include "bits.h"


#define _MEM_BYTE_ORDER_BIG 	4321			/* 大端定义 */
#define _MEM_BYTE_ORDER_LITTLE  1234			/* 小端定义 */

/* 用户可在这里使用 _MEM_USER_DEF_ORDER 定义字节序, 一般不用定义 */
//#define _MEM_USER_DEF_ORDER 	_MEM_BYTE_ORDER_LITTLE








#ifdef _MEM_USER_DEF_ORDER
#define _MEM_BYTE_ORDER 		(_MEM_USER_DEF_ORDER)
#endif

#if defined(_MEM_BYTE_ORDER) && _MEM_BYTE_ORDER != _MEM_BYTE_ORDER_BIG && _MEM_BYTE_ORDER != _MEM_BYTE_ORDER_LITTLE
#undef _MEM_BYTE_ORDER
#endif

#ifndef _MEM_BYTE_ORDER
	#if defined(__BYTE_ORDER__) && defined(__ORDER_BIG_ENDIAN__)
		#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
			/* 大端 */
			#define _MEM_BYTE_ORDER  _MEM_BYTE_ORDER_BIG
		#else
			#define _MEM_BYTE_ORDER  _MEM_BYTE_ORDER_LITTLE
			/* 小段 */
		#endif /* __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__ */
	#else
		/* 若没有定义__BYTE_ORDER__ 一般在windows系统上编译 那么默认系统类型为小端 */
		#define _MEM_BYTE_ORDER  _MEM_BYTE_ORDER_LITTLE
	#endif
#endif




/* 内部：字节序转换 */
static inline void _byte_order_change(void* ptr, uint32_t len){
	uint32_t i; char *p = (char*)(ptr);
	for(i=0;i<(len)/2;i++){
		p[(len)-i-1] ^= p[i];
		p[i] ^=p[(len)-i-1];
		p[(len)-i-1] ^= p[i];
	}
}

/**
 * @brief 指针以单字节自增
 * @param ptr 			指针
 * @param len 			自增数
 */
#define MEM_INC(ptr, len) ((typeof(ptr))((char *)(ptr) + (len)))

/** 
 * @brief 获取内存中某个类型的值
 * @param pdst 			某段内存
 * @param type 			类型
 */
#define GET_MEM_VAL(psrc, type)  (*((type *)(psrc)))

/**
 * @brief 内存指针字节序转换
 *		BYTE_ORDER_CHANGE: 					执行字节序转换
 *		BYTE_ORDER_LITTLE_TO_SYSTEM:		将内存中的 小端字节序 转换为 系统字节序
 *		BYTE_ORDER_BIG_TO_SYSTEM:			将内存中的 大端字节序 转换为 系统字节序
 *		BYTE_ORDER_LITTLE_TO_SYSTEM:		将内存中的 系统字节序 转换为 小端字节序
 *		BYTE_ORDER_BIG_TO_SYSTEM:			将内存中的 系统字节序 转换为 大端字节序
 * @param ptr 			指针
 * @param len 			内容大小
 */
#define MEM_BYTE_ORDER_CHANGE(ptr,len) _byte_order_change((void*)(ptr), (len))

#if _MEM_BYTE_ORDER == _MEM_BYTE_ORDER_LITTLE
	#define MEM_BYTE_ORDER_LITTLE_TO_SYSTEM(ptr, len) 
	#define MEM_BYTE_ORDER_BIG_TO_SYSTEM(ptr, len) 		MEM_BYTE_ORDER_CHANGE(ptr,len)
	#define MEM_BYTE_ORDER_SYSTEM_TO_LITTLE(ptr, len) 
	#define MEM_BYTE_ORDER_SYSTEM_TO_BIG(ptr, len) 		MEM_BYTE_ORDER_CHANGE(ptr,len)
#else
	#define MEM_BYTE_ORDER_LITTLE_TO_SYSTEM(ptr, len) 	MEM_BYTE_ORDER_CHANGE(ptr,len)
	#define MEM_BYTE_ORDER_BIG_TO_SYSTEM(ptr, len) 		
	#define MEM_BYTE_ORDER_SYSTEM_TO_LITTLE(ptr, len) 	MEM_BYTE_ORDER_CHANGE(ptr,len)
	#define MEM_BYTE_ORDER_SYSTEM_TO_BIG(ptr, len) 		
#endif

/**
 * @brief 	变量的字节序转换
 *		VAR_BYTE_ORDER_CHANGE: 					执行变量字节序转换
 *		VAR_BYTE_ORDER_LITTLE_TO_SYSTEM:		将变量的 小端字节序 转换为 系统字节序
 *		VAR_BYTE_ORDER_BIG_TO_SYSTEM:			将变量的 大端字节序 转换为 系统字节序
 *		VAR_BYTE_ORDER_SYSTEM_TO_LITTLE:		将变量的 系统字节序 转换为 小端字节序
 *		VAR_BYTE_ORDER_SYSTEM_TO_BIG:			将变量的 系统字节序 转换为 大端字节序
 * @param val 			必须是变量
 */
#define VAR_BYTE_ORDER_CHANGE(val)  			MEM_BYTE_ORDER_CHANGE(&(val),sizeof(val))
#define VAR_BYTE_ORDER_LITTLE_TO_SYSTEM(val)  	MEM_BYTE_ORDER_LITTLE_TO_SYSTEM(&(val),sizeof(val))
#define VAR_BYTE_ORDER_BIG_TO_SYSTEM(val)  		MEM_BYTE_ORDER_BIG_TO_SYSTEM(&(val),sizeof(val))
#define VAR_BYTE_ORDER_SYSTEM_TO_LITTLE(val)  	MEM_BYTE_ORDER_LITTLE_TO_SYSTEM(&(val),sizeof(val))
#define VAR_BYTE_ORDER_SYSTEM_TO_BIG(val)  		MEM_BYTE_ORDER_BIG_TO_SYSTEM(&(val),sizeof(val))


/** 
 * @brief 用一个值，来设置一段内存
 *		SET_MEM_VAL_TYPE:						将type类型的变量设置到指定内存中，忽略字节序大小端
 *		SET_MEM_VAL_TYPE_SYSTEM_TO_LITTLE:		将type类型的系统字节序变量设置到指定内存，且以小端存储
 *		SET_MEM_VAL_TYPE_SYSTEM_TO_BIG:			将type类型的系统字节序变量设置到指定内存，且以大端存储
 *		SET_MEM_VAL_TYPE_LITTLE_TO_SYSTEM:		将type类型的小端字节序变量设置到指定内存，且以系统字节序存储
 *		SET_MEM_VAL_TYPE_BIG_TO_SYSTEM:			将type类型的大端字节序变量设置到指定内存，且以系统字节序存储
 * @param pdst 			目的地址
 * @param val 			值
 * @param type 			要设置值的类型
 */
#define SET_MEM_VAL_TYPE(pdst, val, type) \
		do{\
			*((type *)(pdst)) = (type)(val);\
		}while(0)

#define SET_MEM_VAL_TYPE_SYSTEM_TO_LITTLE(pdst, val, type) 	\
		do{\
			SET_MEM_VAL_TYPE(pdst, val, type);\
			MEM_BYTE_ORDER_SYSTEM_TO_LITTLE(pdst, sizeof(type));\
		}while(0)

#define SET_MEM_VAL_TYPE_SYSTEM_TO_BIG(pdst, val, type) 	\
		do{\
			SET_MEM_VAL_TYPE(pdst, val, type);\
			MEM_BYTE_ORDER_SYSTEM_TO_BIG(pdst, sizeof(type));\
		}while(0)


#define SET_MEM_VAL_TYPE_LITTLE_TO_SYSTEM(pdst, val, type) 	\
		do{\
			SET_MEM_VAL_TYPE(pdst, val, type);\
			MEM_BYTE_ORDER_LITTLE_TO_SYSTEM(pdst, sizeof(type));\
		}while(0)

#define SET_MEM_VAL_TYPE_BIG_TO_SYSTEM(pdst, val, type) 	\
		do{\
			SET_MEM_VAL_TYPE(pdst, val, type);\
			MEM_BYTE_ORDER_BIG_TO_SYSTEM(pdst, sizeof(type));\
		}while(0)


/** 
 * @brief 用段内存，来设置一段内存
 *		SET_MEM_MEM_TYPE:						将type类型的指针设置到指定内存中，忽略字节序大小端
 *		SET_MEM_MEM_TYPE_SYSTEM_TO_LITTLE:		将type类型的系统字节序指针设置到指定内存，且以小端存储
 *		SET_MEM_MEM_TYPE_SYSTEM_TO_BIG:			将type类型的系统字节序指针设置到指定内存，且以大端存储
 *		SET_MEM_MEM_TYPE_LITTLE_TO_SYSTEM:		将type类型的小端字节序指针设置到指定内存，且以系统字节序存储
 *		SET_MEM_MEM_TYPE_BIG_TO_SYSTEM:			将type类型的大端字节序指针设置到指定内存，且以系统字节序存储
 * @param pdst 			目的地址
 * @param psrc 			值
 * @param type 			要设置值的类型
 */

#define SET_MEM_MEM_TYPE(pdst, psrc, type)	\
	SET_MEM_VAL_TYPE(pdst, GET_MEM_VAL(psrc,type), type)

#define SET_MEM_MEM_TYPE_SYSTEM_TO_LITTLE(pdst, psrc, type)	\
	SET_MEM_VAL_TYPE_SYSTEM_TO_LITTLE(pdst, GET_MEM_VAL(psrc, type), type)

#define SET_MEM_MEM_TYPE_SYSTEM_TO_BIG(pdst, psrc, type)	\
	SET_MEM_VAL_TYPE_SYSTEM_TO_BIG(pdst, GET_MEM_VAL(psrc, type), type)

#define SET_MEM_MEM_TYPE_LITTLE_TO_SYSTEM(pdst, psrc, type)	\
	SET_MEM_VAL_TYPE_LITTLE_TO_SYSTEM(pdst, GET_MEM_VAL(psrc ,type), type)

#define SET_MEM_MEM_TYPE_BIG_TO_SYSTEM(pdst, psrc, type)	\
	SET_MEM_VAL_TYPE_BIG_TO_SYSTEM(pdst, GET_MEM_VAL(psrc, type), type)



/*###################################### 向下兼容 ######################################*/
/* 此函数只有gcc编译器可用 */
#define SET_MEM_VAL(pdst,val) 			SET_MEM_VAL_TYPE(pdst, val, typeof(val))
#define BYTE_ORDER_CHANGE(ptr,len)		MEM_BYTE_ORDER_CHANGE(ptr,len)
/*######################################################################################*/

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __MEMCTRL_H__ */
