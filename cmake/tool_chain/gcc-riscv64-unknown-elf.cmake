# https://github.com/riscv-collab/riscv-gnu-toolchain/releases/tag/2024.09.03
set(CROSS_COMPILE_PREFIX riscv64-unknown-elf-)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION  1)
set(CMAKE_SYSTEM_PROCESSOR  riscv64)

find_gcc_toolchain(${CROSS_COMPILE_PREFIX})
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# search for program/library/include in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
