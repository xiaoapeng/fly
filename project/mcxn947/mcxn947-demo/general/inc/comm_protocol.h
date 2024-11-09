/**
 * @file comm_protocol.h
 * @brief 
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2023-03-23
 * 
 * @copyright Copyright (c) 2023  simon.xiaoapeng@gmail.com
 * 

 */

#ifndef  _COMM_PROTOCOL_H_
#define  _COMM_PROTOCOL_H_

    extern int a5_5a_data_recovery(const uint8_t* trans_data, int trans_data_size, uint8_t* proc_data);
    extern int a5_5a_data_escaping(const uint8_t* proc_data, int proc_data_size, uint8_t* trans_data);
#endif
