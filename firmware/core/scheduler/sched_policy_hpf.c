/**
 * @file    sched_policy.c
 * @brief   SchedTiny Scheduling Policy — Highest-Priority-First (HPF).
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_policy.h"

#include <string.h>

SchedStatus_t sched_policy_init(sched_policy_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_policy_t));

    SchedStatus_t rc = sched_rq_clear(&ctx->queue);
    if (rc != SCHED_OK)
    {
        return rc;
    }

    ctx->state = SCHED_POLICY_READY;
    return SCHED_OK;
}

SchedStatus_t sched_policy_reset(sched_policy_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (ctx->state != SCHED_POLICY_READY)
    {
        return SCHED_ERR_STATE;
    }

    return sched_rq_clear(&ctx->queue);
}

SchedStatus_t sched_policy_add_task(sched_policy_t *ctx, uint32_t task_id, uint32_t priority)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (ctx->state != SCHED_POLICY_READY)
    {
        return SCHED_ERR_STATE;
    }

    return sched_rq_enqueue(&ctx->queue, task_id, priority);
}

SchedStatus_t sched_policy_remove_task(sched_policy_t *ctx, uint32_t task_id)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (ctx->state != SCHED_POLICY_READY)
    {
        return SCHED_ERR_STATE;
    }

    return sched_rq_remove(&ctx->queue, task_id);
}

SchedStatus_t sched_policy_select_next(sched_policy_t *ctx, uint32_t *out_task_id)
{
    if (ctx == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (ctx->state != SCHED_POLICY_READY)
    {
        return SCHED_ERR_STATE;
    }

    return sched_rq_dequeue(&ctx->queue, out_task_id);
}

bool sched_policy_has_ready_task(const sched_policy_t *ctx)
{
    if (ctx == NULL || ctx->state != SCHED_POLICY_READY)
    {
        return false;
    }

    return !sched_rq_is_empty(&ctx->queue);
}

SchedStatus_t sched_policy_peek_next(const sched_policy_t *ctx, uint32_t *out_task_id)
{
    if (ctx == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (ctx->state != SCHED_POLICY_READY)
    {
        return SCHED_ERR_STATE;
    }

    if (sched_rq_is_empty(&ctx->queue))
    {
        return SCHED_ERR_NOT_FOUND;
    }

    /* The queue is sorted descending; entry[0] is highest priority. */
    *out_task_id = ctx->queue.entries[0].task_id;
    return SCHED_OK;
}

SchedStatus_t sched_policy_dispatch_candidate(sched_policy_t *ctx, uint32_t *out_task_id)
{
    return sched_policy_select_next(ctx, out_task_id);
}
