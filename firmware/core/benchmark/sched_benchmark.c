/**
 * @file    sched_benchmark.c
 * @brief   SchedTiny Benchmark Harness & Experimental Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_benchmark.h"

#include "sched_dispatcher.h"
#include "sched_edf.h"
#include "sched_policy.h"
#include "sched_rms.h"
#include "sched_stats.h"

#include <stdio.h>
#include <string.h>

/* Simple LCG for deterministic workloads */
static uint32_t benchmark_lcg_seed = 1;

static uint32_t lcg_rand(void)
{
    benchmark_lcg_seed = benchmark_lcg_seed * 1103515245 + 12345;
    return (uint32_t)((benchmark_lcg_seed / 65536) % 32768);
}

static void lcg_srand(uint32_t seed)
{
    benchmark_lcg_seed = seed;
}

SchedStatus_t sched_benchmark_init(sched_benchmark_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_benchmark_t));
    ctx->initialized = true;

    return SCHED_OK;
}

SchedStatus_t sched_benchmark_reset(sched_benchmark_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    ctx->task_count = 0;
    memset(ctx->results, 0, sizeof(ctx->results));
    memset(ctx->has_results, 0, sizeof(ctx->has_results));

    return SCHED_OK;
}

SchedStatus_t sched_benchmark_load_workload(sched_benchmark_t *ctx,
                                            sched_benchmark_profile_t profile,
                                            uint32_t seed)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    uint32_t count = 10;
    if (profile == SCHED_BENCHMARK_PROFILE_MEDIUM)
        count = 25;
    if (profile == SCHED_BENCHMARK_PROFILE_LARGE)
        count = 50;
    if (profile == SCHED_BENCHMARK_PROFILE_STRESS)
        count = 100;

    if (count > SCHED_BENCHMARK_MAX_TASKS)
    {
        return SCHED_ERR_OVERFLOW;
    }

    ctx->task_count = count;
    lcg_srand(seed);

    for (uint32_t i = 0; i < count; i++)
    {
        sched_benchmark_task_t *t = &ctx->tasks[i];
        memset(t, 0, sizeof(sched_benchmark_task_t));

        t->task_id = i;
        /* Distribute workload types pseudo-randomly */
        uint32_t r_type  = lcg_rand() % 4;
        t->workload_type = (sched_benchmark_workload_t)r_type;

        t->release_time = lcg_rand() % 50;
        t->priority     = (lcg_rand() % 8);  // Assuming 8 priority levels

        if (t->workload_type == SCHED_WORKLOAD_PERIODIC)
        {
            t->execution_time = (lcg_rand() % 10) + 1;
            t->period         = (lcg_rand() % 50) + 20;
            t->deadline       = t->period;
        }
        else if (t->workload_type == SCHED_WORKLOAD_CPU_BOUND)
        {
            t->execution_time = (lcg_rand() % 30) + 20;
            t->period         = (lcg_rand() % 100) + 60;
            t->deadline       = t->period;
        }
        else if (t->workload_type == SCHED_WORKLOAD_DEADLINE_SENSITIVE)
        {
            t->execution_time = (lcg_rand() % 5) + 1;
            t->period         = (lcg_rand() % 50) + 20;
            t->deadline       = t->execution_time + (lcg_rand() % 5);  // Tight deadline
        }
        else                                                           /* APERIODIC */
        {
            t->execution_time = (lcg_rand() % 15) + 1;
            t->period         = 0;  // Does not repeat
            t->deadline       = 1000;
        }
    }

    return SCHED_OK;
}

