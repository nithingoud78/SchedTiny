/**
 * @file    sched_isr.h
 * @brief   SchedTiny ISR hook API for interrupt-aware scheduling.
 *
 * This module provides the ISR hook mechanism that is the core research
 * contribution of SchedTiny. By instrumenting ISR entry/exit points,
 * the scheduler can:
 *   - Attribute interrupt-induced latency to specific IRQ sources
 *   - Compute per-source jitter distributions
 *   - Feed measured jitter into schedulability analysis
 *
 * Usage pattern in an ISR:
 * @code
 *   void TIM2_IRQHandler(void)
 *   {
 *       sched_isr_on_entry(SCHED_ISR_TIM2);
 *       // ... ISR body ...
 *       sched_isr_on_exit(SCHED_ISR_TIM2);
 *   }
 * @endcode
 *
 * @see     docs/SPEC.md REQ-SCHED-001, REQ-MEAS-001, REQ-MEAS-003
 * @see     references/research.md Section D, Contribution D.1
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_ISR_H
#define SCHEDTINY_SCHED_ISR_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * ISR Source Identifiers
 * ========================================================================= */

/**
 * @brief   Identifiers for interrupt sources tracked by the ISR hook.
 *
 * Add new IRQ sources here as needed. Keep in sync with
 * firmware/boards/<board>/board_config.h IRQ priority assignments.
 */
typedef enum
{
    SCHED_ISR_TIM2   = 0,   /**< TIM2: ISR load generator (configurable rate) */
    SCHED_ISR_TIM3   = 1,   /**< TIM3: ISR load generator (backup) */
    SCHED_ISR_DMA1   = 2,   /**< DMA1: UART TX DMA (bench log) */
    SCHED_ISR_I2C1   = 3,   /**< I²C1: MPU6050 sensor */
    SCHED_ISR_I2C2   = 4,   /**< I²C2: INA219 power monitor */
    SCHED_ISR_USART3 = 5,   /**< USART3: ST-LINK VCP (if used for RX) */
    SCHED_ISR_COUNT  = 6,   /**< Total number of tracked ISR sources */
} SchedIsrSource_t;

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief   Initialize the ISR hook subsystem.
 *
 * Clears all accumulated jitter statistics. Must be called from sched_init().
 * Thread-safe: No.
 */
void sched_isr_init(void);

/**
 * @brief   Record ISR entry timestamp.
 *
 * MUST be called as the FIRST instruction in every tracked ISR.
 * Uses DWT->CYCCNT for zero-overhead timing (no interrupt required).
 *
 * @param[in] source  The ISR source identifier (SchedIsrSource_t).
 *
 * @note    This function is interrupt-safe and has < 10 CPU cycles overhead.
 */
void sched_isr_on_entry(SchedIsrSource_t source);

/**
 * @brief   Record ISR exit and compute latency contribution.
 *
 * MUST be called as the LAST instruction in every tracked ISR before
 * returning. Computes the ISR execution time and updates the jitter
 * ring buffer for the bench log.
 *
 * @param[in] source  The ISR source identifier (SchedIsrSource_t).
 */
void sched_isr_on_exit(SchedIsrSource_t source);

/**
 * @brief   Return the maximum observed ISR execution time for a source.
 *
 * @param[in] source  ISR source to query.
 * @return            Maximum observed execution time in CPU cycles.
 *                    Returns 0 if no ISR has been recorded yet.
 */
uint32_t sched_isr_get_max_cycles(SchedIsrSource_t source);

/**
 * @brief   Return the mean ISR execution time for a source (integer µs).
 *
 * @param[in] source  ISR source to query.
 * @return            Mean execution time in microseconds (rounded).
 */
uint32_t sched_isr_get_mean_us(SchedIsrSource_t source);

/**
 * @brief   Reset all accumulated ISR statistics for all sources.
 *
 * Typically called at the start of each measurement window.
 */
void sched_isr_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_ISR_H */
