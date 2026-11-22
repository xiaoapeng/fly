# Leave flash space for the firmware trailer on 32 KiB application partitions.
if(DEFINED FLY_FLASH_SIZE AND FLY_FLASH_SIZE LESS_EQUAL 32768)
    target_compile_options(${FLY_PACK_TARGET} PRIVATE -Os)
    fly_add_package_compile_options("-Os")
endif()