static void run_policy_benchmark(sched_benchmark_t *ctx,
                                 sched_benchmark_policy_t policy,
                                 uint32_t sim_ticks)
{
    sched_policy_t hpf_ctx;
    sched_edf_t edf_ctx;
    sched_rms_t rms_ctx;
    sched_dispatcher_t disp_ctx;
    sched_stats_t stats_ctx;

    /* Initialize tracking */
    sched_stats_init(&stats_ctx);
    sched_dispatcher_init(&disp_ctx);
    sched_stats_start(&stats_ctx, 0);

    /* Initialize policies */
    if (policy == SCHED_BENCHMARK_POLICY_HPF)
    {
        sched_policy_init(&hpf_ctx);
    }
    else if (policy == SCHED_BENCHMARK_POLICY_EDF)
    {
        sched_edf_init(&edf_ctx);
        for (uint32_t i = 0; i < ctx->task_count; i++)
        {
            /* Prevent passing 0 relative deadline to avoid overflow weirdness */
            uint32_t dl = ctx->tasks[i].deadline;
            if (dl == 0)
                dl = 1;
            sched_edf_assign_deadline(&edf_ctx, i, dl);
        }
    }
    else if (policy == SCHED_BENCHMARK_POLICY_RMS)
    {
        sched_rms_init(&rms_ctx);
        for (uint32_t i = 0; i < ctx->task_count; i++)
        {
            uint32_t p = ctx->tasks[i].period;
            if (p == 0)
                p = 1000; /* Fallback for aperiodic */
            sched_rms_assign_period(&rms_ctx, i, p);
        }
    }

    /* Reset task runtime states */
    for (uint32_t i = 0; i < ctx->task_count; i++)
    {
        ctx->tasks[i].remaining_time    = 0;
        ctx->tasks[i].completion_count  = 0;
        ctx->tasks[i].deadline_misses   = 0;
        ctx->tasks[i].response_time_sum = 0;
        ctx->tasks[i].response_time_max = 0;
        ctx->tasks[i].waiting_time_sum  = 0;
        ctx->tasks[i].waiting_time_max  = 0;
        ctx->tasks[i].last_release      = 0;
    }

    for (uint32_t t = 0; t < sim_ticks; t++)
    {
        bool invoke_scheduler = false;

        /* Release tasks */
        for (uint32_t i = 0; i < ctx->task_count; i++)
        {
            sched_benchmark_task_t *task = &ctx->tasks[i];

            bool release = false;
            if (t == task->release_time)
            {
                release = true;
            }
            else if (task->period > 0 && t > task->release_time)
            {
                if ((t - task->release_time) % task->period == 0)
                {
                    release = true;
                }
            }

            if (release)
            {
                /* Overrun check: if it was already running, it missed deadline */
                if (task->remaining_time > 0)
                {
                    task->deadline_misses++;
                }

                task->remaining_time = task->execution_time;
                task->last_release   = t;
                invoke_scheduler     = true;

                /* Add to ready queue */
                if (policy == SCHED_BENCHMARK_POLICY_HPF)
                {
                    sched_policy_add_task(&hpf_ctx, i, task->priority);
                }
                else if (policy == SCHED_BENCHMARK_POLICY_EDF)
                {
                    sched_edf_add_task(&edf_ctx, i, t);
                }
                else if (policy == SCHED_BENCHMARK_POLICY_RMS)
                {
                    sched_rms_add_task(&rms_ctx, i);
                }
            }
        }

        /* Check deadlines */
        for (uint32_t i = 0; i < ctx->task_count; i++)
        {
            sched_benchmark_task_t *task = &ctx->tasks[i];
            if (task->remaining_time > 0 && t > task->last_release)
            {
                if ((t - task->last_release) == task->deadline)
                {
                    task->deadline_misses++;
                }
            }
        }

        /* Pick next task if scheduler invoked */
        uint32_t next_task = SCHED_DISPATCHER_NO_TASK;
        if (policy == SCHED_BENCHMARK_POLICY_HPF)
        {
            sched_policy_peek_next(&hpf_ctx, &next_task);
        }
        else if (policy == SCHED_BENCHMARK_POLICY_EDF)
        {
            sched_edf_peek_next(&edf_ctx, &next_task);
        }
        else if (policy == SCHED_BENCHMARK_POLICY_RMS)
        {
            sched_rms_peek_next(&rms_ctx, &next_task);
        }

        uint32_t old_task = SCHED_DISPATCHER_NO_TASK;
        sched_dispatcher_current_task(&disp_ctx, &old_task);

        if (next_task != old_task || invoke_scheduler)
        {
            sched_stats_record_invocation(&stats_ctx);
            sched_dispatcher_dispatch(&disp_ctx, next_task);
            uint32_t new_switches = 0;
            sched_dispatcher_context_switch_count(&disp_ctx, &new_switches);
            /* This is a simple approximation; sched_stats can track it internally or we sync it */
            stats_ctx.data.context_switch_count = new_switches;
        }

        uint32_t current_task = SCHED_DISPATCHER_NO_TASK;
        sched_dispatcher_current_task(&disp_ctx, &current_task);

        /* Execute one tick */
        if (current_task != SCHED_DISPATCHER_NO_TASK)
        {
            sched_stats_record_busy_time(&stats_ctx, 1);
            sched_benchmark_task_t *task = &ctx->tasks[current_task];

            if (task->remaining_time > 0)
            {
                task->remaining_time--;
                if (task->remaining_time == 0)
                {
                    /* Task completed */
                    task->completion_count++;
                    uint32_t response_time = (t - task->last_release) + 1;
                    task->response_time_sum += response_time;
                    if (response_time > task->response_time_max)
                        task->response_time_max = response_time;

                    uint32_t wait_time = response_time - task->execution_time;
                    task->waiting_time_sum += wait_time;
                    if (wait_time > task->waiting_time_max)
                        task->waiting_time_max = wait_time;

                    /* Remove from queue */
                    if (policy == SCHED_BENCHMARK_POLICY_HPF)
                    {
                        sched_policy_remove_task(&hpf_ctx, current_task);
                    }
                    else if (policy == SCHED_BENCHMARK_POLICY_EDF)
                    {
                        sched_edf_remove_task(&edf_ctx, current_task);
                    }
                    else if (policy == SCHED_BENCHMARK_POLICY_RMS)
                    {
                        sched_rms_remove_task(&rms_ctx, current_task);
                    }

                    /* Dispatch idle to force picking a new task next tick */
                    sched_dispatcher_idle(&disp_ctx);
                }
            }
        }
        else
        {
            sched_stats_record_idle_time(&stats_ctx, 1);
        }

        sched_dispatcher_tick(&disp_ctx);
    }

    sched_stats_stop(&stats_ctx, sim_ticks);

    /* Aggregate results */
    sched_benchmark_results_t *res = &ctx->results[policy];
    memset(res, 0, sizeof(sched_benchmark_results_t));
    res->policy      = policy;
    res->total_tasks = ctx->task_count;

    sched_stats_data_t snap;
    sched_stats_snapshot(&stats_ctx, &snap);

    res->cpu_utilization_bp         = snap.cpu_utilization;
    res->context_switches           = snap.context_switch_count;
    res->idle_time                  = snap.idle_time;
    res->busy_time                  = snap.busy_time;
    res->scheduler_invocation_count = snap.scheduler_invocation_count;

    for (uint32_t i = 0; i < ctx->task_count; i++)
    {
        sched_benchmark_task_t *task = &ctx->tasks[i];
        res->task_completion_count += task->completion_count;
        res->deadline_misses += task->deadline_misses;

        if (task->response_time_max > res->max_response_time)
            res->max_response_time = task->response_time_max;

        if (task->waiting_time_max > res->max_waiting_time)
            res->max_waiting_time = task->waiting_time_max;

        res->avg_response_time += task->response_time_sum;
        res->avg_waiting_time += task->waiting_time_sum;
    }

    if (res->task_completion_count > 0)
    {
        res->avg_response_time /= res->task_completion_count;
        res->avg_waiting_time /= res->task_completion_count;

        /* completions per 10000 ticks */
        res->throughput = (res->task_completion_count * 10000) / sim_ticks;
    }

    ctx->has_results[policy] = true;
}

