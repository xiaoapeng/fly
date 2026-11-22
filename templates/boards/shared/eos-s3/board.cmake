set(FLY_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/timebase.cmake")

# The application executes in SRAM after the loader hands off at address zero.
set(FLY_FLASH_ADDRESS "0x00000000")
set(FLY_FLASH_SIZE "0x00027000")

function(fly_configure_board target)
    if(NOT TARGET arch-quicklogic-eos-s3)
        message(FATAL_ERROR "The board requires arch-quicklogic-eos-s3. Load its defconfig.")
    endif()

    target_sources(${target} PRIVATE "${FLY_BOARD_DIR}/board.c")
    target_include_directories(${target} PRIVATE "${FLY_BOARD_DIR}/include")
    target_link_libraries(${target} arch-quicklogic-eos-s3)
    set(linker_script "${FLY_BOARD_DIR}/sram.ld")
    target_link_options(${target} PRIVATE "-T${linker_script}")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()
