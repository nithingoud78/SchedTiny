/**
 * @file    sched_policy.h
 * @brief   SchedTiny Scheduling Policy — Highest-Priority-First (HPF).
 *
 * Provides a deterministic, static-memory scheduling policy that always
 * selects the highest-priority ready task.  FIFO ordering is used as
 * a tie-breaker for tasks at the same priority level.
 *
 * The policy layer owns a ready queue instance and exposes a
 * higher-level interface for the scheduler core to drive dispatching.
 *
 * No dynamic allocation.  No RTOS dependency.  Host-testable.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_POLICY_H
#define SCHEDTINY_SCHED_POLICY_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "sched_ready_queue.h"
#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Sentinel value returned when no task is available.
 */
#define SCHED_NO_TASK_ID UINT32_MAX

    /**
     * @brief   Scheduler policy state.
     */
    typedef enum
    {
        SCHED_POLICY_UNINIT = 0, /**< Not yet initialised */
        SCHED_POLICY_READY       /**< Ready for scheduling */
    } sched_policy_state_t;

    /**
     * @brief   Scheduler policy context.
     *
     * Embeds the ready queue and tracks its own lifecycle state.
     */
    typedef struct
    {
        sched_ready_queue_t queue;  /**< Embedded ready queue */
        sched_policy_state_t state; /**< Policy lifecycle */
    } sched_policy_t;

    /**
     * @brief   Initialise the scheduler policy.
     *
     * Clears the internal ready queue and transitions to READY state.
     *
     * @param   ctx  Pointer to the policy context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if ctx is NULL.
     */
    SchedStatus_t sched_policy_init(sched_policy_t *ctx);

    /**
     * @brief   Reset the scheduler policy to its initial state.
     *
     * Clears the ready queue and retains READY state.
     *
     * @param   ctx  Pointer to the policy context.
     * @return  SCHED_OK on success.
     *          SCHED_ERR_PARAM if ctx is NULL.
     *          SCHED_ERR_STATE if not initialised.
     */
    SchedStatus_t sched_policy_reset(sched_policy_t *ctx);

    /**
     * @brief   Add a task to the scheduler as ready.
     *
     * @param   ctx      Pointer to the policy context.
     * @param   task_id  Task identifier.
     * @param   priority Priority value (higher = more urgent).
     * @return  SCHED_OK on success.  Forwards ready-queue errors.
     */
    SchedStatus_t sched_policy_add_task(sched_policy_t *ctx, uint32_t task_id, uint32_t priority);

    /**
     * @brief   Remove a task from the scheduler.
     *
     * @param   ctx      Pointer to the policy context.
     * @param   task_id  Task identifier.
     * @return  SCHED_OK on success.  Forwards ready-queue errors.
     */
    SchedStatus_t sched_policy_remove_task(sched_policy_t *ctx, uint32_t task_id);

    /**
     * @brief   Select and dequeue the next task to run.
     *
     * Removes the highest-priority task from the ready queue and
     * writes its ID to *out_task_id.
     *
     * @param   ctx         Pointer to the policy context.
     * @param   out_task_id Receives the selected task ID.
     * @return  SCHED_OK on success.
     *          SCHED_ERR_NOT_FOUND if no task is ready.
     *          SCHED_ERR_PARAM / SCHED_ERR_STATE on bad input.
     */
    SchedStatus_t sched_policy_select_next(sched_policy_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Check whether any task is ready without modifying state.
     *
     * @param   ctx  Pointer to the policy context.
     * @return  true if at least one task is queued, false otherwise.
     */
    bool sched_policy_has_ready_task(const sched_policy_t *ctx);

    /**
     * @brief   Peek at the highest-priority task without dequeuing.
     *
     * @param   ctx         Pointer to the policy context.
     * @param   out_task_id Receives the task ID of the front entry.
     * @return  SCHED_OK on success.
     *          SCHED_ERR_NOT_FOUND if queue is empty.
     *          SCHED_ERR_PARAM on bad input.
     */
    SchedStatus_t sched_policy_peek_next(const sched_policy_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Convenience wrapper: peek + dequeue in one call.
     *
     * Equivalent to sched_policy_select_next but named to clarify
     * that this is the "dispatch candidate" the core should run next.
     *
     * @param   ctx         Pointer to the policy context.
     * @param   out_task_id Receives the dispatched task ID.
     * @return  Same semantics as sched_policy_select_next.
     */
    SchedStatus_t sched_policy_dispatch_candidate(sched_policy_t *ctx, uint32_t *out_task_id);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_POLICY_H */