SchedStatus_t sched_benchmark_run(sched_benchmark_t *ctx,
                                  sched_benchmark_policy_t policy,
                                  uint32_t sim_ticks)
{
    if (ctx == NULL || sim_ticks == 0)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->task_count == 0)
    {
        return SCHED_ERR_STATE;
    }

    if (policy > SCHED_BENCHMARK_POLICY_RMS)
    {
        return SCHED_ERR_PARAM;
    }

    run_policy_benchmark(ctx, policy, sim_ticks);

    return SCHED_OK;
}

SchedStatus_t sched_benchmark_run_all(sched_benchmark_t *ctx, uint32_t sim_ticks)
{
    if (ctx == NULL || sim_ticks == 0)
    {
        return SCHED_ERR_PARAM;
    }

    SchedStatus_t st = sched_benchmark_run(ctx, SCHED_BENCHMARK_POLICY_HPF, sim_ticks);
    if (st != SCHED_OK)
        return st;

    st = sched_benchmark_run(ctx, SCHED_BENCHMARK_POLICY_EDF, sim_ticks);
    if (st != SCHED_OK)
        return st;

    st = sched_benchmark_run(ctx, SCHED_BENCHMARK_POLICY_RMS, sim_ticks);
    if (st != SCHED_OK)
        return st;

    return SCHED_OK;
}

SchedStatus_t sched_benchmark_stop(sched_benchmark_t *ctx)
{
    (void)ctx;
    /* In this simulation loop design, running is blocking/synchronous.
     * stop() is a no-op, but provided for API completeness. */
    return SCHED_OK;
}

