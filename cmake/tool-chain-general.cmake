
function(fly_add_target_bin ELF_FILE)
    add_custom_command(
        TARGET ${ELF_FILE}
        POST_BUILD
        BYPRODUCTS
            "${CMAKE_CURRENT_BINARY_DIR}/${ELF_FILE}.list"
            "${CMAKE_CURRENT_BINARY_DIR}/${ELF_FILE}.S"
            "${CMAKE_CURRENT_BINARY_DIR}/${ELF_FILE}.debug"
            "${CMAKE_CURRENT_BINARY_DIR}/${ELF_FILE}.hex"
            "${CMAKE_CURRENT_BINARY_DIR}/${ELF_FILE}.bin"
        COMMAND ${CMAKE_SIZE} --format=berkeley $<TARGET_FILE:${ELF_FILE}>
        COMMAND ${CMAKE_OBJDUMP} --all-headers --demangle --disassemble $<TARGET_FILE:${ELF_FILE}> > ${ELF_FILE}.list
        COMMAND ${CMAKE_OBJDUMP} -d -S -s $<TARGET_FILE:${ELF_FILE}> > ${ELF_FILE}.S
        COMMAND ${CMAKE_OBJCOPY} --only-keep-debug $<TARGET_FILE:${ELF_FILE}> ${ELF_FILE}.debug
        COMMAND ${CMAKE_OBJCOPY} --strip-debug $<TARGET_FILE:${ELF_FILE}> 
        COMMAND ${CMAKE_OBJCOPY} --add-gnu-debuglink=${ELF_FILE}.debug $<TARGET_FILE:${ELF_FILE}> 
        COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded -O ihex $<TARGET_FILE:${ELF_FILE}> ${ELF_FILE}.hex
        COMMAND ${CMAKE_OBJCOPY} --strip-debug --strip-unneeded -O binary $<TARGET_FILE:${ELF_FILE}> ${ELF_FILE}.bin
        COMMENT "Building ${ELF_FILE}\nBuilding ${ELF_FILE}.bin"
    )
endfunction()

