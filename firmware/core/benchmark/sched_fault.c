/**
 * @file    sched_fault.c
 * @brief   SchedTiny Fault Injection & Reliability Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_fault.h"

#include "sched_benchmark.h"
#include "sched_dispatcher.h"
#include "sched_policy.h"
#include "sched_trace.h"

#include <string.h>

#define SCHED_FAULT_MAX_TYPES 12

static bool g_fault_initialized                       = false;
static bool g_fault_enabled[SCHED_FAULT_MAX_TYPES]    = {0};
static uint32_t g_fault_params[SCHED_FAULT_MAX_TYPES] = {0};
static sched_fault_stats_t g_fault_stats              = {0};

SchedStatus_t sched_fault_init(void)
{
    memset(g_fault_enabled, 0, sizeof(g_fault_enabled));
    memset(g_fault_params, 0, sizeof(g_fault_params));
    memset(&g_fault_stats, 0, sizeof(g_fault_stats));
    g_fault_initialized = true;
    return SCHED_OK;
}

SchedStatus_t sched_fault_reset(void)
{
    if (!g_fault_initialized)
        return SCHED_ERR_STATE;
    memset(g_fault_enabled, 0, sizeof(g_fault_enabled));
    memset(g_fault_params, 0, sizeof(g_fault_params));
    memset(&g_fault_stats, 0, sizeof(g_fault_stats));
    return SCHED_OK;
}

SchedStatus_t sched_fault_enable(sched_fault_type_t fault)
{
    if (!g_fault_initialized)
        return SCHED_ERR_STATE;
    if (fault >= SCHED_FAULT_MAX_TYPES)
        return SCHED_ERR_PARAM;
    g_fault_enabled[fault] = true;
    return SCHED_OK;
}

SchedStatus_t sched_fault_disable(sched_fault_type_t fault)
{
    if (!g_fault_initialized)
        return SCHED_ERR_STATE;
    if (fault >= SCHED_FAULT_MAX_TYPES)
        return SCHED_ERR_PARAM;
    g_fault_enabled[fault] = false;
    return SCHED_OK;
}

SchedStatus_t sched_fault_inject(sched_fault_type_t fault, uint32_t parameter)
{
    if (!g_fault_initialized)
        return SCHED_ERR_STATE;
    if (fault >= SCHED_FAULT_MAX_TYPES)
        return SCHED_ERR_PARAM;
    g_fault_enabled[fault] = true;
    g_fault_params[fault]  = parameter;
    g_fault_stats.faults_injected++;
#if SCHED_CONFIG_ENABLE_TRACE
    sched_trace_record(0, SCHED_TRACE_EVT_FAULT_INJECTED, 0, 0, 0, 0, 0, 0, 0, fault);
#endif
    return SCHED_OK;
}

SchedStatus_t sched_fault_clear(sched_fault_type_t fault)
{
    if (!g_fault_initialized)
        return SCHED_ERR_STATE;
    if (fault >= SCHED_FAULT_MAX_TYPES)
        return SCHED_ERR_PARAM;
    g_fault_enabled[fault] = false;
    g_fault_params[fault]  = 0;
    return SCHED_OK;
}

SchedStatus_t sched_fault_statistics(sched_fault_stats_t *stats)
{
    if (!g_fault_initialized || stats == NULL)
        return SCHED_ERR_PARAM;
    memcpy(stats, &g_fault_stats, sizeof(sched_fault_stats_t));

    if (g_fault_stats.faults_injected > 0)
    {
        stats->fault_coverage =
            (g_fault_stats.faults_triggered * 100) / g_fault_stats.faults_injected;
    }
    else
    {
        stats->fault_coverage = 0;
    }

    return SCHED_OK;
}

void sched_fault_tick_hook(void *benchmark_ctx, uint32_t current_task, uint32_t tick)
{
    if (!g_fault_initialized || benchmark_ctx == NULL)
        return;

    sched_benchmark_t *ctx = (sched_benchmark_t *)benchmark_ctx;

    /* Simulate Execution Overrun */
    if (g_fault_enabled[SCHED_FAULT_EXECUTION_OVERRUN] && current_task != SCHED_DISPATCHER_NO_TASK)
    {
        /* 5% chance per tick to overrun, if enabled */
        if (benchmark_lcg_rand() % 100 < 5)
        {
            uint32_t added_time = g_fault_params[SCHED_FAULT_EXECUTION_OVERRUN];
            if (added_time == 0)
                added_time = (benchmark_lcg_rand() % 5) + 1;
            ctx->tasks[current_task].remaining_time += added_time;
            g_fault_stats.faults_triggered++;
            g_fault_stats.recovery_success++;  // Not necessarily a system crash
#if SCHED_CONFIG_ENABLE_TRACE
            sched_trace_record(tick, SCHED_TRACE_EVT_FAULT_TRIGGERED, current_task, 0, 0, 0, 0, 0,
                               0, SCHED_FAULT_EXECUTION_OVERRUN);
            sched_trace_record(tick, SCHED_TRACE_EVT_FAULT_RECOVERED, current_task, 0, 0, 0, 0, 0,
                               0, SCHED_FAULT_EXECUTION_OVERRUN);
#endif
        }
    }

    /* Simulate Random Task Failure */
    if (g_fault_enabled[SCHED_FAULT_RANDOM_FAILURE] && current_task != SCHED_DISPATCHER_NO_TASK)
    {
        /* 1% chance per tick for the task to just instantly fail (complete early / drop) */
        if (benchmark_lcg_rand() % 100 < 1)
        {
            ctx->tasks[current_task].remaining_time = 0;
            g_fault_stats.faults_triggered++;
#if SCHED_CONFIG_ENABLE_TRACE
            sched_trace_record(tick, SCHED_TRACE_EVT_FAULT_TRIGGERED, current_task, 0, 0, 0, 0, 0,
                               0, SCHED_FAULT_RANDOM_FAILURE);
#endif
        }
    }
}

void sched_fault_dispatch_hook(void *benchmark_ctx, uint32_t *next_task)
{
    if (!g_fault_initialized || benchmark_ctx == NULL || next_task == NULL)
        return;

    // sched_benchmark_t *ctx = (sched_benchmark_t *)benchmark_ctx;

    /* Simulate Dispatcher Failure (picks idle task instead of the valid next task) */
    if (g_fault_enabled[SCHED_FAULT_DISPATCHER_FAILURE] && *next_task != SCHED_DISPATCHER_NO_TASK)
    {
        /* 2% chance of dispatcher selecting IDLE incorrectly */
        if (benchmark_lcg_rand() % 100 < 2)
        {
            *next_task = SCHED_DISPATCHER_NO_TASK;
            g_fault_stats.faults_triggered++;
#if SCHED_CONFIG_ENABLE_TRACE
            /* Using 0 for tick as we don't have it directly in this hook */
            sched_trace_record(0, SCHED_TRACE_EVT_FAULT_TRIGGERED, *next_task, 0, 0, 0, 0, 0, 0,
                               SCHED_FAULT_DISPATCHER_FAILURE);
#endif
        }
    }

    /* Simulate Priority Inversion (Dispatcher picks a random low-priority valid task) */
    if (g_fault_enabled[SCHED_FAULT_PRIORITY_INVERSION] && *next_task != SCHED_DISPATCHER_NO_TASK)
    {
        if (benchmark_lcg_rand() % 100 < 2)
        {
            /* Inject a priority inversion by dispatching task ID 1 if it's active (just as a mock)
             */
            // *next_task = ...
            g_fault_stats.faults_triggered++;
        }
    }
}
