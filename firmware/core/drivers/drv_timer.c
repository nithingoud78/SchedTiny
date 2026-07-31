/**
 * @file    drv_timer.c
 * @brief   Implementation of hardware timer abstraction.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "drv_timer.h"

// STM32 Core Debug registers for DWT
#define DEMCR_REG          (*((volatile uint32_t *)0xE000EDFCU))
#define DEMCR_TRCENA       (1U << 24)
#define DWT_CTRL_REG       (*((volatile uint32_t *)0xE0001000U))
#define DWT_CTRL_CYCCNTENA (1U << 0)
#define DWT_CYCCNT_REG     (*((volatile uint32_t *)0xE0001004U))

void drv_timer_dwt_init(void)
{
    // Enable TRCENA
    DEMCR_REG |= DEMCR_TRCENA;

    // Reset cycle counter
    DWT_CYCCNT_REG = 0;

    // Enable cycle counter
    DWT_CTRL_REG |= DWT_CTRL_CYCCNTENA;
}

uint32_t drv_timer_dwt_now(void)
{
    // Access via volatile pointer to prevent compiler optimization.
    return DWT_CYCCNT_REG;
}

void drv_timer_isr_gen_init(uint8_t timer_id, uint32_t rate_hz)
{
    // Stubbed for future implementation.
    // In Milestone 2, this will configure TIM2 or TIM3 via direct register writes
    // to generate interrupts at the specified rate.
    (void)timer_id;
    (void)rate_hz;
}

void drv_timer_delay_us(uint32_t us)
{
    // Assuming 480 MHz system clock for H743ZI2.
    // Wait, since we are hardware-independent, we should probably pull SystemCoreClock
    // but to avoid CMSIS dependency for now, we hardcode or use a macro.
    // 1 us = 480 cycles.
    uint32_t cycles = us * 480;
    uint32_t start  = drv_timer_dwt_now();

    while ((drv_timer_dwt_now() - start) < cycles)
    {
        // Busy wait
    }
}
