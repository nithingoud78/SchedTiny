/**
 * @file    hal_timer.h
 * @brief   Hardware Abstraction Layer — High-Resolution Timer.
 *
 * Provides a board-independent API for:
 *   - DWT cycle counter initialization and reading
 *   - Hardware timer configuration (TIM2/TIM3 for ISR load generation)
 *   - Microsecond delay (busy-wait, for initialization only)
 *
 * Board-specific register addresses are defined in boards/<board>/board_config.h.
 * This header is board-independent; it must not include any STM32 HAL headers.
 *
 * @see     docs/SPEC.md REQ-MEAS-001
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_HAL_TIMER_H
#define SCHEDTINY_HAL_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Returns the raw DWT cycle counter value (32-bit, wraps at 2^32 cycles). */
static inline uint32_t hal_timer_dwt_now(void)
{
    /* DWT->CYCCNT is a memory-mapped register at 0xE0001004.
     * Reading it costs exactly 1 cycle on Cortex-M7 with no cache stall.
     * Access via volatile pointer to prevent compiler optimization. */
    return *((volatile uint32_t *)0xE0001004U);
}

/**
 * @brief   Initialize the DWT cycle counter.
 *
 * Enables DWT tracing and clears the cycle counter.
 * Must be called once from main() before any measurement.
 */
void hal_timer_dwt_init(void);

/**
 * @brief   Initialize a hardware timer as an ISR load generator.
 *
 * Configures TIM2 or TIM3 to fire at the specified interrupt rate.
 * The ISR body is handled in sched_isr.c.
 *
 * @param[in] timer_id    0 = TIM2, 1 = TIM3
 * @param[in] rate_hz     Interrupt rate in Hz. 0 = disable.
 */
void hal_timer_isr_gen_init(uint8_t timer_id, uint32_t rate_hz);

/**
 * @brief   Busy-wait delay in microseconds.
 *
 * Uses DWT cycle counter. Accurate only after hal_timer_dwt_init().
 * Do NOT use in task code — use vTaskDelay() instead.
 * Intended for use during hardware initialization only.
 *
 * @param[in] us  Delay in microseconds.
 */
void hal_timer_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_HAL_TIMER_H */
