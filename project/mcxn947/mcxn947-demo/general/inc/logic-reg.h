/**
 * @file logic_reg.h
 * @brief 逻辑寄存器的实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */
 #ifndef _LOGIC_REG_H_
 #define _LOGIC_REG_H_
 
 
 #ifdef __cplusplus
 #if __cplusplus
 extern "C"{
 #endif
 #endif /* __cplusplus */

#include "common_ringbuffer.h"

#define LOGICREG_MAX_GROUP_CNT      10           /* 最多支持5组寄存器 */


#define CBREG_CMD_GET_SIZE         0x00         /* 读4个字节 获取已用容量 */
#define CBREG_CMD_GET_FREESIZE     0x01         /* 读4个字节 获取可用容量 */
#define CBREG_CMD_READ             0x02         /* 读N个字节 调用read函数 读前先获取容量 超过将失败 */
#define CBREG_CMD_WRITE            0x03         /* 写N个字节 调用write函数 写前先获取容量 超过将失败 */
#define CBREG_CMD_CLEAN            0x04         /* 随便写1个字节 调用clean函数 */
#define CBREG_CMD_READAIR          0x05         /* 读空气 写4个字节 写要读的空气数量 调用 crb_ReadAir */
#define CBREG_CMD_PEEP             0x06         /* 偷看N个字节 调用read函数 读前先获取容量 超过将失败 */
#define CBREG_SIZE                 0x07         /* CBREG 寄存器的长度 */

typedef enum _RegGroupType{
    REG_GROUP_TYPE_WR,          /* 可读可写寄存器 */
    REG_GROUP_TYPE_RO,          /* 只读寄存器 */
    REG_GROUP_TYPE_CB,          /* 寄存器方式实现的环形缓冲区 */
}RegGroupType;


typedef struct _RegGroupDefine{
    RegGroupType        group_type;          /* 寄存器类型 */
    uint16_t            group_start;         /* 寄存器开始地址 */
    uint16_t            group_end;           /* 寄存器结束地址 = 开始地址 + 寄存器长度 */
    uint8_t             *mem;                /* 寄存器对应的内存 */
}RegGroup;

extern int LogicReg_Read(uint16_t addr, uint16_t size, uint8_t *reg_data);
extern int LogicReg_Write(uint16_t addr, uint16_t size, const uint8_t *reg_data);
extern int LogicReg_RegisterGroup(RegGroup *static_reg_group);
extern void LogicReg_UnregisterGroup(RegGroup *static_reg_group);
extern int LogicReg_Init(void); 

 
 #ifdef __cplusplus
 #if __cplusplus
 }
 #endif
 #endif /* __cplusplus */
 
 
 #endif // _LOGIC_REG_H_