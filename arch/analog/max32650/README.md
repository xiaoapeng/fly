# MAX32650-series chip support

The MAX32652 target fetches the [Analog Devices MSDK GCC startup](https://github.com/analogdevicesinc/msdk/blob/92a7c488e1f81c3a2a279223c072598e7d7a53c6/Libraries/CMSIS/Device/Maxim/MAX32650/Source/GCC/startup_max32650.S)
at revision `92a7c488e1f81c3a2a279223c072598e7d7a53c6` through the shared Git
cache. The patch for that revision declares Cortex-M4 explicitly and provides
a weak SysTick handler for builds without the MSDK delay library. The startup
retains the upstream Apache-2.0 notice. The board's linker script takes the MSDK
layout and supplies the entry point and newlib `end` symbol used by FLY.

The FLY reset-clock initialization keeps the MSDK SRAM timing workaround,
enables the Cortex-M4 FPU, and selects the application interrupt vector. It
leaves peripheral and clock configuration to the application. The Flash and
RAM layout belongs to the selected board package.
