# Renesas RA4M1 chip support

The local FLY startup follows the Cortex-M4F reset sequence documented in the Renesas FSP
[startup and system sources](https://github.com/renesas/fsp/tree/33f7320daf5549dfb7054be373ba2d0318b963a6/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source)
at revision `33f7320daf5549dfb7054be373ba2d0318b963a6` (FSP v6.2.0). That FSP
revision is a reference for the standalone implementation. The FSP attribution
is preserved in `LICENSE`. The device has 96 ICU event slots according to the
[FSP device header](https://github.com/renesas/fsp/blob/33f7320daf5549dfb7054be373ba2d0318b963a6/ra/fsp/src/bsp/cmsis/Device/RENESAS/Include/R7FA4M1AB.h).

The standalone initialization sets the application VTOR, enables the FPU, and
initializes the C runtime with newlib. The board and application own the clock,
pin mux and ICU configuration.
