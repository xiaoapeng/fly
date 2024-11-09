/**
 * @file factory_data.h
 * @brief 通用的MCU出厂信息获取，
    通过脚本，在bin文件尾部增加,然后本程序遍历全部的固件，来找到该信息。
 * @author simon.xiaoapeng (simon.xiaoapeng@gmail.com)
 * @date 2024-06-04
 * 
 * @copyright Copyright (c) 2024  simon.xiaoapeng@gmail.com
 * 
 */
#ifndef _FACTORY_DATA_H_
#define _FACTORY_DATA_H_

#include <stdint.h>

#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */
#define FC_PART_NAME_MAX_SIZE           12
#define FC_FIRMWARE_NAME_MAX_SIZE       32
#define FC_DESCRIPTION_LEN              64
#define FC_MAGIC_A                      0xAA5555AA
#define FC_MAGIC_B                      0x88777788
typedef struct  __attribute__((aligned(1))) {
    uint32_t Magic_A;
    uint32_t Magic_B;
    uint32_t software_len;                                              /* 分区开始位置到本数据结构的长度 */
    uint32_t factory_len;                                               /* 本数据结构的长度，应该大于等于 sizeof(FactoryData) */
    uint32_t crc32;                                                     /* 整个包的CRC32大小，在做CRC32时本字段为0xFFFFFFFF */
    union{
        struct {
            uint8_t major;
            uint8_t minor;
            uint8_t patch;
            uint8_t reserved;
        };
        uint32_t software_version;
    };
    char        part_name[FC_PART_NAME_MAX_SIZE];                       /* 所在分区名称 */
    char        firmware_name[FC_FIRMWARE_NAME_MAX_SIZE];               /* 固件的名称 */
    char        generated_description[FC_DESCRIPTION_LEN];              /* 应该存放git hash、生成日期、 比如 git@0fb003003f_dirty time@20240604_160211 */
    uint8_t  oter_data[0];
}FactoryData ;

extern const FactoryData* FactoryData_detect(const void *part_start_addr, uint32_t part_size);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif // _FACTORY_DATA_H_