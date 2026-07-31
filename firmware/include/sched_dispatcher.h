/**
 * @file    sched_dispatcher.h
 * @brief   SchedTiny Dispatcher Layer for executing scheduler decisions.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_DISPATCHER_H
#define SCHEDTINY_SCHED_DISPATCHER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Sentinel value for no task running (idle).
 */
#define SCHED_DISPATCHER_NO_TASK UINT32_MAX

    /**
     * @brief   Dispatcher state context.
     *
     * Tracks the currently running task, idle state, context switches,
     * and system ticks without tying into specific hardware.
     */
    typedef struct
    {
        uint32_t current_task_id;
        uint32_t context_switch_count;
        uint32_t tick_count;
        bool is_idle;
        bool initialized;
    } sched_dispatcher_t;

    /**
     * @brief   Initialize the dispatcher.
     * @param   ctx Pointer to dispatcher context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if NULL.
     */
    SchedStatus_t sched_dispatcher_init(sched_dispatcher_t *ctx);

    /**
     * @brief   Reset the dispatcher state.
     * @param   ctx Pointer to dispatcher context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM/SCHED_ERR_STATE on failure.
     */
    SchedStatus_t sched_dispatcher_reset(sched_dispatcher_t *ctx);

    /**
     * @brief   Dispatch a task (switch context if changed).
     * @param   ctx          Pointer to dispatcher context.
     * @param   next_task_id The task ID selected by the scheduler to run next.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_dispatcher_dispatch(sched_dispatcher_t *ctx, uint32_t next_task_id);

    /**
     * @brief   Handle system tick in the dispatcher.
     * @param   ctx Pointer to dispatcher context.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_dispatcher_tick(sched_dispatcher_t *ctx);

    /**
     * @brief   Force the dispatcher into an idle state.
     * @param   ctx Pointer to dispatcher context.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_dispatcher_idle(sched_dispatcher_t *ctx);

    /**
     * @brief   Get the currently running task ID.
     * @param   ctx         Pointer to dispatcher context.
     * @param   out_task_id Pointer to store the current task ID.
     * @return  SCHED_OK on success, SCHED_ERR_STATE if idle.
     */
    SchedStatus_t sched_dispatcher_current_task(const sched_dispatcher_t *ctx,
                                                uint32_t *out_task_id);

    /**
     * @brief   Get the total number of context switches.
     * @param   ctx       Pointer to dispatcher context.
     * @param   out_count Pointer to store the count.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_dispatcher_context_switch_count(const sched_dispatcher_t *ctx,
                                                        uint32_t *out_count);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_DISPATCHER_H */
