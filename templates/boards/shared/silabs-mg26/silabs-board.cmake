set(FLY_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/timebase.cmake")
set(FLY_FLASH_ADDRESS 0x08000000)
# Use the first 399 pages for the image, as mapped by the supported model.
set(FLY_FLASH_SIZE 0x0031E000)

function(fly_configure_board target)
    if(NOT TARGET "${FLY_SILABS_ARCH_TARGET}")
        message(FATAL_ERROR "The board requires ${FLY_SILABS_ARCH_TARGET}. Load its defconfig.")
    endif()

    target_sources("${target}" PRIVATE "${FLY_BOARD_DIR}/board.c")
    target_include_directories("${target}" PRIVATE "${FLY_BOARD_DIR}/include")
    target_link_libraries("${target}" "${FLY_SILABS_ARCH_TARGET}" c m gcc)

    set(linker_script "${CMAKE_CURRENT_BINARY_DIR}/${target}.ld")
    configure_file("${FLY_BOARD_DIR}/flash.ld.in" "${linker_script}" @ONLY)
    target_link_options("${target}" PRIVATE "-T${linker_script}")
    set_property(TARGET "${target}" APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()
