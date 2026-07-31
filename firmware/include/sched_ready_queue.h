/**
 * @file    sched_ready_queue.h
 * @brief   SchedTiny Ready Queue — priority-sorted static task queue.
 *
 * Provides a fixed-capacity, priority-ordered queue for scheduling
 * ready tasks. Higher priority values are dequeued first.
 * No dynamic memory allocation. No RTOS dependency.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_READY_QUEUE_H
#define SCHEDTINY_SCHED_READY_QUEUE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Maximum number of entries the ready queue can hold.
 */
#ifndef SCHED_READY_QUEUE_CAPACITY
#define SCHED_READY_QUEUE_CAPACITY 8
#endif

    /**
     * @brief   A single entry in the ready queue.
     */
    typedef struct
    {
        uint32_t task_id;  /**< Task identifier */
        uint32_t priority; /**< Priority (higher = more urgent) */
    } sched_rq_entry_t;

    /**
     * @brief   Ready queue control structure.
     *
     * Entries are maintained in descending priority order.
     * Entry [0] always has the highest priority.
     */
    typedef struct
    {
        sched_rq_entry_t entries[SCHED_READY_QUEUE_CAPACITY];
        uint32_t count; /**< Number of valid entries */
    } sched_ready_queue_t;

    /**
     * @brief   Initialize or reset the ready queue.
     * @param   q Pointer to the queue.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if q is NULL.
     */
    SchedStatus_t sched_rq_clear(sched_ready_queue_t *q);

    /**
     * @brief   Insert a task into the ready queue sorted by priority.
     *
     * Duplicate task_id entries are rejected.
     *
     * @param   q        Pointer to the queue.
     * @param   task_id  Task identifier.
     * @param   priority Priority value (higher = more urgent).
     * @return  SCHED_OK on success.
     *          SCHED_ERR_PARAM if q is NULL.
     *          SCHED_ERR_OVERFLOW if the queue is full.
     *          SCHED_ERR_STATE if task_id already exists.
     */
    SchedStatus_t sched_rq_enqueue(sched_ready_queue_t *q, uint32_t task_id, uint32_t priority);

    /**
     * @brief   Remove and return the highest-priority task from the queue.
     * @param   q           Pointer to the queue.
     * @param   out_task_id Pointer to store the dequeued task ID.
     * @return  SCHED_OK on success.
     *          SCHED_ERR_PARAM if any pointer is NULL.
     *          SCHED_ERR_NOT_FOUND if the queue is empty.
     */
    SchedStatus_t sched_rq_dequeue(sched_ready_queue_t *q, uint32_t *out_task_id);

    /**
     * @brief   Remove a specific task by ID from the queue.
     * @param   q       Pointer to the queue.
     * @param   task_id Task identifier to remove.
     * @return  SCHED_OK on success.
     *          SCHED_ERR_PARAM if q is NULL.
     *          SCHED_ERR_NOT_FOUND if task_id is not present.
     */
    SchedStatus_t sched_rq_remove(sched_ready_queue_t *q, uint32_t task_id);

    /**
     * @brief   Check whether a task is present in the ready queue.
     * @param   q       Pointer to the queue.
     * @param   task_id Task identifier to search for.
     * @return  true if found, false otherwise.
     */
    bool sched_rq_contains(const sched_ready_queue_t *q, uint32_t task_id);

    /**
     * @brief   Get the number of tasks currently in the queue.
     * @param   q Pointer to the queue.
     * @return  Current count, or 0 if q is NULL.
     */
    uint32_t sched_rq_size(const sched_ready_queue_t *q);

    /**
     * @brief   Check whether the queue is empty.
     * @param   q Pointer to the queue.
     * @return  true if empty or q is NULL.
     */
    bool sched_rq_is_empty(const sched_ready_queue_t *q);

    /**
     * @brief   Check whether the queue is full.
     * @param   q Pointer to the queue.
     * @return  true if full, false otherwise or if q is NULL.
     */
    bool sched_rq_is_full(const sched_ready_queue_t *q);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_READY_QUEUE_H */
