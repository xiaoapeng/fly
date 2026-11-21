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
#include <factory-data.h>

#define POLY 0xEDB88320U
#define FACTORY_DATA_ALIGNMENT 8U

uint32_t factory_data_crc32(uint32_t crc, const uint8_t *msg, uint32_t len)
{
    uintptr_t addr = (uintptr_t)msg;
    int k;

    crc = ~crc;
    while (len--) {
        crc ^= *(const uint8_t *)addr;
        ++addr;
        for (k = 0; k < 8; k++)
            crc = crc & 1 ? (crc >> 1) ^ POLY : crc >> 1;
    }
    return ~crc;
}

const FactoryData *factory_data_detect(const void *part_start_addr, uint32_t part_size)
{
    const uintptr_t start = (uintptr_t)part_start_addr;
    const uintptr_t alignment_mask = FACTORY_DATA_ALIGNMENT - 1U;
    const uint32_t placeholder_crc = UINT32_MAX;
    const uint32_t version_offset = offsetof(FactoryData, software_version);

    if (part_size < sizeof(FactoryData) || part_size - 1U > UINTPTR_MAX - start) {
        return NULL;
    }

    const uint32_t first_offset = (uint32_t)(((start + alignment_mask) & ~alignment_mask) - start);
    for (uint32_t offset = first_offset; offset <= part_size - sizeof(FactoryData);
         offset += FACTORY_DATA_ALIGNMENT) {
        const FactoryData *data = (const FactoryData *)(start + offset);
        if (data->magic_a != FC_MAGIC_A || data->magic_b != FC_MAGIC_B ||
            data->software_len != offset || data->factory_len < sizeof(FactoryData) ||
            data->factory_len > part_size - offset) {
            continue;
        }

        uint32_t crc = factory_data_crc32(0, part_start_addr, offset + offsetof(FactoryData, crc32));
        crc = factory_data_crc32(crc, (const uint8_t *)&placeholder_crc, sizeof(placeholder_crc));
        crc = factory_data_crc32(crc, (const uint8_t *)&data->software_version,
                                 data->factory_len - version_offset);
        if (crc == data->crc32) {
            return data;
        }
    }
    return NULL;
}
