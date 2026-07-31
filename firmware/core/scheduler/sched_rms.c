/**
 * @file    sched_rms.c
 * @brief   SchedTiny Rate Monotonic Scheduling (RMS) Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_rms.h"

#include <string.h>

SchedStatus_t sched_rms_init(sched_rms_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_rms_t));
    ctx->initialized = true;

    return SCHED_OK;
}

SchedStatus_t sched_rms_reset(sched_rms_t *ctx)
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

SchedStatus_t sched_rms_assign_period(sched_rms_t *ctx, uint32_t task_id, uint32_t period)
{
    if (ctx == NULL || task_id == SCHED_RMS_NO_TASK || period == 0)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Check if already assigned to update it or avoid duplicate */
    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            ctx->registry[i].period = period;
            return SCHED_OK;
        }
    }

    /* Find empty slot */
    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        if (!ctx->registry[i].active)
        {
            ctx->registry[i].task_id = task_id;
            ctx->registry[i].period  = period;
            ctx->registry[i].active  = true;
            return SCHED_OK;
        }
    }

    return SCHED_ERR_OVERFLOW;
}

SchedStatus_t sched_rms_update_period(sched_rms_t *ctx, uint32_t task_id, uint32_t new_period)
{
    if (ctx == NULL || task_id == SCHED_RMS_NO_TASK || new_period == 0)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            ctx->registry[i].period = new_period;
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_rms_add_task(sched_rms_t *ctx, uint32_t task_id)
{
    if (ctx == NULL || task_id == SCHED_RMS_NO_TASK)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->queue_count >= SCHED_RMS_CAPACITY)
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

    /* Find period in registry */
    uint32_t period = 0;
    bool found      = false;
    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            period = ctx->registry[i].period;
            found  = true;
            break;
        }
    }

    if (!found)
    {
        return SCHED_ERR_NOT_FOUND; /* Task must be assigned a period first */
    }

    /* Find insertion point to maintain ascending order (shortest period first).
     * To preserve FIFO for equal periods, we skip past equal periods.
     * We insert before the first element that is strictly LARGER (longer period). */
    uint32_t pos = ctx->queue_count;
    for (uint32_t i = 0; i < ctx->queue_count; i++)
    {
        if (ctx->queue[i].period > period)
        {
            pos = i;
            break;
        }
    }

    /* Shift entries up to make room */
    if (pos < ctx->queue_count)
    {
        memmove(&ctx->queue[pos + 1], &ctx->queue[pos],
                (ctx->queue_count - pos) * sizeof(sched_rms_queue_entry_t));
    }

    ctx->queue[pos].task_id = task_id;
    ctx->queue[pos].period  = period;
    ctx->queue_count++;

    return SCHED_OK;
}

SchedStatus_t sched_rms_remove_task(sched_rms_t *ctx, uint32_t task_id)
{
    if (ctx == NULL || task_id == SCHED_RMS_NO_TASK)
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
                        remaining * sizeof(sched_rms_queue_entry_t));
            }
            ctx->queue_count--;
            memset(&ctx->queue[ctx->queue_count], 0, sizeof(sched_rms_queue_entry_t));
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

bool sched_rms_has_ready_task(const sched_rms_t *ctx)
{
    if (ctx == NULL || !ctx->initialized)
    {
        return false;
    }
    return ctx->queue_count > 0;
}

SchedStatus_t sched_rms_peek_next(const sched_rms_t *ctx, uint32_t *out_task_id)
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

SchedStatus_t sched_rms_select_next(sched_rms_t *ctx, uint32_t *out_task_id)
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
        memmove(&ctx->queue[0], &ctx->queue[1], ctx->queue_count * sizeof(sched_rms_queue_entry_t));
    }

    memset(&ctx->queue[ctx->queue_count], 0, sizeof(sched_rms_queue_entry_t));

    return SCHED_OK;
}

SchedStatus_t sched_rms_dispatch_candidate(sched_rms_t *ctx, uint32_t *out_task_id)
{
    return sched_rms_select_next(ctx, out_task_id);
}
