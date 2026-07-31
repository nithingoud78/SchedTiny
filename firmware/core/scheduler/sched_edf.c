/**
 * @file    sched_edf.c
 * @brief   SchedTiny Earliest Deadline First (EDF) Scheduling Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_edf.h"

#include <string.h>

bool sched_edf_deadline_is_earlier(uint32_t d1, uint32_t d2)
{
    /* Use signed 32-bit arithmetic to handle overflow correctly.
       If (d1 - d2) is negative, d1 is earlier.
       If it is 0, they are equal. */
    return ((int32_t)(d1 - d2)) <= 0;
}

SchedStatus_t sched_edf_init(sched_edf_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_edf_t));
    ctx->initialized = true;

    return SCHED_OK;
}

SchedStatus_t sched_edf_reset(sched_edf_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Retain registry, just clear the ready queue */
    memset(ctx->queue, 0, sizeof(ctx->queue));
    ctx->queue_count = 0;

    return SCHED_OK;
}

SchedStatus_t sched_edf_assign_deadline(sched_edf_t *ctx, uint32_t task_id, uint32_t rel_deadline)
{
    if (ctx == NULL || task_id == SCHED_EDF_NO_TASK)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Check if already assigned to update it or avoid duplicate */
    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            ctx->registry[i].rel_deadline = rel_deadline;
            return SCHED_OK;
        }
    }

    /* Find empty slot */
    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        if (!ctx->registry[i].active)
        {
            ctx->registry[i].task_id      = task_id;
            ctx->registry[i].rel_deadline = rel_deadline;
            ctx->registry[i].active       = true;
            return SCHED_OK;
        }
    }

    return SCHED_ERR_OVERFLOW;
}

SchedStatus_t sched_edf_update_deadline(sched_edf_t *ctx,
                                        uint32_t task_id,
                                        uint32_t new_rel_deadline)
{
    if (ctx == NULL || task_id == SCHED_EDF_NO_TASK)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            ctx->registry[i].rel_deadline = new_rel_deadline;
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_edf_add_task(sched_edf_t *ctx, uint32_t task_id, uint32_t current_time)
{
    if (ctx == NULL || task_id == SCHED_EDF_NO_TASK)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->queue_count >= SCHED_EDF_CAPACITY)
    {
        return SCHED_ERR_OVERFLOW;
    }

    /* Prevent duplicates in the ready queue */
    for (uint32_t i = 0; i < ctx->queue_count; i++)
    {
        if (ctx->queue[i].task_id == task_id)
        {
            return SCHED_ERR_STATE;
        }
    }

    /* Find relative deadline in registry */
    uint32_t rel_deadline = 0;
    bool found            = false;
    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            rel_deadline = ctx->registry[i].rel_deadline;
            found        = true;
            break;
        }
    }

    if (!found)
    {
        return SCHED_ERR_NOT_FOUND; /* Task must be assigned a deadline first */
    }

    uint32_t abs_deadline = current_time + rel_deadline;

    /* Find insertion point to maintain ascending order (earliest first).
     * To preserve FIFO for equal deadlines, we skip past equal deadlines.
     * We insert before the first element that is strictly LATER. */
    uint32_t pos = ctx->queue_count;
    for (uint32_t i = 0; i < ctx->queue_count; i++)
    {
        if (((int32_t)(ctx->queue[i].abs_deadline - abs_deadline)) > 0)
        {
            pos = i;
            break;
        }
    }

    /* Shift entries up to make room */
    if (pos < ctx->queue_count)
    {
        memmove(&ctx->queue[pos + 1], &ctx->queue[pos],
                (ctx->queue_count - pos) * sizeof(sched_edf_queue_entry_t));
    }

    ctx->queue[pos].task_id      = task_id;
    ctx->queue[pos].abs_deadline = abs_deadline;
    ctx->queue_count++;

    return SCHED_OK;
}

SchedStatus_t sched_edf_remove_task(sched_edf_t *ctx, uint32_t task_id)
{
    if (ctx == NULL || task_id == SCHED_EDF_NO_TASK)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    for (uint32_t i = 0; i < ctx->queue_count; i++)
    {
        if (ctx->queue[i].task_id == task_id)
        {
            uint32_t remaining = ctx->queue_count - i - 1;
            if (remaining > 0)
            {
                memmove(&ctx->queue[i], &ctx->queue[i + 1],
                        remaining * sizeof(sched_edf_queue_entry_t));
            }
            ctx->queue_count--;
            memset(&ctx->queue[ctx->queue_count], 0, sizeof(sched_edf_queue_entry_t));
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

bool sched_edf_has_ready_task(const sched_edf_t *ctx)
{
    if (ctx == NULL || !ctx->initialized)
    {
        return false;
    }
    return ctx->queue_count > 0;
}

SchedStatus_t sched_edf_peek_next(const sched_edf_t *ctx, uint32_t *out_task_id)
{
    if (ctx == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->queue_count == 0)
    {
        return SCHED_ERR_NOT_FOUND;
    }

    *out_task_id = ctx->queue[0].task_id;
    return SCHED_OK;
}

SchedStatus_t sched_edf_select_next(sched_edf_t *ctx, uint32_t *out_task_id)
{
    if (ctx == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->queue_count == 0)
    {
        return SCHED_ERR_NOT_FOUND;
    }

    *out_task_id = ctx->queue[0].task_id;

    ctx->queue_count--;
    if (ctx->queue_count > 0)
    {
        memmove(&ctx->queue[0], &ctx->queue[1], ctx->queue_count * sizeof(sched_edf_queue_entry_t));
    }

    memset(&ctx->queue[ctx->queue_count], 0, sizeof(sched_edf_queue_entry_t));

    return SCHED_OK;
}

SchedStatus_t sched_edf_dispatch_candidate(sched_edf_t *ctx, uint32_t *out_task_id)
{
    return sched_edf_select_next(ctx, out_task_id);
}
