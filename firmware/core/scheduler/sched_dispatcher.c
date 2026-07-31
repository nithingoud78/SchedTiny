/**
 * @file    sched_dispatcher.c
 * @brief   SchedTiny Dispatcher Layer for executing scheduler decisions.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_dispatcher.h"

#include "sched_trace.h"

#include <string.h>

SchedStatus_t sched_dispatcher_init(sched_dispatcher_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_dispatcher_t));
    ctx->current_task_id = SCHED_DISPATCHER_NO_TASK;
    ctx->is_idle         = true;
    ctx->initialized     = true;

    return SCHED_OK;
}

SchedStatus_t sched_dispatcher_reset(sched_dispatcher_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    ctx->current_task_id      = SCHED_DISPATCHER_NO_TASK;
    ctx->context_switch_count = 0;
    ctx->tick_count           = 0;
    ctx->is_idle              = true;

    return SCHED_OK;
}

SchedStatus_t sched_dispatcher_dispatch(sched_dispatcher_t *ctx, uint32_t next_task_id)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (next_task_id == SCHED_DISPATCHER_NO_TASK)
    {
        return sched_dispatcher_idle(ctx);
    }

    /* Perform a context switch if the task has changed or we were idle */
    if (ctx->is_idle || ctx->current_task_id != next_task_id)
    {
        uint32_t prev_task = ctx->current_task_id;
        bool was_idle      = ctx->is_idle;

        ctx->context_switch_count++;
        ctx->current_task_id = next_task_id;
        ctx->is_idle         = false;

#if SCHED_CONFIG_ENABLE_TRACE
        if (was_idle)
        {
            sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_IDLE_EXIT, 0, 0, 0, 0, 0, 0, 0, 0);
        }
        else
        {
            sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_TASK_PREEMPT, prev_task, 0, 0, 0, 0,
                               0, 0, 0);
        }

        sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_CONTEXT_SWITCH, next_task_id, 0, 0, 0,
                           0, 0, 0, 0);
        sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_TASK_RESUME, next_task_id, 0, 0, 0, 0,
                           0, 0, 0);
#endif

        /*
         * Hardware-specific context switch logic (e.g. saving/restoring SP)
         * would normally be invoked here via a hook or inline assembly.
         */
    }

    return SCHED_OK;
}

SchedStatus_t sched_dispatcher_tick(sched_dispatcher_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    ctx->tick_count++;
    return SCHED_OK;
}

SchedStatus_t sched_dispatcher_idle(sched_dispatcher_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Transition to idle state if not already there */
    if (!ctx->is_idle)
    {
#if SCHED_CONFIG_ENABLE_TRACE
        sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_TASK_PREEMPT, ctx->current_task_id, 0,
                           0, 0, 0, 0, 0, 0);
        sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_CONTEXT_SWITCH, 0, 0, 0, 0, 0, 0, 0, 0);
        sched_trace_record(ctx->tick_count, SCHED_TRACE_EVT_IDLE_ENTER, 0, 0, 0, 0, 0, 0, 0, 0);
#endif

        ctx->context_switch_count++;
        ctx->current_task_id = SCHED_DISPATCHER_NO_TASK;
        ctx->is_idle         = true;
    }

    return SCHED_OK;
}

SchedStatus_t sched_dispatcher_current_task(const sched_dispatcher_t *ctx, uint32_t *out_task_id)
{
    if (ctx == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->is_idle)
    {
        return SCHED_ERR_STATE;
    }

    *out_task_id = ctx->current_task_id;
    return SCHED_OK;
}

SchedStatus_t sched_dispatcher_context_switch_count(const sched_dispatcher_t *ctx,
                                                    uint32_t *out_count)
{
    if (ctx == NULL || out_count == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    *out_count = ctx->context_switch_count;
    return SCHED_OK;
}
