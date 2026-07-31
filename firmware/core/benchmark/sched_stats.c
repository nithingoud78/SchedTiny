/**
 * @file    sched_stats.c
 * @brief   SchedTiny Runtime Statistics & Benchmark Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_stats.h"

#include <stdio.h>
#include <string.h>

SchedStatus_t sched_stats_init(sched_stats_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_stats_t));
    ctx->data.min_scheduling_latency = UINT32_MAX;
    ctx->initialized                 = true;

    return SCHED_OK;
}

SchedStatus_t sched_stats_reset(sched_stats_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    memset(&ctx->data, 0, sizeof(sched_stats_data_t));
    ctx->data.min_scheduling_latency = UINT32_MAX;
    ctx->active                      = false;
    ctx->start_time                  = 0;
    ctx->end_time                    = 0;

    return SCHED_OK;
}

SchedStatus_t sched_stats_start(sched_stats_t *ctx, uint32_t current_time)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->active)
    {
        return SCHED_ERR_STATE;
    }

    ctx->start_time = current_time;
    ctx->active     = true;

    return SCHED_OK;
}

SchedStatus_t sched_stats_stop(sched_stats_t *ctx, uint32_t current_time)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized || !ctx->active)
    {
        return SCHED_ERR_STATE;
    }

    ctx->end_time = current_time;
    ctx->active   = false;

    return SCHED_OK;
}

SchedStatus_t sched_stats_record_invocation(sched_stats_t *ctx)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }
    ctx->data.scheduler_invocation_count++;
    return SCHED_OK;
}

SchedStatus_t sched_stats_record_execution(sched_stats_t *ctx)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }
    ctx->data.scheduler_execution_count++;
    return SCHED_OK;
}

SchedStatus_t sched_stats_record_task_exec(sched_stats_t *ctx)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }
    ctx->data.task_execution_count++;
    return SCHED_OK;
}

SchedStatus_t sched_stats_record_context_switch(sched_stats_t *ctx)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }
    ctx->data.context_switch_count++;
    return SCHED_OK;
}

SchedStatus_t sched_stats_record_idle_time(sched_stats_t *ctx, uint32_t idle_ticks)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }
    ctx->data.idle_time += idle_ticks;
    return SCHED_OK;
}

SchedStatus_t sched_stats_record_busy_time(sched_stats_t *ctx, uint32_t busy_ticks)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }
    ctx->data.busy_time += busy_ticks;
    return SCHED_OK;
}

SchedStatus_t sched_stats_record_latency(sched_stats_t *ctx, uint32_t latency_ticks)
{
    if (ctx == NULL || !ctx->active)
    {
        return SCHED_ERR_PARAM;
    }

    ctx->data._total_latency += latency_ticks;
    ctx->data._latency_samples++;

    if (latency_ticks > ctx->data.max_scheduling_latency)
    {
        ctx->data.max_scheduling_latency = latency_ticks;
    }

    if (latency_ticks < ctx->data.min_scheduling_latency)
    {
        ctx->data.min_scheduling_latency = latency_ticks;
    }

    return SCHED_OK;
}

SchedStatus_t sched_stats_snapshot(sched_stats_t *ctx, sched_stats_data_t *out_snapshot)
{
    if (ctx == NULL || out_snapshot == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Compute derived metrics */
    uint32_t total_time = ctx->data.idle_time + ctx->data.busy_time;
    if (total_time > 0)
    {
        /* Basis points (10000 = 100%) */
        uint64_t util             = (uint64_t)ctx->data.busy_time * 10000ULL;
        ctx->data.cpu_utilization = (uint32_t)(util / total_time);
    }
    else
    {
        ctx->data.cpu_utilization = 0;
    }

    if (ctx->data._latency_samples > 0)
    {
        ctx->data.avg_scheduling_latency = ctx->data._total_latency / ctx->data._latency_samples;
    }
    else
    {
        ctx->data.avg_scheduling_latency = 0;
    }

    /* Copy to snapshot */
    memcpy(out_snapshot, &ctx->data, sizeof(sched_stats_data_t));

    /* If no samples were recorded, min latency shouldn't be UINT32_MAX in output */
    if (out_snapshot->_latency_samples == 0)
    {
        out_snapshot->min_scheduling_latency = 0;
    }

    return SCHED_OK;
}

SchedStatus_t sched_stats_export(const sched_stats_data_t *snapshot, char *buffer, size_t max_len)
{
    if (snapshot == NULL || buffer == NULL || max_len == 0)
    {
        return SCHED_ERR_PARAM;
    }

    int ret = snprintf(buffer, max_len,
                       "{"
                       "\"scheduler_execution_count\":%lu,"
                       "\"task_execution_count\":%lu,"
                       "\"context_switch_count\":%lu,"
                       "\"scheduler_invocation_count\":%lu,"
                       "\"idle_time\":%lu,"
                       "\"busy_time\":%lu,"
                       "\"cpu_utilization_bp\":%lu,"
                       "\"avg_latency\":%lu,"
                       "\"max_latency\":%lu,"
                       "\"min_latency\":%lu"
                       "}",
                       (unsigned long)snapshot->scheduler_execution_count,
                       (unsigned long)snapshot->task_execution_count,
                       (unsigned long)snapshot->context_switch_count,
                       (unsigned long)snapshot->scheduler_invocation_count,
                       (unsigned long)snapshot->idle_time, (unsigned long)snapshot->busy_time,
                       (unsigned long)snapshot->cpu_utilization,
                       (unsigned long)snapshot->avg_scheduling_latency,
                       (unsigned long)snapshot->max_scheduling_latency,
                       (unsigned long)snapshot->min_scheduling_latency);

    if (ret < 0 || (size_t)ret >= max_len)
    {
        /* Output was truncated or error occurred */
        return SCHED_ERR_OVERFLOW;
    }

    return SCHED_OK;
}
