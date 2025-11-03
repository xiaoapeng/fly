# https://developer.arm.com/downloads/-/gnu-rm
set(CROSS_COMPILE_PREFIX "")
set(CMAKE_SYSTEM_NAME Linux)

find_gcc_toolchain("${CROSS_COMPILE_PREFIX}")

# search for program/library/include in the build host directories
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
