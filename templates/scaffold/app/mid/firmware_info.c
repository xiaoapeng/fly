#include "mid/firmware_info.h"

#include "board.h"
#include "factory-data.h"

#include <inttypes.h>
#include <stdio.h>

void fly_log_firmware_info(void)
{
    const FactoryData *info = factory_data_detect((const void *)(uintptr_t)FLY_FLASH_ADDRESS,
                                                  FLY_FLASH_SIZE);
    if (info == NULL) {
        board_log("Firmware info unavailable\n");
        return;
    }

    char message[256];
    snprintf(message, sizeof(message),
             "Firmware %.*s v%u.%u.%u\n"
             "Partition %.*s at 0x%08" PRIX32 "\n"
             "Image %" PRIu32 " bytes (aligned), trailer %" PRIu32 " bytes\n"
             "CRC32 0x%08" PRIX32 " (OK)\n"
             "Package %.*s\n",
             (int)sizeof(info->firmware_name), info->firmware_name,
             (unsigned)info->software_version.major, (unsigned)info->software_version.minor,
             (unsigned)info->software_version.patch,
             (int)sizeof(info->part_name), info->part_name, (uint32_t)FLY_FLASH_ADDRESS,
             info->software_len, info->factory_len, info->crc32,
             (int)sizeof(info->generated_description), info->generated_description);
    board_log(message);
}
