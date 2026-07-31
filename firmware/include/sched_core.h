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

    /**
     * @brief   Initialize the RTOS scheduler and all background frameworks.
     *
     * This function initializes hardware, spawns the System Task, and passes control
     * to FreeRTOS. This function never returns.
     */
    void sched_core_start(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_CORE_H */
