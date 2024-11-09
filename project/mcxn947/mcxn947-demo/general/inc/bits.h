/**
 * @file bits.h
 * @brief 内存bit位操作
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2020-12-26
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */

#ifndef __BITS_H__
#define __BITS_H__

#include <stdint.h>

/* 32位计算机 */

#define WB(x) UINT32_MAX >> (32-x)

#define W_1		WB(1)
#define W_2		WB(2)
#define W_3		WB(3)
#define W_4		WB(4)
#define W_5		WB(5)
#define W_6		WB(6)
#define W_7		WB(7)
#define W_8		WB(8)
#define W_9		WB(9)
#define W_10	WB(10)
#define W_12	WB(12)
#define W_13	WB(13)
#define W_14	WB(14)
#define W_15	WB(15)
#define W_16	WB(16)
#define W_17	WB(17)
#define W_18	WB(18)
#define W_19	WB(19)
#define W_20	WB(20)
#define W_21	WB(21)
#define W_22	WB(22)
#define W_23	WB(23)
#define W_24	WB(24)
#define W_25	WB(25)
#define W_26	WB(26)
#define W_27	WB(27)
#define W_28	WB(28)
#define W_29	WB(29)
#define W_30	WB(30)
#define W_31	WB(31)


/* 将第A个字节的第d位转换为线性的 */
#define To_Bitn(_A_,_D_)							(((_A_)*8)+(_D_))

/* 获取一个值中某几个位的值 */
#define Get_ValBitw(_VAR, _W, _BIT) 		(((_VAR)>>(_BIT))&(_W))
#define Set_ValBitw(_VAR, _W, _BIT, VAL_)	(_VAR) = (((_VAR)&(~((_W) << (_BIT))))|((((typeof(_VAR))(VAL_))&(_W))<< (_BIT)))

/* 获取某个地址第n个比特位的状态 */
#define Get_Bit(Start, bit_x)	((((uint8_t*)(Start))[(bit_x)/8] >> ((bit_x) % 8)) & 1)
#define Set_Bit(Start, bit_x, val) ((uint8_t*)(Start))[(bit_x)/8] =  (uint8_t)((((uint8_t*)(Start))[(bit_x)/8] & (~(1 << ((bit_x) % 8)))) | (((val) ? 1 : 0) << ((bit_x) % 8)))

#endif /* __BITS_H__ */
