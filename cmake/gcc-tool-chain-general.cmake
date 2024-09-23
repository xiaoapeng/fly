
function(add_target_make_bin ELF_FILE)
    add_custom_command(
        TARGET ${ELF_FILE}
        POST_BUILD
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

function(add_target_make_map TARGET_NAME)
    target_link_options(${TARGET_NAME}  PRIVATE
        "-Wl,-Map,${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.map"
    )
endfunction(add_target_make_map)

function(add_target_print_memory_usage TARGET_NAME)
    target_link_options(${TARGET_NAME}  PRIVATE
        "-Wl,--print-memory-usage"
    )
endfunction(add_target_print_memory_usage)

function(find_gcc_toolchain CROSS_COMPILE_PREFIX)
    if(DEFINED CMAKE_C_COMPILER )
        # 检查 C 编译器和 C++ 编译器路径是否包含 CROSS_COMPILE_PREFIX
        get_filename_component(CMAKE_C_COMPILER_PATH "${CMAKE_C_COMPILER}" ABSOLUTE)
        string(FIND "${CMAKE_C_COMPILER_PATH}" "${CROSS_COMPILE_PREFIX}" C_COMPILER_INDEX)

        # 如果编译器路径不包含 CROSS_COMPILE_PREFIX，则清除所有构建文件
        if (C_COMPILER_INDEX EQUAL -1)
            message(STATUS "Compiler paths do not contain CROSS_COMPILE_PREFIX, clearing build files.")
            unset(CMAKE_C_COMPILER CACHE )
            unset(CMAKE_CXX_COMPILER CACHE )
            unset(CMAKE_ASM_COMPILER CACHE )
            unset(CMAKE_OBJCOPY CACHE )
            unset(CMAKE_OBJDUMP CACHE )
            unset(CMAKE_AR CACHE )
            unset(CMAKE_SIZE CACHE )
            unset(CMAKE_GDB CACHE )
            unset(CMAKE_LINKER CACHE )
            unset(CMAKE_NM CACHE )
            unset(CMAKE_STRIP CACHE )
        endif()
    endif()

    find_program(CMAKE_C_COMPILER    ${CROSS_COMPILE_PREFIX}gcc PARENT_SCOPE)
    find_program(CMAKE_CXX_COMPILER  ${CROSS_COMPILE_PREFIX}g++ PARENT_SCOPE)
    find_program(CMAKE_ASM_COMPILER  ${CROSS_COMPILE_PREFIX}gcc PARENT_SCOPE)
    find_program(CMAKE_OBJCOPY       ${CROSS_COMPILE_PREFIX}objcopy PARENT_SCOPE)
    find_program(CMAKE_OBJDUMP       ${CROSS_COMPILE_PREFIX}objdump PARENT_SCOPE)
    find_program(CMAKE_AR            ${CROSS_COMPILE_PREFIX}ar PARENT_SCOPE)
    find_program(CMAKE_SIZE          ${CROSS_COMPILE_PREFIX}size PARENT_SCOPE)
    find_program(CMAKE_GDB           ${CROSS_COMPILE_PREFIX}gdb PARENT_SCOPE)
    find_program(CMAKE_GDB           gdb-multiarch PARENT_SCOPE)
    find_program(CMAKE_LINKER        ${CROSS_COMPILE_PREFIX}ld PARENT_SCOPE)
    find_program(CMAKE_NM            ${CROSS_COMPILE_PREFIX}nm PARENT_SCOPE)
    find_program(CMAKE_STRIP         ${CROSS_COMPILE_PREFIX}strip PARENT_SCOPE)

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
