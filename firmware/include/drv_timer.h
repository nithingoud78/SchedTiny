/**
 * @file    drv_timer.h
 * @brief   Hardware Abstraction Layer — High-Resolution Timer.
 *
 * Provides a board-independent API for:
 *   - DWT cycle counter initialization and reading
 *   - Hardware timer configuration (TIM2/TIM3 for ISR load generation)
 *   - Microsecond delay (busy-wait, for initialization only)
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_DRV_TIMER_H
#define SCHEDTINY_DRV_TIMER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /** Returns the raw DWT cycle counter value (32-bit, wraps at 2^32 cycles). */
    uint32_t drv_timer_dwt_now(void);

    /**
     * @brief   Initialize the DWT cycle counter.
     *
     * Enables DWT tracing and clears the cycle counter.
     * Must be called once from main() before any measurement.
     */
    void drv_timer_dwt_init(void);

    /**
     * @brief   Initialize a hardware timer as an ISR load generator.
     *
     * @param[in] timer_id 0 = TIM2, 1 = TIM3
     * @param[in] rate_hz Interrupt rate in Hz. 0 = disable.
     */
    void drv_timer_isr_gen_init(uint8_t timer_id, uint32_t rate_hz);

    /**
     * @brief   Busy-wait delay in microseconds.
     *
     * Uses DWT cycle counter. Accurate only after drv_timer_dwt_init().
     *
     * @param[in] us Delay in microseconds.
     */
    void drv_timer_delay_us(uint32_t us);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_DRV_TIMER_H */
