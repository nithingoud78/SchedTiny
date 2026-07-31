/**
 * @file    sched_edf.h
 * @brief   SchedTiny Earliest Deadline First (EDF) Scheduling Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_EDF_H
#define SCHEDTINY_SCHED_EDF_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef SCHED_EDF_CAPACITY
#define SCHED_EDF_CAPACITY 8
#endif

/**
 * @brief Sentinel value for no task ID.
 */
#define SCHED_EDF_NO_TASK UINT32_MAX

    /**
     * @brief   Ready queue entry containing absolute deadline.
     */
    typedef struct
    {
        uint32_t task_id;
        uint32_t abs_deadline;
    } sched_edf_queue_entry_t;

    /**
     * @brief   Registry entry for relative deadlines.
     */
    typedef struct
    {
        uint32_t task_id;
        uint32_t rel_deadline;
        bool active;
    } sched_edf_registry_entry_t;

    /**
     * @brief   EDF scheduler context.
     *
     * Contains the relative deadline registry and the sorted ready queue.
     */
    typedef struct
    {
        sched_edf_registry_entry_t registry[SCHED_EDF_CAPACITY];
        sched_edf_queue_entry_t queue[SCHED_EDF_CAPACITY];
        uint32_t queue_count;
        bool initialized;
    } sched_edf_t;

    /**
     * @brief   Initialize the EDF scheduler context.
     * @param   ctx Pointer to EDF context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if ctx is NULL.
     */
    SchedStatus_t sched_edf_init(sched_edf_t *ctx);

    /**
     * @brief   Reset the EDF scheduler context to its initial state.
     * @param   ctx Pointer to EDF context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM/SCHED_ERR_STATE on failure.
     */
    SchedStatus_t sched_edf_reset(sched_edf_t *ctx);

    /**
     * @brief   Assign a relative deadline to a task ID.
     * @param   ctx          Pointer to EDF context.
     * @param   task_id      Task identifier.
     * @param   rel_deadline Relative deadline in system ticks.
     * @return  SCHED_OK on success, SCHED_ERR_OVERFLOW if registry full.
     */
    SchedStatus_t sched_edf_assign_deadline(sched_edf_t *ctx,
                                            uint32_t task_id,
                                            uint32_t rel_deadline);

    /**
     * @brief   Update the relative deadline for a previously assigned task.
     * @param   ctx              Pointer to EDF context.
     * @param   task_id          Task identifier.
     * @param   new_rel_deadline New relative deadline.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if task unassigned.
     */
    SchedStatus_t sched_edf_update_deadline(sched_edf_t *ctx,
                                            uint32_t task_id,
                                            uint32_t new_rel_deadline);

    /**
     * @brief   Compare two absolute deadlines safely, handling overflow.
     * @param   d1 First absolute deadline.
     * @param   d2 Second absolute deadline.
     * @return  true if d1 is earlier than or equal to d2.
     */
    bool sched_edf_deadline_is_earlier(uint32_t d1, uint32_t d2);

    /**
     * @brief   Add a task to the ready queue based on its absolute deadline.
     *          The absolute deadline is computed as current_time + rel_deadline.
     * @param   ctx          Pointer to EDF context.
     * @param   task_id      Task identifier.
     * @param   current_time Current system time in ticks.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_edf_add_task(sched_edf_t *ctx, uint32_t task_id, uint32_t current_time);

    /**
     * @brief   Remove a task from the ready queue.
     * @param   ctx     Pointer to EDF context.
     * @param   task_id Task identifier.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if not in queue.
     */
    SchedStatus_t sched_edf_remove_task(sched_edf_t *ctx, uint32_t task_id);

    /**
     * @brief   Check if there is at least one ready task.
     * @param   ctx Pointer to EDF context.
     * @return  true if queue has tasks, false otherwise.
     */
    bool sched_edf_has_ready_task(const sched_edf_t *ctx);

    /**
     * @brief   Peek at the task with the earliest deadline without removing it.
     * @param   ctx         Pointer to EDF context.
     * @param   out_task_id Pointer to store the result.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if empty.
     */
    SchedStatus_t sched_edf_peek_next(const sched_edf_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Remove and return the task with the earliest deadline.
     * @param   ctx         Pointer to EDF context.
     * @param   out_task_id Pointer to store the result.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if empty.
     */
    SchedStatus_t sched_edf_select_next(sched_edf_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Convenience wrapper for select_next (dispatch candidate).
     * @param   ctx         Pointer to EDF context.
     * @param   out_task_id Pointer to store the result.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_edf_dispatch_candidate(sched_edf_t *ctx, uint32_t *out_task_id);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_EDF_H */
