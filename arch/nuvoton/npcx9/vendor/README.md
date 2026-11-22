# NPCX9M6F application startup

This target uses Arm's generic Cortex-M4 GCC reset and core exception setup from
CMSIS 5.9.0, fetched into FLY's shared cache at commit
`2b7495b8535bdcb306dac29b9ded4cfb679d7e5c` under Apache-2.0.
A peripheral application needs an appropriate NPCX9 interrupt table and driver setup.
The board's SRAM linker script derives from the same CMSIS source and includes
its license in the generated project.

The boot ROM on real hardware reads a Nuvoton header from SPI flash and loads the
program into executable SRAM. The Renode startup smoke test goes directly to
the linked application at `0x10090000` and checks its boot log and CRC.

- [CMSIS Cortex-M4 startup and linker template](https://github.com/ARM-software/CMSIS_5/tree/2b7495b8535bdcb306dac29b9ded4cfb679d7e5c/Device/ARM/ARMCM4/Source/GCC)
- [CMSIS license](https://github.com/ARM-software/CMSIS_5/blob/2b7495b8535bdcb306dac29b9ded4cfb679d7e5c/LICENSE.txt)
- [Chromium EC NPCX9M6F memory layout](https://chromium.googlesource.com/chromiumos/platform/ec/+/08f5a1e6fc2c9467230444ac9b582dcf4d9f0068/chip/npcx/config_chip-npcx9.h)
- [Renode NPCX9 boot ROM model](https://github.com/renode/renode/blob/ab721d88e135a1bcb8ed2ecc5a38f51cbe61fdd2/scripts/pydev/nuvoton_npcx9_bootrom.py)
