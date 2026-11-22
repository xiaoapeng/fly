#include <stdbool.h>
#include <stdint.h>

/*
 * GCC emits this libatomic call for 32-bit CAS on Cortex-M0. Use a short
 * critical section for single-core code in thread mode or maskable ISRs.
 * The barriers provide sequential consistency for every requested order.
 */
bool fly_armv6m_compare_exchange_4(volatile uint32_t *object, uint32_t *expected,
                                  uint32_t desired, int success_order,
                                  int failure_order) __asm__("__atomic_compare_exchange_4");

bool fly_armv6m_compare_exchange_4(volatile uint32_t *object, uint32_t *expected,
                                  uint32_t desired, int success_order,
                                  int failure_order)
{
    (void)success_order;
    (void)failure_order;

    uint32_t interrupt_state;
    __asm__ volatile ("mrs %0, primask\n\tcpsid i\n\tdmb"
                      : "=r"(interrupt_state) :: "memory");
    const uint32_t observed = *object;
    const bool matched = observed == *expected;
    if (matched) {
        *object = desired;
    } else {
        *expected = observed;
    }
    __asm__ volatile ("dmb\n\tmsr primask, %0" :: "r"(interrupt_state) : "memory");
    return matched;
}
