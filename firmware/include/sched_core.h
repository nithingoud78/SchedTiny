/**
 * @file    sched_core.h
 * @brief   SchedTiny Core RTOS Initialization API.
 *
 * Provides the RTOS boot sequence and exposes global handles if necessary.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#ifndef SCHEDTINY_SCHED_CORE_H
#define SCHEDTINY_SCHED_CORE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

    /**
     * @brief   Global state of the SchedTiny RTOS scheduler.
     */
    typedef enum
    {
        SCHED_STATE_UNINIT = 0, /**< Not initialized */
        SCHED_STATE_READY,      /**< Initialized, ready to start */
        SCHED_STATE_RUNNING,    /**< FreeRTOS scheduler running */
        SCHED_STATE_HALTED      /**< Halted due to fatal error or manual stop */
    } sched_state_t;

    /**
     * @brief   Initialize the SchedTiny core context.
     * @return  SCHED_OK on success, SCHED_ERR_STATE if already initialized.
     */
    SchedStatus_t sched_core_init(void);

    /**
     * @brief   Get the current state of the SchedTiny scheduler.
     * @return  The current sched_state_t.
     */
    sched_state_t sched_core_get_state(void);

    /**
     * @brief   Initialize the RTOS scheduler and all background frameworks.
     *
     * This function initializes hardware, spawns the System Task, and passes
     * control to FreeRTOS. This function never returns.
     */
    void sched_core_start(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_CORE_H */
