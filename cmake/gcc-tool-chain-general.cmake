
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

    find_program(TMP_CMAKE_C_COMPILER    ${CROSS_COMPILE_PREFIX}gcc NO_CACHE)
    find_program(TMP_CMAKE_CXX_COMPILER  ${CROSS_COMPILE_PREFIX}g++ NO_CACHE)
    find_program(TMP_CMAKE_ASM_COMPILER  ${CROSS_COMPILE_PREFIX}gcc NO_CACHE)
    find_program(TMP_CMAKE_OBJCOPY       ${CROSS_COMPILE_PREFIX}objcopy NO_CACHE)
    find_program(TMP_CMAKE_OBJDUMP       ${CROSS_COMPILE_PREFIX}objdump NO_CACHE)
    find_program(TMP_CMAKE_AR            ${CROSS_COMPILE_PREFIX}ar NO_CACHE)
    find_program(TMP_CMAKE_SIZE          ${CROSS_COMPILE_PREFIX}size NO_CACHE)
    find_program(TMP_CMAKE_GDB           ${CROSS_COMPILE_PREFIX}gdb NO_CACHE)
    find_program(TMP_CMAKE_GDB           gdb-multiarch NO_CACHE)
    find_program(TMP_CMAKE_LINKER        ${CROSS_COMPILE_PREFIX}ld NO_CACHE)
    find_program(TMP_CMAKE_NM            ${CROSS_COMPILE_PREFIX}nm NO_CACHE)
    find_program(TMP_CMAKE_STRIP         ${CROSS_COMPILE_PREFIX}strip NO_CACHE)

    set(CMAKE_C_COMPILER "${TMP_CMAKE_C_COMPILER}" CACHE INTERNAL "C compiler")
    set(CMAKE_CXX_COMPILER "${TMP_CMAKE_CXX_COMPILER}" CACHE INTERNAL "CXX compiler")
    set(CMAKE_ASM_COMPILER "${TMP_CMAKE_ASM_COMPILER}" CACHE INTERNAL "ASM compiler")
    set(CMAKE_OBJCOPY "${TMP_CMAKE_OBJCOPY}" CACHE INTERNAL "OBJCOPY")
    set(CMAKE_OBJDUMP "${TMP_CMAKE_OBJDUMP}" CACHE INTERNAL "OBJDUMP")
    set(CMAKE_AR "${TMP_CMAKE_AR}" CACHE INTERNAL "AR")
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