SchedStatus_t sched_benchmark_get_results(const sched_benchmark_t *ctx,
                                          sched_benchmark_policy_t policy,
                                          sched_benchmark_results_t *out_results)
{
    if (ctx == NULL || out_results == NULL || policy > SCHED_BENCHMARK_POLICY_RMS)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->has_results[policy])
    {
        return SCHED_ERR_NOT_FOUND;
    }

    memcpy(out_results, &ctx->results[policy], sizeof(sched_benchmark_results_t));
    return SCHED_OK;
}

static const char *policy_to_string(sched_benchmark_policy_t p)
{
    switch (p)
    {
        case SCHED_BENCHMARK_POLICY_HPF:
            return "HPF";
        case SCHED_BENCHMARK_POLICY_EDF:
            return "EDF";
        case SCHED_BENCHMARK_POLICY_RMS:
            return "RMS";
        default:
            return "UNKNOWN";
    }
}

SchedStatus_t sched_benchmark_export_csv(const sched_benchmark_t *ctx, char *buffer, size_t max_len)
{
    if (ctx == NULL || buffer == NULL || max_len == 0)
    {
        return SCHED_ERR_PARAM;
    }

    int offset = snprintf(
        buffer, max_len, "Algorithm,Tasks,CPUUtilization,Latency,DeadlineMisses,ContextSwitches\n");
    if (offset < 0 || (size_t)offset >= max_len)
        return SCHED_ERR_OVERFLOW;

    for (int i = 0; i < 3; i++)
    {
        if (ctx->has_results[i])
        {
            const sched_benchmark_results_t *r = &ctx->results[i];
            /* Format CPU util as float percentage (e.g., 8234 bp -> 82.34) */
            float cpu_util = (float)r->cpu_utilization_bp / 100.0f;
            int written    = snprintf(
                buffer + offset, max_len - offset, "%s,%lu,%.2f,%lu,%lu,%lu\n",
                policy_to_string((sched_benchmark_policy_t)i), (unsigned long)r->total_tasks,
                cpu_util, (unsigned long)r->avg_scheduling_latency,
                (unsigned long)r->deadline_misses, (unsigned long)r->context_switches);
            if (written < 0 || (size_t)written >= max_len - offset)
                return SCHED_ERR_OVERFLOW;
            offset += written;
        }
    }

    return SCHED_OK;
}

SchedStatus_t sched_benchmark_export_json(const sched_benchmark_t *ctx,
                                          char *buffer,
                                          size_t max_len)
{
    if (ctx == NULL || buffer == NULL || max_len == 0)
    {
        return SCHED_ERR_PARAM;
    }

    int offset = snprintf(buffer, max_len, "[\n");
    if (offset < 0 || (size_t)offset >= max_len)
        return SCHED_ERR_OVERFLOW;

    bool first = true;
    for (int i = 0; i < 3; i++)
    {
        if (ctx->has_results[i])
        {
            if (!first)
            {
                int written = snprintf(buffer + offset, max_len - offset, ",\n");
                if (written < 0 || (size_t)written >= max_len - offset)
                    return SCHED_ERR_OVERFLOW;
                offset += written;
            }
            first = false;

            const sched_benchmark_results_t *r = &ctx->results[i];
            float cpu_util                     = (float)r->cpu_utilization_bp / 100.0f;

            int written = snprintf(
                buffer + offset, max_len - offset,
                "  {\n"
                "    \"Algorithm\": \"%s\",\n"
                "    \"Tasks\": %lu,\n"
                "    \"CPUUtilization\": %.2f,\n"
                "    \"Latency\": %lu,\n"
                "    \"DeadlineMisses\": %lu,\n"
                "    \"ContextSwitches\": %lu\n"
                "  }",
                policy_to_string((sched_benchmark_policy_t)i), (unsigned long)r->total_tasks,
                cpu_util, (unsigned long)r->avg_scheduling_latency,
                (unsigned long)r->deadline_misses, (unsigned long)r->context_switches);
            if (written < 0 || (size_t)written >= max_len - offset)
                return SCHED_ERR_OVERFLOW;
            offset += written;
        }
    }

    int written = snprintf(buffer + offset, max_len - offset, "\n]\n");
    if (written < 0 || (size_t)written >= max_len - offset)
        return SCHED_ERR_OVERFLOW;

    return SCHED_OK;
}