function(fly_add_target_map TARGET_NAME)
    # 确保是目标存在
    if(NOT TARGET ${TARGET_NAME})
        message(FATAL_ERROR "Target ${TARGET_NAME} does not exist")
    endif()

    # 设置 map 文件输出路径
    set(MAP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.map")

    if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
        # GNU ld on Linux: -Wl,-Map=file.map
        target_link_options(${TARGET_NAME} PRIVATE
            "-Wl,-Map=${MAP_FILE}"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Generic")
        target_link_options(${TARGET_NAME} PRIVATE
            "-Wl,-Map=${MAP_FILE}"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        # ld64 on macOS: -Wl,-map,file.map
        target_link_options(${TARGET_NAME} PRIVATE
            "-Wl,-map,${MAP_FILE}"
        )
    elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
        # MSVC link.exe on Windows: /MAP:file.map
        if(MSVC)
            target_link_options(${TARGET_NAME} PRIVATE
                "/MAP:${MAP_FILE}"
            )
        else()
            # MinGW on Windows: -Wl,-Map=file.map
            target_link_options(${TARGET_NAME} PRIVATE
                "-Wl,-Map=${MAP_FILE}"
            )
        endif()
    elseif(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
        # FreeBSD 使用 GNU ld
        target_link_options(${TARGET_NAME} PRIVATE
            "-Wl,-Map=${MAP_FILE}"
        )
    else()
        # 其他系统，尝试 GNU 语法，输出警告
        message(WARNING "Unsupported system ${CMAKE_SYSTEM_NAME} for map file generation, trying GNU style")
        target_link_options(${TARGET_NAME} PRIVATE
            "-Wl,-Map=${MAP_FILE}"
        )
    endif()

    # 添加清理规则，清理 map 文件
    set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_CLEAN_FILES "${MAP_FILE}")
    
    # 输出信息
    message(STATUS "Map file will be generated: ${MAP_FILE}")
endfunction(fly_add_target_map)

function(fly_add_target_memory_report TARGET_NAME)
    target_link_options(${TARGET_NAME}  PRIVATE
        "-Wl,--print-memory-usage"
    )
endfunction(fly_add_target_memory_report)

function(_fly_find_gcc_toolchain CROSS_COMPILE_PREFIX)

    find_program(TMP_CMAKE_C_COMPILER    ${CROSS_COMPILE_PREFIX}gcc NO_CACHE)
    find_program(TMP_CMAKE_CXX_COMPILER  ${CROSS_COMPILE_PREFIX}g++ NO_CACHE)
    find_program(TMP_CMAKE_ASM_COMPILER  ${CROSS_COMPILE_PREFIX}gcc NO_CACHE)
    find_program(TMP_CMAKE_OBJCOPY       ${CROSS_COMPILE_PREFIX}objcopy NO_CACHE)
    find_program(TMP_CMAKE_OBJDUMP       ${CROSS_COMPILE_PREFIX}objdump NO_CACHE)
    find_program(TMP_CMAKE_SIZE          ${CROSS_COMPILE_PREFIX}size NO_CACHE)
    find_program(TMP_CMAKE_GDB           ${CROSS_COMPILE_PREFIX}gdb NO_CACHE)
    find_program(TMP_CMAKE_GDB           gdb-multiarch NO_CACHE)
    find_program(TMP_CMAKE_LINKER        ${CROSS_COMPILE_PREFIX}ld NO_CACHE)
    find_program(TMP_CMAKE_NM            ${CROSS_COMPILE_PREFIX}nm NO_CACHE)
    find_program(TMP_CMAKE_STRIP         ${CROSS_COMPILE_PREFIX}strip NO_CACHE)

    # GCC's archive tools load the matching LTO plugin when indexing objects.
    get_filename_component(compiler_dir "${TMP_CMAKE_C_COMPILER}" DIRECTORY)
    find_program(TMP_CMAKE_AR NAMES ${CROSS_COMPILE_PREFIX}gcc-ar
        PATHS "${compiler_dir}" NO_DEFAULT_PATH NO_CACHE)
    find_program(TMP_CMAKE_AR NAMES ${CROSS_COMPILE_PREFIX}ar NO_CACHE)
    find_program(TMP_CMAKE_RANLIB NAMES ${CROSS_COMPILE_PREFIX}gcc-ranlib
        PATHS "${compiler_dir}" NO_DEFAULT_PATH NO_CACHE)
    find_program(TMP_CMAKE_RANLIB NAMES ${CROSS_COMPILE_PREFIX}ranlib NO_CACHE)

    set(CMAKE_C_COMPILER "${TMP_CMAKE_C_COMPILER}" CACHE INTERNAL "C compiler")
    set(CMAKE_CXX_COMPILER "${TMP_CMAKE_CXX_COMPILER}" CACHE INTERNAL "CXX compiler")
    set(CMAKE_ASM_COMPILER "${TMP_CMAKE_ASM_COMPILER}" CACHE INTERNAL "ASM compiler")
    set(CMAKE_OBJCOPY "${TMP_CMAKE_OBJCOPY}" CACHE INTERNAL "OBJCOPY")
    set(CMAKE_OBJDUMP "${TMP_CMAKE_OBJDUMP}" CACHE INTERNAL "OBJDUMP")
    set(CMAKE_AR "${TMP_CMAKE_AR}" CACHE INTERNAL "AR")
    if(TMP_CMAKE_RANLIB)
        set(CMAKE_RANLIB "${TMP_CMAKE_RANLIB}" CACHE INTERNAL "Archive indexer")
    endif()
    set(CMAKE_SIZE "${TMP_CMAKE_SIZE}" CACHE INTERNAL "SIZE")
    set(CMAKE_GDB "${TMP_CMAKE_GDB}" CACHE INTERNAL "GDB")
    set(CMAKE_LINKER "${TMP_CMAKE_LINKER}" CACHE INTERNAL "LD")
    set(CMAKE_NM "${TMP_CMAKE_NM}" CACHE INTERNAL "NM")
    set(CMAKE_STRIP "${TMP_CMAKE_STRIP}" CACHE INTERNAL "STRIP")

    message(STATUS "Using CC : ${CMAKE_C_COMPILER}")
    message(STATUS "Using CXX: ${CMAKE_CXX_COMPILER}")
    message(STATUS "Using ASM: ${CMAKE_ASM_COMPILER}")
    message(STATUS "Using AR : ${CMAKE_AR}")
    message(STATUS "Using LD : ${CMAKE_LINKER}")
    message(STATUS "Using NM : ${CMAKE_NM}")
    message(STATUS "Using STRIP : ${CMAKE_STRIP}")
    message(STATUS "Using SIZE : ${CMAKE_SIZE}")
    message(STATUS "Using OBJCOPY : ${CMAKE_OBJCOPY}")
    message(STATUS "Using OBJDUMP : ${CMAKE_OBJDUMP}")
    message(STATUS "Using GDB : ${CMAKE_GDB}")
endfunction()
