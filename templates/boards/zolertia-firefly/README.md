# Zolertia Firefly

The CC2538SF53 has 512 KiB of flash at `0x00200000` and 32 KiB of SRAM at
`0x20000000`. The bare-metal template keeps the 16 MHz reset clock.

The final 2 KiB flash erase page (`0x0027F800` through `0x0027FFFF`) contains the
TI Customer Configuration Area (CCA) starting at `0x0027FFD4`. FLY's APP
partition ends before this page, leaving room for the firmware metadata trailer
without overwriting the CCA. Renode loads the APP directly. Before booting on
physical hardware, separately provision a valid CCA with the APP vector table
address `0x00200000`, and preserve that page during APP updates.

See the [Zolertia Firefly hardware description](https://github.com/Zolertia/Resources/wiki/Firefly)
and [TI's serial bootloader documentation](https://www.ti.com/lit/pdf/swra466).
