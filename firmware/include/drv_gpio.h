/**
 * @file    drv_gpio.h
 * @brief   Hardware Abstraction Layer — GPIO Event Markers.
 *
 * Provides a board-independent API for:
 *   - Initializing specific debug/marker pins.
 *   - Setting, resetting, and toggling pins for logic analyzer triggering.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_DRV_GPIO_H
#define SCHEDTINY_DRV_GPIO_H

#include "schedtiny_status.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief   Logical identifiers for GPIO marker pins.
     */
    typedef enum
    {
        DRV_GPIO_MARKER_0 = 0,
        DRV_GPIO_MARKER_1 = 1,
        DRV_GPIO_MARKER_2 = 2,
        DRV_GPIO_MARKER_MAX
    } DrvGpioPin_t;

    /**
     * @brief   Initialize all configured marker GPIO pins.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t drv_gpio_init(void);

    /**
     * @brief   Set a GPIO marker pin High.
     * @param[in] pin Logical pin identifier.
     */
    void drv_gpio_set(DrvGpioPin_t pin);

    /**
     * @brief   Reset a GPIO marker pin Low.
     * @param[in] pin Logical pin identifier.
     */
    void drv_gpio_reset(DrvGpioPin_t pin);

    /**
     * @brief   Toggle a GPIO marker pin.
     * @param[in] pin Logical pin identifier.
     */
    void drv_gpio_toggle(DrvGpioPin_t pin);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_DRV_GPIO_H */
