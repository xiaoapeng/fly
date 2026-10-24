# https://developer.arm.com/downloads/-/gnu-rm
set(CMAKE_SYSTEM_NAME Darwin)

find_program(TMP_CMAKE_C_COMPILER    clang NO_CACHE)
find_program(TMP_CMAKE_CXX_COMPILER  clang++ NO_CACHE)
find_program(TMP_CMAKE_ASM_COMPILER  clang NO_CACHE)
find_program(TMP_CMAKE_LINKER        ld NO_CACHE)

set(CMAKE_C_COMPILER "${TMP_CMAKE_C_COMPILER}" CACHE INTERNAL "C compiler")
set(CMAKE_CXX_COMPILER "${TMP_CMAKE_CXX_COMPILER}" CACHE INTERNAL "CXX compiler")
set(CMAKE_ASM_COMPILER "${TMP_CMAKE_ASM_COMPILER}" CACHE INTERNAL "ASM compiler")
set(CMAKE_LINKER "${TMP_CMAKE_LINKER}" CACHE INTERNAL "LD")

message(STATUS "Using CC : ${CMAKE_C_COMPILER}")
message(STATUS "Using CXX: ${CMAKE_CXX_COMPILER}")
message(STATUS "Using ASM: ${CMAKE_ASM_COMPILER}")
message(STATUS "Using LD : ${CMAKE_LINKER}")

# lld 只在机器上真的存在时才用：clang 的 -fuse-ld=lld 找不到 ld.lld 会直接
# 报 "invalid linker name"，而纯净的 macOS 只有 Apple ld。装了 lld
# (brew install lld) 就走 lld，没装就用系统默认链接器。
find_program(TMP_CMAKE_LLD ld.lld NO_CACHE)
if(TMP_CMAKE_LLD)
    message(STATUS "Using LLD: ${TMP_CMAKE_LLD}")
    add_link_options("-fuse-ld=lld")
else()
    message(STATUS "ld.lld not found, using the default linker")
endif()

# search for program/library/include in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
