/**
 * @file regwr_cb.c
 * @brief 使用寄存器读写的形式实现的环形缓冲区，使MCU和MPU数据流畅通无阻
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-09-02
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */


#include <stdint.h>
#include <string.h>


#include "regwr_cb.h"

/* 
 * 实现分别对应 mcu模块common_ringbuffer的7个函数
 * extern uint32_t crb_Read(Crb* fifo, uint8_t *buf, uint32_t buf_size);
 * extern uint32_t crb_Peep(Crb* fifo, uint8_t *buf, uint32_t buf_size);
 * extern uint32_t crb_Write(Crb* fifo, const uint8_t *buf, uint32_t buf_size);
 * extern uint32_t crb_ReadAir(Crb* fifo, uint32_t buf_size);
 * extern void crb_Clear(Crb* fifo);
 * extern uint32_t crb_Size(Crb *fifo);
 * extern uint32_t crb_FreeSize(Crb* fifo);
 */



/**
 * @brief                   获取缓冲区已经使用的大小
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @return int 
 */
int RegWrCb_Size(RegWrCbHandle *h, uint16_t cb_addr, uint32_t timeout){
    int ret;
    int size;
    ret = h->read_reg(cb_addr+CBREG_CMD_GET_SIZE, (uint8_t*)&size, 4, timeout);
    if(ret < 0) return ret;
    return size;
}

/**
 * @brief                   获取缓冲区余量
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @return int 
 */
int RegWrCb_FreeSize(RegWrCbHandle *h, uint16_t cb_addr, uint32_t timeout){
    int ret;
    int size;
    ret = h->read_reg(cb_addr+CBREG_CMD_GET_FREESIZE, (uint8_t*)&size, 4, timeout);
    if(ret < 0) return ret;
    return size;
}

/**
 * @brief                   读环形缓冲区
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @param  buf              数据存储的缓冲区
 * @param  buf_size         
 * @return int 
 */
int RegWrCb_Read(RegWrCbHandle *h, uint16_t cb_addr, uint8_t *buf, uint32_t buf_size, uint32_t timeout){
    int ret;
    uint16_t r_len = (uint16_t)buf_size;
    ret = RegWrCb_Size(h, cb_addr, timeout);
    if(ret < 0) return ret;
    if(ret == 0) return 0;
    r_len = r_len > ret ? (uint16_t)ret : r_len;
    ret = h->read_reg(cb_addr+CBREG_CMD_READ, buf, r_len, timeout);
    if(ret < 0) return ret;
    return r_len;
}

/**
 * @brief                   按最小读写粒度来读环形缓冲区
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @param  data             要读到的缓冲区
 * @param  gran_size        数据的读写粒度
 * @param  nmemb            在该粒度下读写数据的数量
 * @param  timeout          超时时间
 * @return int              成功返回读到数据的数量，失败返回负数
 */
int RegWrCb_GranRead(RegWrCbHandle *h, uint16_t cb_addr, uint8_t *data, uint32_t gran_size, uint32_t nmemb, uint32_t timeout){
    int ret;
    uint16_t r_num = (uint16_t)nmemb;
    uint16_t r_len;
    ret = RegWrCb_Size(h, cb_addr, timeout);
    if(ret < 0) return ret;
    r_num = ((uint16_t)((uint16_t)ret/gran_size)) > r_num ? r_num : (uint16_t)((uint16_t)ret/gran_size);
    if(r_num == 0) return 0;
    r_len = (uint16_t)(r_num*gran_size);
    ret = h->read_reg(cb_addr+CBREG_CMD_READ, data, r_len, timeout);
    if(ret < 0) return ret;
    return r_num;
}


/**
 * @brief                   写环形缓冲区
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @param  data             要写的数据
 * @param  data_size        要写的数据长度
 * @return int              成功返回成功的数量
 */
