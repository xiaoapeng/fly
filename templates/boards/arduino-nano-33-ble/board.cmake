set(FLY_BOARD_DIR "${CMAKE_CURRENT_LIST_DIR}")

# ArduinoCore-mbed reserves 64 KiB at the beginning of Flash for its bootloader.
set(FLY_FLASH_ADDRESS "0x00010000")
set(FLY_FLASH_SIZE "0x000F0000")

include("${FLY_BOARD_DIR}/nrf52840-board.cmake")
