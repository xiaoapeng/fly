# Arduino UNO R4 Minima

The R7FA4M1AB3CFM has 256 KiB of code Flash and 32 KiB of SRAM. This template
reserves the first 16 KiB of Flash for the Arduino bootloader and places the
FLY APP and its metadata in the remaining 240 KiB. The application start
address `0x4000` comes from the [official MINIMA variant memory layout](https://github.com/arduino/ArduinoCore-renesas/blob/ba2cb9383709e193e8edea9b6bb37b03799d8cd1/variants/MINIMA/memory_regions.ld).

Renode starts the APP directly at `0x4000` using its vector table. Booting on
hardware through the stock Arduino path requires the bootloader to be present
in `0x0000–0x3FFF`. The [stock bootloader](https://github.com/arduino/arduino-renesas-bootloader/blob/581a59d0d9aa8e9ad21a43d62ea6fd063bc5fe19/src/main.c)
sets VTOR and MSP and leaves interrupts masked on entry. The FLY startup
unmasks them after initializing the runtime. USB DFU and the bootloader jump
are outside the Renode startup test.
