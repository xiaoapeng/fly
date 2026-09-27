/**
 * @file freertos_assert.c
 * @brief Default configASSERT handler for the FreeRTOS package.
 *
 * Provides a weak vAssertCalled() so every project using this package gets
 * a defined (if minimal) assertion policy out of the box: the core parks
 * with interrupts masked, leaving a debugging session free to inspect the
 * exact assert site (configASSERT call sites carry __FILE__/__LINE__).
 *
 * The package boundary deliberately has no knowledge of any debug output
 * backend (Segger RTT, UART, ...). Projects that want asserted output route
 * it from their own strong vAssertCalled(); here the symbol is left empty
 * on purpose.
 */

#include "FreeRTOS.h"
#include "task.h"

__attribute__((weak)) void vAssertCalled(const char *pcFile, unsigned long ulLine)
{
    (void)pcFile;
    (void)ulLine;
    taskDISABLE_INTERRUPTS();
    for (;;) {
    }
}
