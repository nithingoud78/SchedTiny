/**
 * @file    drv_gpio.c
 * @brief   Implementation of GPIO marker abstraction.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "drv_gpio.h"

// STM32 register addresses are typically found in boards/<board>/board_config.h
// Since we are decoupling from specific boards, we provide stub implementations
// that will be mapped to the actual memory registers in the board support layer.

SchedStatus_t drv_gpio_init(void)
{
    // Enable GPIO clocks and configure pins as push-pull outputs.
    // Stubbed.
    return SCHED_OK;
}

void drv_gpio_set(DrvGpioPin_t pin)
{
    // Direct BSRR register write based on pin mapping.
    (void)pin;
}

void drv_gpio_reset(DrvGpioPin_t pin)
{
    // Direct BSRR register write (reset half) based on pin mapping.
    (void)pin;
}

void drv_gpio_toggle(DrvGpioPin_t pin)
{
    // Direct ODR register read-modify-write.
    (void)pin;
}
