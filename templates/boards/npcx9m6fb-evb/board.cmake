set(FLY_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")
include("${CMAKE_CURRENT_LIST_DIR}/timebase.cmake")

# NPCX9M6F enters the application in code SRAM after ROM boot.
set(FLY_FLASH_ADDRESS "0x10090000")
set(FLY_FLASH_SIZE "0x00030000")

function(fly_configure_board target)
    if(NOT TARGET arch-npcx9m6f)
        message(FATAL_ERROR "The board requires arch-npcx9m6f. Load its defconfig.")
    endif()

    target_sources(${target} PRIVATE "${FLY_BOARD_DIR}/board.c")
    target_include_directories(${target} PRIVATE "${FLY_BOARD_DIR}/include")
    target_link_libraries(${target} arch-npcx9m6f)
    set(linker_script "${FLY_BOARD_DIR}/sram.ld")
    target_link_options(${target} PRIVATE "-T${linker_script}")
    set_property(TARGET ${target} APPEND PROPERTY LINK_DEPENDS "${linker_script}")
endfunction()
