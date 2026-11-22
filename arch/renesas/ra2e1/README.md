# Renesas RA2E1 chip support

The local FLY startup follows the reset sequence documented in the Renesas FSP
[startup and system sources](https://github.com/renesas/fsp/tree/33f7320daf5549dfb7054be373ba2d0318b963a6/ra/fsp/src/bsp/cmsis/Device/RENESAS/Source)
at revision `33f7320daf5549dfb7054be373ba2d0318b963a6` (FSP v6.2.0). That FSP
revision is a reference for the standalone implementation. The startup sets
VTOR and initializes the C runtime through the FLY linker script and newlib.
It keeps the hardware reset clock and leaves pin, ICU, and peripheral
initialization to the application. The fixed exception slots use RA2E1's
Cortex-M23 profile without TrustZone. The FSP attribution is preserved in
`LICENSE`.

The nominal reset ICLK of 500 kHz is derived from the 8 MHz MOCO and divide-by-16
reset fields in the [Renesas RA2E1 hardware manual](https://www.renesas.com/en/document/man/ra2e1-group-users-manual-hardware).
