# TI CC2538 chip support

The CC2538 target fetches the Texas Instruments startup file
from [Contiki-NG, revision `dba7cb6d`](https://github.com/contiki-ng/contiki-ng/blob/dba7cb6d98e1f435dc84b45ee08b0061d7c5628e/arch/cpu/cc2538/startup-gcc.c)
through the shared Git cache. The patch for that revision adapts the vector
table and reset sequence to use weak interrupt symbols, linker-provided
section boundaries, and newlib constructor initialization. The fetched
`startup-gcc.c` retains its original BSD-3-Clause notice.

Register reset values and interrupt priority width are described in
[TI SWRU319C](https://www.ti.com/lit/pdf/swru319). The CC2538SF53 target provides
the Cortex-M3 startup. Flash partitioning and CCA provisioning belong to the
board package.
