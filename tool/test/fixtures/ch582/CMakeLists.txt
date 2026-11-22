cmake_minimum_required(VERSION 3.20)

include("${FLY_TOP_DIR}/cmake/tool-chain-general.cmake")
include("${FLY_ARCH_DIR}/gcc-config.cmake")
include("${FLY_TOP_DIR}/cmake/arch-config.cmake")

project(ch582_smoke C CXX ASM)
_fly_configure_architecture()
include("${FLY_TOP_DIR}/cmake/firmware-image.cmake")

set(FLY_WORKSPACE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")
set(FLY_MIRROR_SOURCE_JSON_FILE "${CMAKE_CURRENT_BINARY_DIR}/mirror.json")
file(WRITE "${FLY_MIRROR_SOURCE_JSON_FILE}" "{}\n")
add_subdirectory("${FLY_ARCH_DIR}" ch58x)
get_property(vendor_dir GLOBAL PROPERTY FLY_GIT_SOURCE_vendor-ch58x)
set(linker_script "${vendor_dir}/EVT/EXAM/SRC/Ld/Link.ld")

add_executable(ch582-smoke main.c)
target_compile_features(ch582-smoke PRIVATE c_std_11)
target_link_libraries(ch582-smoke PRIVATE arch-ch58x)
target_link_options(ch582-smoke PRIVATE "-T${linker_script}")
set_property(TARGET ch582-smoke APPEND PROPERTY LINK_DEPENDS "${linker_script}")
fly_add_target_bin(ch582-smoke)
fly_add_firmware_image(ch582
    IMAGE_NAME ch582-smoke
    BOARD_ID board/ch582-smoke
    CHIP ch582
    CORE_ID core0
    FIRMWARE_LIST "APP:1.0.0:0:0x70000:ch582-smoke.bin"
    SYMBOLS APP ch582-smoke
)