int RegWrCb_Write(RegWrCbHandle *h, uint16_t cb_addr, const uint8_t *data, uint32_t data_size, uint32_t timeout){
    int ret;
    uint16_t w_len = (uint16_t)data_size;
    ret = RegWrCb_FreeSize(h, cb_addr, timeout);
    if(ret < 0) return ret;
    if(ret == 0) return 0;
    w_len = w_len > ret ? (uint16_t)ret : w_len;
    ret = h->write_reg(cb_addr+CBREG_CMD_WRITE, data, w_len, timeout);
    if(ret < 0) return ret;
    return w_len;
}

/**
 * @brief                   按最小读写粒度来写环形缓冲区
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @param  data             要写的数据
 * @param  gran_size        数据的读写粒度
 * @param  nmemb            在该粒度下读写数据的数量
 * @param  timeout          超时时间
 * @return int              成功返回写数据的数量，失败返回负数
 */
int RegWrCb_GranWrite(RegWrCbHandle *h, uint16_t cb_addr, const uint8_t *data, uint32_t gran_size, uint32_t nmemb, uint32_t timeout){
    int ret;
    uint16_t w_num = (uint16_t)nmemb;
    uint16_t w_len;
    ret = RegWrCb_FreeSize(h, cb_addr, timeout);
    if(ret < 0) return ret;
    w_num = ((uint16_t)((uint16_t)ret/gran_size)) > w_num ? w_num : (uint16_t)((uint16_t)ret/gran_size);
    if(w_num == 0) return 0;
    w_len = (uint16_t)(w_num*gran_size);
    ret = h->write_reg(cb_addr+CBREG_CMD_WRITE, data, w_len, timeout);
    if(ret < 0) return ret;
    return w_num;
}

/**
 * @brief                   清空环形缓冲区
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @return int              成功0
 */
int RegWrCb_Clean(RegWrCbHandle *h, uint16_t cb_addr, uint32_t timeout){
    int ret;
    uint8_t ch = 0x00;
    ret = h->write_reg(cb_addr+CBREG_CMD_CLEAN, &ch, 1, timeout);
    if(ret < 0) return ret;
    return 0;
}

/**
 * @brief                   读空气
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @param  read_size        偏移的数量，不读出数据，但是进行偏移
 * @return int              返回读成功的数量
 */
int RegWrCb_ReadAir(RegWrCbHandle *h, uint16_t cb_addr, uint32_t read_size, uint32_t timeout){
    int ret;
    uint32_t r_len = (uint16_t)read_size;
    ret = RegWrCb_Size(h, cb_addr, timeout);
    if(ret < 0) return ret;
    if(ret == 0) return 0;
    r_len = r_len > (uint32_t)ret ? (uint32_t)ret : r_len;
    ret = h->write_reg(cb_addr+CBREG_CMD_READAIR, (uint8_t *)&r_len, 4, timeout);
    if(ret < 0) return ret;
    return (int)r_len;
}



/**
 * @brief                   偷看数据,建议使用这种方式再去读空气，因为读写寄存器可能会出错导致环形缓冲区丢数据
 * @param  h                句柄
 * @param  cb_addr          环形缓冲区 寄存器起始地址
 * @param  buf              存储偷看数据的缓冲区
 * @param  buf_size         缓冲区大小
 * @return int              返回读成功的数量
 */
int RegWrCb_Peep(RegWrCbHandle *h, uint16_t cb_addr, uint8_t *buf, uint32_t buf_size, uint32_t timeout){
    int ret;
    uint16_t r_len = (uint16_t)buf_size;
    ret = RegWrCb_Size(h, cb_addr, timeout);
    if(ret < 0) return ret;
    if(ret == 0) return 0;
    r_len = r_len > ret ? (uint16_t)ret : r_len;
    ret = h->read_reg(cb_addr+CBREG_CMD_PEEP, buf, r_len, timeout);
    if(ret < 0) return ret;
    return r_len;
}

