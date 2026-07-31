/**
 * @file    sched_rms.h
 * @brief   SchedTiny Rate Monotonic Scheduling (RMS) Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_RMS_H
#define SCHEDTINY_SCHED_RMS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef SCHED_RMS_CAPACITY
#define SCHED_RMS_CAPACITY 8
#endif

/**
 * @brief Sentinel value for no task ID.
 */
#define SCHED_RMS_NO_TASK UINT32_MAX

    /**
     * @brief   Ready queue entry containing task period.
     */
    typedef struct
    {
        uint32_t task_id;
        uint32_t period;
    } sched_rms_queue_entry_t;

    /**
     * @brief   Registry entry for task periods.
     */
    typedef struct
    {
        uint32_t task_id;
        uint32_t period;
        bool active;
    } sched_rms_registry_entry_t;

    /**
     * @brief   RMS scheduler context.
     *
     * Contains the task period registry and the sorted ready queue.
     */
    typedef struct
    {
        sched_rms_registry_entry_t registry[SCHED_RMS_CAPACITY];
        sched_rms_queue_entry_t queue[SCHED_RMS_CAPACITY];
        uint32_t queue_count;
        bool initialized;
    } sched_rms_t;

    /**
     * @brief   Initialize the RMS scheduler context.
     * @param   ctx Pointer to RMS context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if ctx is NULL.
     */
    SchedStatus_t sched_rms_init(sched_rms_t *ctx);

    /**
     * @brief   Reset the RMS scheduler context to its initial state.
     * @param   ctx Pointer to RMS context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM/SCHED_ERR_STATE on failure.
     */
    SchedStatus_t sched_rms_reset(sched_rms_t *ctx);

    /**
     * @brief   Assign a period to a task ID.
     * @param   ctx     Pointer to RMS context.
     * @param   task_id Task identifier.
     * @param   period  Task period in system ticks (0 is invalid).
     * @return  SCHED_OK on success, SCHED_ERR_OVERFLOW if registry full.
     */
    SchedStatus_t sched_rms_assign_period(sched_rms_t *ctx, uint32_t task_id, uint32_t period);

    /**
     * @brief   Update the period for a previously assigned task.
     * @param   ctx        Pointer to RMS context.
     * @param   task_id    Task identifier.
     * @param   new_period New task period.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if task unassigned.
     */
    SchedStatus_t sched_rms_update_period(sched_rms_t *ctx, uint32_t task_id, uint32_t new_period);

    /**
     * @brief   Add a task to the ready queue based on its period.
     *          The period is retrieved from the registry.
     * @param   ctx     Pointer to RMS context.
     * @param   task_id Task identifier.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_rms_add_task(sched_rms_t *ctx, uint32_t task_id);

    /**
     * @brief   Remove a task from the ready queue.
     * @param   ctx     Pointer to RMS context.
     * @param   task_id Task identifier.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if not in queue.
     */
    SchedStatus_t sched_rms_remove_task(sched_rms_t *ctx, uint32_t task_id);

    /**
     * @brief   Check if there is at least one ready task.
     * @param   ctx Pointer to RMS context.
     * @return  true if queue has tasks, false otherwise.
     */
    bool sched_rms_has_ready_task(const sched_rms_t *ctx);

    /**
     * @brief   Peek at the task with the shortest period without removing it.
     * @param   ctx         Pointer to RMS context.
     * @param   out_task_id Pointer to store the result.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if empty.
     */
    SchedStatus_t sched_rms_peek_next(const sched_rms_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Remove and return the task with the shortest period.
     * @param   ctx         Pointer to RMS context.
     * @param   out_task_id Pointer to store the result.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if empty.
     */
    SchedStatus_t sched_rms_select_next(sched_rms_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Convenience wrapper for select_next (dispatch candidate).
     * @param   ctx         Pointer to RMS context.
     * @param   out_task_id Pointer to store the result.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_rms_dispatch_candidate(sched_rms_t *ctx, uint32_t *out_task_id);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_RMS_H */
