/**
 * @file factory-data.c
 * @brief 通用的MCU出厂信息获取，
    通过脚本，在bin文件尾部增加,然后本程序遍历全部的固件，来找到该信息。
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */


#include <stddef.h>
#include "factory-data.h"

#define POLY 0xEDB88320U

uint32_t factory_data_crc32(uint32_t crc, const uint8_t *msg, uint32_t len){
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= *msg++;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}

const FactoryData* factory_data_detect(const void *part_start_addr, uint32_t part_size){
    const uint8_t *pos_addr = part_start_addr;
    const uint8_t *end_addr = pos_addr + part_size;
    const FactoryData* pos_factory_data;
    uint32_t ffffffff = 0xffffffff;
    uint32_t crc32_val;
    /* 
     * pos_addr 向上8字节对齐  
     * end_addr 向下8字节对齐
     */
    pos_addr = (const uint8_t*)(((uintptr_t)pos_addr+7U) & ~7U);
    end_addr = (const uint8_t*)((uintptr_t)end_addr & ~7U);

    /* FactoryData 数据必然在8字节对齐的地址处 */
    for(; end_addr - pos_addr >= (int)sizeof(FactoryData); pos_addr+=8 ){
        pos_factory_data = (const FactoryData*)pos_addr;
        if( pos_factory_data->Magic_A != FC_MAGIC_A || 
            pos_factory_data->Magic_B != FC_MAGIC_B ||
            pos_factory_data->factory_len < offsetof(FactoryData, software_version) ||
            pos_factory_data->factory_len > (uint32_t)(end_addr - pos_addr)
        )
            continue;
        crc32_val = ~(0xFFFFFFFF);
        crc32_val = factory_data_crc32(crc32_val, pos_addr-pos_factory_data->software_len, 
            pos_factory_data->software_len + offsetof(FactoryData, crc32));
        crc32_val = factory_data_crc32(crc32_val, (uint8_t*)&ffffffff, sizeof(uint32_t));    /* 中间的crc32值使用0xffffffff代替 */
        crc32_val = factory_data_crc32(crc32_val, (uint8_t*)(&pos_factory_data->software_version), 
            pos_factory_data->factory_len - offsetof(FactoryData, software_version));
        
        if(crc32_val != pos_factory_data->crc32)
            continue;
        return pos_factory_data;
    }
    return NULL;
}