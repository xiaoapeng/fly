/**
 * @file comm_protocol.c
 * @brief 0xa5 .............0x5a 数据转义与还原
 *
 * 转义:
 * 0xA5 --> 0xAA 0x01
 * 0xAA --> 0xAA 0x02
 * 0x5A --> 0x55 0x01
 * 0x55 --> 0x55 0x02
 *
 * 还原:
 * 0xAA 0x01 --> 0xA5
 * 0xAA 0x02 --> 0xAA
 * 0x55 0x01 --> 0x5A
 * 0x55 0x02 --> 0x55
 *
 * @author 未知 simon.xiaoapeng 进行部分修改
 *
 */

#include "stdint.h"
#include "comm_protocol.h"

/**
 * @brief  数据转义成传输帧  01 02 03 04 05  --> A5 01 02 03 04 05 5A
 * @param  proc_data        待转义
 * @param  proc_data_size   待转义数据大小
 * @param  trans_data       转义后的数据存放缓冲区，该缓冲区应该是 proc_data_size的两倍
 * @return int 
 */
int a5_5a_data_escaping(const uint8_t* proc_data, int proc_data_size, uint8_t* trans_data){
    /***
     * 0xA5 --> 0xAA 0x01
     * 0xAA --> 0xAA 0x02
     * 0x5A --> 0x55 0x01
     * 0x55 --> 0x55 0x02
     */
    int trans_data_i=0;
    int i;
    trans_data[trans_data_i++]=0xA5;
    for(i=0;i<proc_data_size;i++){
        if(proc_data[i]==0xA5){
            trans_data[trans_data_i++] = 0xAA;
            trans_data[trans_data_i++] = 0x01;
        }else if(proc_data[i]==0xAA){
            trans_data[trans_data_i++] = 0xAA;
            trans_data[trans_data_i++] = 0x02;
        }else if(proc_data[i]==0x5A){
            trans_data[trans_data_i++] = 0x55;
            trans_data[trans_data_i++] = 0x01;
        }else if(proc_data[i]==0x55){
            trans_data[trans_data_i++] = 0x55;
            trans_data[trans_data_i++] = 0x02;
        }else{
            trans_data[trans_data_i++] = proc_data[i];
        }
    }
    trans_data[trans_data_i++] = 0x5A;
    return trans_data_i;
}

/**
 * @brief  传输帧转义成数据  A5 01 02 03 04 05 AA 01 5A --> 01 02 03 04 05 A5
 * @param  trans_data       传输帧，待还原数据
 * @param  trans_data_size  传输帧大小
 * @param  proc_data        还原后的数据存放缓冲区，该缓冲区应该不小于trans_data_size的大小
 * @return int 
 */
int a5_5a_data_recovery(const uint8_t* trans_data, int trans_data_size, uint8_t* proc_data){
	uint16_t i;
    /***
     * 0xAA 0x01 --> 0xA5
     * 0xAA 0x02 --> 0xAA
     * 0x55 0x01 --> 0x5A
     * 0x55 0x02 --> 0x55
     */
    int proc_data_size_temp=0;
    if(trans_data[0] != 0xA5 || trans_data[trans_data_size-1] != 0x5A)
        return -1;
    for(i=1;i<trans_data_size-1;i++){
        if(trans_data[i] == 0xAA){
            if(trans_data[i+1] == 0x01){
                proc_data[proc_data_size_temp++] = 0xA5;
            }else if(trans_data[i+1] == 0x02){
                proc_data[proc_data_size_temp++] = 0xAA;
            }else
                return -1;
            i++;
        }else if(trans_data[i] == 0x55){
            if(trans_data[i+1] == 0x01){
                proc_data[proc_data_size_temp++] = 0x5A;
            }else if(trans_data[i+1] == 0x02){
                proc_data[proc_data_size_temp++] = 0x55;
            }
            else
                return -1;
            i++;
        }else{
            proc_data[proc_data_size_temp++] = trans_data[i];
        }
    }
    return proc_data_size_temp;
}
