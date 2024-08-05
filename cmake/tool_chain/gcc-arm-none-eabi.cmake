# the name of the target operating system
set(CROSS_COMPILE_PREFIX arm-none-eabi-)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION  1)
set(CMAKE_SYSTEM_PROCESSOR  cortex-m33)


find_program(CMAKE_C_COMPILER    ${CROSS_COMPILE_PREFIX}gcc)
find_program(CMAKE_CXX_COMPILER  ${CROSS_COMPILE_PREFIX}g++)
find_program(CMAKE_ASM_COMPILER  ${CROSS_COMPILE_PREFIX}gcc)
find_program(CMAKE_OBJCOPY       ${CROSS_COMPILE_PREFIX}objcopy)
find_program(CMAKE_OBJDUMP       ${CROSS_COMPILE_PREFIX}objdump)
find_program(CMAKE_AR            ${CROSS_COMPILE_PREFIX}ar)
find_program(CMAKE_SIZE          ${CROSS_COMPILE_PREFIX}size)
find_program(CMAKE_GDB           ${CROSS_COMPILE_PREFIX}gdb)
find_program(CMAKE_GDB           gdb-multiarch)
find_program(CMAKE_LINKER        ${CROSS_COMPILE_PREFIX}ld)
find_program(CMAKE_NM            ${CROSS_COMPILE_PREFIX}nm)
find_program(CMAKE_STRIP         ${CROSS_COMPILE_PREFIX}strip)

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

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# search for program/library/include in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
