/**
 * @file    sched_ready_queue.c
 * @brief   SchedTiny Ready Queue — priority-sorted static task queue.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_ready_queue.h"

#include <string.h>

SchedStatus_t sched_rq_clear(sched_ready_queue_t *q)
{
    if (q == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(q->entries, 0, sizeof(q->entries));
    q->count = 0;
    return SCHED_OK;
}

SchedStatus_t sched_rq_enqueue(sched_ready_queue_t *q, uint32_t task_id, uint32_t priority)
{
    if (q == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (q->count >= SCHED_READY_QUEUE_CAPACITY)
    {
        return SCHED_ERR_OVERFLOW;
    }

    /* Reject duplicates */
    for (uint32_t i = 0; i < q->count; i++)
    {
        if (q->entries[i].task_id == task_id)
        {
            return SCHED_ERR_STATE;
        }
    }

    /* Find insertion point — maintain descending priority order.
     * Walk from the front; insert before the first entry with lower priority. */
    uint32_t pos = q->count;
    for (uint32_t i = 0; i < q->count; i++)
    {
        if (priority > q->entries[i].priority)
        {
            pos = i;
            break;
        }
    }

    /* Shift entries down to make room */
    if (pos < q->count)
    {
        memmove(&q->entries[pos + 1], &q->entries[pos],
                (q->count - pos) * sizeof(sched_rq_entry_t));
    }

    q->entries[pos].task_id  = task_id;
    q->entries[pos].priority = priority;
    q->count++;

    return SCHED_OK;
}

SchedStatus_t sched_rq_dequeue(sched_ready_queue_t *q, uint32_t *out_task_id)
{
    if (q == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (q->count == 0)
    {
        return SCHED_ERR_NOT_FOUND;
    }

    /* Highest priority is always at index 0 */
    *out_task_id = q->entries[0].task_id;

    /* Shift remaining entries up */
    q->count--;
    if (q->count > 0)
    {
        memmove(&q->entries[0], &q->entries[1], q->count * sizeof(sched_rq_entry_t));
    }

    /* Zero the vacated slot */
    memset(&q->entries[q->count], 0, sizeof(sched_rq_entry_t));

    return SCHED_OK;
}

SchedStatus_t sched_rq_remove(sched_ready_queue_t *q, uint32_t task_id)
{
    if (q == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    for (uint32_t i = 0; i < q->count; i++)
    {
        if (q->entries[i].task_id == task_id)
        {
            /* Shift entries up to fill the gap */
            uint32_t remaining = q->count - i - 1;
            if (remaining > 0)
            {
                memmove(&q->entries[i], &q->entries[i + 1], remaining * sizeof(sched_rq_entry_t));
            }

            q->count--;
            memset(&q->entries[q->count], 0, sizeof(sched_rq_entry_t));

            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

bool sched_rq_contains(const sched_ready_queue_t *q, uint32_t task_id)
{
    if (q == NULL)
    {
        return false;
    }

    for (uint32_t i = 0; i < q->count; i++)
    {
        if (q->entries[i].task_id == task_id)
        {
            return true;
        }
    }

    return false;
}

uint32_t sched_rq_size(const sched_ready_queue_t *q)
{
    if (q == NULL)
    {
        return 0;
    }
    return q->count;
}

bool sched_rq_is_empty(const sched_ready_queue_t *q)
{
    if (q == NULL)
    {
        return true;
    }
    return q->count == 0;
}

bool sched_rq_is_full(const sched_ready_queue_t *q)
{
    if (q == NULL)
    {
        return false;
    }
    return q->count >= SCHED_READY_QUEUE_CAPACITY;
}
