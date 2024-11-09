/**
 * @file logic_reg.c
 * @brief 关于本MCU寄存器读写的全部实现
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-08-16
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>


#include "common_ringbuffer.h"
#include "memctrl.h"
#include "logic-reg.h"

typedef struct _LogiRegRun{
    RegGroup *reg_group[LOGICREG_MAX_GROUP_CNT];
}LogiRegRun;

static LogiRegRun logiRegRun;


/* 
 * 检查操作的寄存器是不是在合理的范围内，若在返回寄存器组所在句柄索引
 */
static int _RegGroupValid(uint16_t addr, uint16_t len){
    uint16_t end = addr+len;
    for(int i=0;i<LOGICREG_MAX_GROUP_CNT;i++){
        if(logiRegRun.reg_group[i])
        {
            if(logiRegRun.reg_group[i]->group_type == REG_GROUP_TYPE_CB  ){ 
                if( addr >= logiRegRun.reg_group[i]->group_start &&
                    addr < logiRegRun.reg_group[i]->group_end)
                    return i;
            }else{
                if( addr >= logiRegRun.reg_group[i]->group_start  && 
                    end <= logiRegRun.reg_group[i]->group_end){
                    return i;
                }
            }
        }
    }
    return -1;
}

static int _NormalRead(RegGroup *reg_g, uint16_t addr, uint16_t size, uint8_t *reg_data){
    addr -= reg_g->group_start;
    memcpy(reg_data, reg_g->mem+addr, size);
    return size;
}

static int _NormalWrite(RegGroup *reg_g, uint16_t addr, uint16_t size, const uint8_t *reg_data){
    addr -= reg_g->group_start;
    memcpy(reg_g->mem+addr, reg_data, size);
    return size;
}


static int _CbRead(RegGroup *reg_g, uint16_t addr, uint16_t size, uint8_t *reg_data){
    Crb* crb = (Crb*)reg_g->mem;
    addr -= reg_g->group_start;
    switch (addr) {
        case CBREG_CMD_GET_SIZE:
            if(size != 4) return 0;
            SET_MEM_VAL_TYPE(reg_data, crb_Size(crb), uint32_t);
            return size;
        case CBREG_CMD_GET_FREESIZE:
            if(size != 4) return 0;
            SET_MEM_VAL_TYPE(reg_data, crb_FreeSize(crb), uint32_t);
            return size;
        case CBREG_CMD_READ:
            if(size > crb_Size(crb)) return 0;
            return (int)crb_Read(crb, reg_data, size);
        case CBREG_CMD_PEEP:
            if(size > crb_Size(crb)) return 0;
            return (int)crb_Peep(crb, reg_data, size);
    }
    return 0;
}

static int _CbWrite(RegGroup *reg_g, uint16_t addr, uint16_t size, const uint8_t *reg_data){
    Crb* crb = (Crb*)reg_g->mem;
    addr -= reg_g->group_start;
    switch (addr) {
        case CBREG_CMD_WRITE:
            if(size > crb_FreeSize(crb)) return 0;
            return (int)crb_Write(crb, reg_data, size);
        case CBREG_CMD_CLEAN:
            if(size != 1) return 0;
            crb_Clear(crb);
            return size;
        case CBREG_CMD_READAIR:
            if(size != 4) return 0;
            if(GET_MEM_VAL(reg_data, uint32_t) > crb_Size(crb)) 
                return 0;
            crb_ReadAir(crb, GET_MEM_VAL(reg_data, uint32_t));
            return size;
    }
    return 0;
}

/**
 * @brief 读寄存器
 * @param  addr             寄存器地址
 * @param  size             寄存器大小
 * @param  reg_data         寄存器值
 * @return int              返回读到的字节数
 */
int LogicReg_Read(uint16_t addr, uint16_t size, uint8_t *reg_data){
    int ret;
    RegGroup *reg_g;
    if(reg_data == NULL) return 0;

    ret = _RegGroupValid(addr, size);
    if(ret < 0) return 0;
    reg_g = logiRegRun.reg_group[ret];
    if(reg_g->group_type != REG_GROUP_TYPE_CB){
        return _NormalRead(reg_g, addr, size, reg_data);
    }
    return _CbRead(reg_g, addr, size, reg_data);
}

/**
 * @brief  写寄存器
 * @param  addr             寄存器地址
 * @param  size             寄存器大小
 * @param  reg_data         寄存器值
 * @return int              返回写的字节数
 */
int LogicReg_Write(uint16_t addr, uint16_t size, const uint8_t *reg_data){
    int ret;
    RegGroup *reg_g;
    if(reg_data == NULL) return 0;
    
    ret = _RegGroupValid(addr, size);
    if(ret < 0) return 0;
    reg_g = logiRegRun.reg_group[ret];
    if(reg_g->group_type == REG_GROUP_TYPE_WR){
        return _NormalWrite(reg_g, addr, size, reg_data);
    }else if(reg_g->group_type == REG_GROUP_TYPE_CB){
        return _CbWrite(reg_g, addr, size, reg_data);
    }
    return 0;
}

/**
 * @brief 注册寄存器组
 * @param  static_reg_group     寄存器组描述结构
 * @return int                  成功0 失败-1
 */
int LogicReg_RegisterGroup(RegGroup *static_reg_group){
    if( static_reg_group == NULL      ||
        static_reg_group->mem == NULL
    )   return -1;
    if(static_reg_group->group_type == REG_GROUP_TYPE_CB)
        static_reg_group->group_end = static_reg_group->group_start + CBREG_SIZE;
    if( _RegGroupValid(static_reg_group->group_start, 
        static_reg_group->group_end - static_reg_group->group_start) >= 0 )
        return -1;
    
    for(int i=0;i<LOGICREG_MAX_GROUP_CNT;i++){
        if(logiRegRun.reg_group[i] == NULL){
            logiRegRun.reg_group[i] = static_reg_group;
            return 0;
        }
    }
    return -1;
}


/**
 * @brief 注销寄存器组
 * @param  static_reg_group     寄存器组描述结构
 * @return int                  成功0 失败-1
 */
void LogicReg_UnregisterGroup(RegGroup *static_reg_group){
    
    for(int i=0;i<LOGICREG_MAX_GROUP_CNT;i++){
        if(logiRegRun.reg_group[i] == static_reg_group){
            logiRegRun.reg_group[i] = NULL;
            return ;
        }
    }
    return ;
}


int LogicReg_Init(void){
    memset(&logiRegRun, 0, sizeof(logiRegRun));
    return 0;
}