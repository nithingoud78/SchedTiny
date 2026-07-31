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

#include "sched_adaptive.h"
#include "sched_dispatcher.h"
#include "sched_edf.h"
#include "sched_fault.h"
#include "sched_mc.h"
#include "sched_policy.h"
#include "sched_power.h"
#include "sched_rms.h"
#include "sched_stats.h"

#include <stdio.h>
#include <string.h>

#ifdef STM32
#include "platform_timer.h"
#endif

/* Simple LCG for deterministic workloads */
static uint32_t benchmark_lcg_seed = 1;

uint32_t benchmark_lcg_rand(void)
{
    benchmark_lcg_seed = benchmark_lcg_seed * 1103515245 + 12345;
    return (uint32_t)((benchmark_lcg_seed / 65536) % 32768);
}

void benchmark_lcg_srand(uint32_t seed)
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

    /* Initialize fault framework */
    sched_fault_init();

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
    benchmark_lcg_srand(seed);

    for (uint32_t i = 0; i < count; i++)
    {
        sched_benchmark_task_t *t = &ctx->tasks[i];
        memset(t, 0, sizeof(sched_benchmark_task_t));

        t->task_id = i;
        /* Distribute workload types pseudo-randomly */
        uint32_t r_type  = benchmark_lcg_rand() % 4;
        t->workload_type = (sched_benchmark_workload_t)r_type;

        t->release_time = benchmark_lcg_rand() % 50;
        t->priority     = (benchmark_lcg_rand() % 8);  // Assuming 8 priority levels

        if (t->workload_type == SCHED_WORKLOAD_PERIODIC)
        {
            t->execution_time = (benchmark_lcg_rand() % 10) + 1;
            t->period         = (benchmark_lcg_rand() % 50) + 20;
            t->deadline       = t->period;
        }
        else if (t->workload_type == SCHED_WORKLOAD_CPU_BOUND)
        {
            t->execution_time = (benchmark_lcg_rand() % 30) + 20;
            t->period         = (benchmark_lcg_rand() % 100) + 60;
            t->deadline       = t->period;
        }
        else if (t->workload_type == SCHED_WORKLOAD_DEADLINE_SENSITIVE)
        {
            t->execution_time = (benchmark_lcg_rand() % 5) + 1;
            t->period         = (benchmark_lcg_rand() % 50) + 20;
            t->deadline       = t->execution_time + (benchmark_lcg_rand() % 5);  // Tight deadline
        }
        else                                                                     /* APERIODIC */
        {
            t->execution_time = (benchmark_lcg_rand() % 15) + 1;
            t->period         = 0;  // Does not repeat
            t->deadline       = 1000;
        }

        /* Mixed Criticality Initialization */
        /* ~30% of tasks are HI criticality */
        t->criticality = (benchmark_lcg_rand() % 100 < 30) ? 1 : 0;

        /* Execution time is treated as the TRUE execution time in simulation.
         * For MC, let's say the LO WCET is slightly less than execution time if
         * it's going to overrun, or more than execution time if it's well-behaved.
         * To trigger mode switches, we want some HI tasks to overrun their LO WCET.
         * Let's set lo_wcet to execution_time - (execution_time/4) so it always
         * overruns, but ONLY if it's HI criticality. Wait, if it always overruns,
         * we always switch. Let's make it overrun 50% of the time by randomly
         * setting lo_wcet.
         */
        if (t->criticality == 1) /* HI */
        {
            if (benchmark_lcg_rand() % 2 == 0)
            {
                /* Overrun: execution_time > lo_wcet */
                t->lo_wcet = (t->execution_time > 2) ? t->execution_time - 1 : 1;
            }
            else
            {
                /* No overrun */
                t->lo_wcet = t->execution_time + 1;
            }
            t->hi_wcet = t->execution_time + 5; /* HI WCET is an upper bound */
        }
        else                                    /* LO */
        {
            t->lo_wcet = t->execution_time + 2;
            t->hi_wcet = t->lo_wcet;
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
    sched_mc_t mc_ctx;
    sched_adaptive_t adaptive_ctx;
    sched_dispatcher_t disp_ctx;
    sched_stats_t stats_ctx;

    /* The active sub-policy for adaptive mode */
    sched_benchmark_policy_t active_sub_policy = SCHED_BENCHMARK_POLICY_HPF;

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
    else if (policy == SCHED_BENCHMARK_POLICY_MC)
    {
        sched_mc_init(&mc_ctx);
        for (uint32_t i = 0; i < ctx->task_count; i++)
        {
            sched_benchmark_task_t *t = &ctx->tasks[i];
            sched_mc_register(&mc_ctx, i, t->criticality == 1 ? SCHED_MC_CRIT_HI : SCHED_MC_CRIT_LO,
                              t->lo_wcet, t->hi_wcet, t->priority, t->deadline);
        }
    }
    else if (policy == SCHED_BENCHMARK_POLICY_ADAPTIVE)
    {
        /* Adaptive mode: initialise ALL sub-schedulers and the adaptive engine */
        sched_adaptive_init(&adaptive_ctx);
        sched_policy_init(&hpf_ctx);
        sched_edf_init(&edf_ctx);
        sched_rms_init(&rms_ctx);
        sched_mc_init(&mc_ctx);

        for (uint32_t i = 0; i < ctx->task_count; i++)
        {
            uint32_t dl = ctx->tasks[i].deadline;
            if (dl == 0)
                dl = 1;
            sched_edf_assign_deadline(&edf_ctx, i, dl);

            uint32_t p = ctx->tasks[i].period;
            if (p == 0)
                p = 1000;
            sched_rms_assign_period(&rms_ctx, i, p);

            sched_benchmark_task_t *t = &ctx->tasks[i];
            sched_mc_register(&mc_ctx, i, t->criticality == 1 ? SCHED_MC_CRIT_HI : SCHED_MC_CRIT_LO,
                              t->lo_wcet, t->hi_wcet, t->priority, t->deadline);
        }

        active_sub_policy = sched_adaptive_get_active_policy(&adaptive_ctx);
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
                else if (policy == SCHED_BENCHMARK_POLICY_MC)
                {
                    sched_mc_reset_budget(&mc_ctx, i);
                }
                else if (policy == SCHED_BENCHMARK_POLICY_ADAPTIVE)
                {
                    /* Add to ALL sub-schedulers so we can switch freely */
                    sched_policy_add_task(&hpf_ctx, i, task->priority);
                    sched_edf_add_task(&edf_ctx, i, t);
                    sched_rms_add_task(&rms_ctx, i);
                    sched_mc_reset_budget(&mc_ctx, i);
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
        else if (policy == SCHED_BENCHMARK_POLICY_MC)
        {
            sched_mc_dispatch(&mc_ctx, &next_task);
        }
        else if (policy == SCHED_BENCHMARK_POLICY_ADAPTIVE)
        {
            /* Periodically re-evaluate policy selection */
            if (sched_adaptive_tick(&adaptive_ctx))
            {
                sched_adaptive_select_policy(&adaptive_ctx, ctx, active_sub_policy);
                active_sub_policy = sched_adaptive_get_active_policy(&adaptive_ctx);
            }

            /* Dispatch using the active sub-policy */
            if (active_sub_policy == SCHED_BENCHMARK_POLICY_HPF)
            {
                sched_policy_peek_next(&hpf_ctx, &next_task);
            }
            else if (active_sub_policy == SCHED_BENCHMARK_POLICY_EDF)
            {
                sched_edf_peek_next(&edf_ctx, &next_task);
            }
            else if (active_sub_policy == SCHED_BENCHMARK_POLICY_RMS)
            {
                sched_rms_peek_next(&rms_ctx, &next_task);
            }
            else
            {
                sched_mc_dispatch(&mc_ctx, &next_task);
            }
        }

        uint32_t old_task = SCHED_DISPATCHER_NO_TASK;
        sched_dispatcher_current_task(&disp_ctx, &old_task);

        if (next_task != old_task || invoke_scheduler)
        {
#ifdef STM32
            uint32_t start_us = platform_timestamp_us();
#endif
            sched_stats_record_invocation(&stats_ctx);
            sched_dispatcher_dispatch(&disp_ctx, next_task);
#ifdef STM32
            uint32_t end_us = platform_timestamp_us();
            /* Record actual measured hardware latency */
            sched_stats_record_latency(&stats_ctx, end_us - start_us);
#endif
            uint32_t new_switches = 0;
            sched_dispatcher_context_switch_count(&disp_ctx, &new_switches);
            /* This is a simple approximation; sched_stats can track it internally or
             * we sync it */
            stats_ctx.data.context_switch_count = new_switches;
        }

        uint32_t current_task = SCHED_DISPATCHER_NO_TASK;
        sched_dispatcher_current_task(&disp_ctx, &current_task);

        /* Hook for per-tick faults (e.g. execution overrun, random drop) */
        sched_fault_tick_hook(ctx, current_task, t);

        /* Execute one tick */
        if (next_task != SCHED_DISPATCHER_NO_TASK)
        {
            sched_fault_dispatch_hook(ctx, &next_task);
#ifdef STM32
            uint32_t start_us = platform_timestamp_us();
#endif
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
                    else if (policy == SCHED_BENCHMARK_POLICY_MC)
                    {
                        for (int k = 0; k < SCHED_MC_CAPACITY; k++)
                        {
                            if (mc_ctx.registry[k].task_id == current_task)
                            {
                                mc_ctx.registry[k].dropped = true;
                                break;
                            }
                        }
                    }
                    else if (policy == SCHED_BENCHMARK_POLICY_ADAPTIVE)
                    {
                        /* Remove from ALL sub-schedulers */
                        sched_policy_remove_task(&hpf_ctx, current_task);
                        sched_edf_remove_task(&edf_ctx, current_task);
                        sched_rms_remove_task(&rms_ctx, current_task);
                        for (int k = 0; k < SCHED_MC_CAPACITY; k++)
                        {
                            if (mc_ctx.registry[k].task_id == current_task)
                            {
                                mc_ctx.registry[k].dropped = true;
                                break;
                            }
                        }
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

        if (policy == SCHED_BENCHMARK_POLICY_MC)
        {
            sched_mc_tick(&mc_ctx, current_task, t);
        }
        else if (policy == SCHED_BENCHMARK_POLICY_ADAPTIVE)
        {
            sched_mc_tick(&mc_ctx, current_task, t);
        }
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

    sched_power_metrics_t pwr;
    if (sched_power_compute(SCHED_POWER_MODEL_IDLE_ACTIVE, &snap, res->task_completion_count,
                            &pwr) == SCHED_OK)
    {
        res->estimated_energy_uj = pwr.estimated_energy_uj;
        res->estimated_power_uw  = pwr.estimated_power_uw;
        res->energy_per_task_uj  = pwr.energy_per_task_uj;
        res->energy_per_cs_uj    = pwr.energy_per_cs_uj;
    }

    /* Aggregate Mixed Criticality Metrics */
    if (policy == SCHED_BENCHMARK_POLICY_MC)
    {
        res->mode_switches      = mc_ctx.mode_switch_count;
        res->hi_mode_entries    = mc_ctx.hi_mode_entries;
        res->max_hi_duration    = mc_ctx.max_hi_duration;
        res->dropped_lo_tasks   = mc_ctx.dropped_lo_tasks;
        res->recovered_lo_tasks = mc_ctx.recovered_lo_tasks;
    }

    /* Aggregate Fault Injection Metrics */
    sched_fault_stats_t fstats;
    if (sched_fault_statistics(&fstats) == SCHED_OK)
    {
        res->faults_injected              = fstats.faults_injected;
        res->faults_triggered             = fstats.faults_triggered;
        res->recovery_success             = fstats.recovery_success;
        res->recovery_time                = fstats.recovery_time;
        res->missed_deadlines_after_fault = fstats.missed_deadlines_after_fault;
        res->task_recovery_count          = fstats.task_recovery_count;
        res->task_restart_count           = fstats.task_restart_count;
        res->system_availability_bp       = fstats.system_availability;
        res->fault_coverage_bp            = fstats.fault_coverage;
    }

    ctx->has_results[policy] = true;

    /* Aggregate Adaptive Metrics */
    if (policy == SCHED_BENCHMARK_POLICY_ADAPTIVE)
    {
        sched_adaptive_stats_t astats;
        if (sched_adaptive_get_stats(&adaptive_ctx, &astats) == SCHED_OK)
        {
            res->adaptive_decisions      = astats.total_decisions;
            res->adaptive_switches       = astats.total_switches;
            res->adaptive_overhead_ticks = astats.overhead_ticks;
            if (astats.total_decisions > 0)
            {
                res->adaptive_decision_latency = astats.decision_latency / astats.total_decisions;
            }
            res->adaptive_accuracy_bp = astats.accuracy_bp;
        }
    }
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

    if (policy > SCHED_BENCHMARK_POLICY_ADAPTIVE)
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

    st = sched_benchmark_run(ctx, SCHED_BENCHMARK_POLICY_MC, sim_ticks);
    if (st != SCHED_OK)
        return st;

    st = sched_benchmark_run(ctx, SCHED_BENCHMARK_POLICY_ADAPTIVE, sim_ticks);
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
    if (ctx == NULL || out_results == NULL || policy > SCHED_BENCHMARK_POLICY_ADAPTIVE)
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
        case SCHED_BENCHMARK_POLICY_MC:
            return "MC";
        case SCHED_BENCHMARK_POLICY_ADAPTIVE:
            return "ADAPTIVE";
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

    const char *header =
        "Algorithm,Tasks,CPUUtilization,Latency,DeadlineMisses,ContextSwitches,AvgResponseTime,"
        "AvgWaitingTime,Throughput,IdleTime,BusyTime,EstimatedEnergy,EstimatedPower,EnergyPerTask,"
        "EnergyPerCS,ModeSwitches,HiModeEntries,MaxHiDuration,DroppedLoTasks,RecoveredLoTasks,"
        "FaultsInjected,FaultsTriggered,RecoverySuccess,RecoveryTime,MissedDeadlinesAfterFault,"
        "TaskRecoveryCount,TaskRestartCount,SystemAvailability,FaultCoverage,"
        "AdaptiveDecisions,AdaptiveSwitches,AdaptiveDecisionLatency,AdaptiveOverheadTicks,"
        "AdaptiveAccuracy\n";
    int offset = snprintf(buffer, max_len, "%s", header);
    if (offset < 0 || (size_t)offset >= max_len)
        return SCHED_ERR_OVERFLOW;

    for (int i = 0; i < 5; i++)
    {
        if (ctx->has_results[i])
        {
            const sched_benchmark_results_t *r = &ctx->results[i];
            /* Format CPU util as float percentage (e.g., 8234 bp -> 82.34) */
            float cpu_util = (float)r->cpu_utilization_bp / 100.0f;
            int written    = snprintf(
                buffer + offset, max_len - offset,
                "%s,%lu,%.2f,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%lu,%"
                   "lu,%lu,%lu,%lu,%lu,%lu,%lu,%.2f,%.2f,%lu,%lu,%lu,%lu,%.2f\n",
                policy_to_string((sched_benchmark_policy_t)i), (unsigned long)r->total_tasks,
                cpu_util, (unsigned long)r->avg_scheduling_latency,
                (unsigned long)r->deadline_misses, (unsigned long)r->context_switches,
                (unsigned long)r->avg_response_time, (unsigned long)r->avg_waiting_time,
                (unsigned long)r->throughput, (unsigned long)r->idle_time,
                (unsigned long)r->busy_time, (unsigned long)r->estimated_energy_uj,
                (unsigned long)r->estimated_power_uw, (unsigned long)r->energy_per_task_uj,
                (unsigned long)r->energy_per_cs_uj, (unsigned long)r->mode_switches,
                (unsigned long)r->hi_mode_entries, (unsigned long)r->max_hi_duration,
                (unsigned long)r->dropped_lo_tasks, (unsigned long)r->recovered_lo_tasks,
                (unsigned long)r->faults_injected, (unsigned long)r->faults_triggered,
                (unsigned long)r->recovery_success, (unsigned long)r->recovery_time,
                (unsigned long)r->missed_deadlines_after_fault,
                (unsigned long)r->task_recovery_count, (unsigned long)r->task_restart_count,
                (float)r->system_availability_bp / 100.0f, (float)r->fault_coverage_bp / 100.0f,
                (unsigned long)r->adaptive_decisions, (unsigned long)r->adaptive_switches,
                (unsigned long)r->adaptive_decision_latency,
                (unsigned long)r->adaptive_overhead_ticks, (float)r->adaptive_accuracy_bp / 100.0f);
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
    for (int i = 0; i < 5; i++)
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
            bool is_last                       = (i == 3);

            int written = snprintf(
                buffer + offset, max_len - offset,
                "  {\n"
                "    \"Algorithm\": \"%s\",\n"
                "    \"Tasks\": %lu,\n"
                "    \"CPUUtilization\": %.2f,\n"
                "    \"Latency\": %lu,\n"
                "    \"DeadlineMisses\": %lu,\n"
                "    \"ContextSwitches\": %lu,\n"
                "    \"AvgResponseTime\": %lu,\n"
                "    \"AvgWaitingTime\": %lu,\n"
                "    \"Throughput\": %lu,\n"
                "    \"IdleTime\": %lu,\n"
                "    \"BusyTime\": %lu,\n"
                "    \"Energy_uJ\": %lu,\n"
                "    \"Power_uW\": %lu,\n"
                "    \"EnergyPerTask_uJ\": %lu,\n"
                "    \"EnergyPerCS_uJ\": %lu,\n"
                "    \"ModeSwitches\": %lu,\n"
                "    \"HIModeEntries\": %lu,\n"
                "    \"MaxHIDuration\": %lu,\n"
                "    \"DroppedLoTasks\":%lu,\n"
                "    \"RecoveredLoTasks\":%lu,\n"
                "    \"FaultsInjected\":%lu,\n"
                "    \"FaultsTriggered\":%lu,\n"
                "    \"RecoverySuccess\":%lu,\n"
                "    \"RecoveryTime\":%lu,\n"
                "    \"MissedDeadlinesAfterFault\":%lu,\n"
                "    \"TaskRecoveryCount\":%lu,\n"
                "    \"TaskRestartCount\":%lu,\n"
                "    \"SystemAvailability\":%.2f,\n"
                "    \"FaultCoverage\":%.2f,\n"
                "    \"AdaptiveDecisions\":%lu,\n"
                "    \"AdaptiveSwitches\":%lu,\n"
                "    \"AdaptiveDecisionLatency\":%lu,\n"
                "    \"AdaptiveOverheadTicks\":%lu,\n"
                "    \"AdaptiveAccuracy\":%.2f\n"
                "  }%s",
                policy_to_string((sched_benchmark_policy_t)i), (unsigned long)r->total_tasks,
                cpu_util, (unsigned long)r->avg_scheduling_latency,
                (unsigned long)r->deadline_misses, (unsigned long)r->context_switches,
                (unsigned long)r->avg_response_time, (unsigned long)r->avg_waiting_time,
                (unsigned long)r->throughput, (unsigned long)r->idle_time,
                (unsigned long)r->busy_time, (unsigned long)r->estimated_energy_uj,
                (unsigned long)r->estimated_power_uw, (unsigned long)r->energy_per_task_uj,
                (unsigned long)r->energy_per_cs_uj, (unsigned long)r->mode_switches,
                (unsigned long)r->hi_mode_entries, (unsigned long)r->max_hi_duration,
                (unsigned long)r->dropped_lo_tasks, (unsigned long)r->recovered_lo_tasks,
                (unsigned long)r->faults_injected, (unsigned long)r->faults_triggered,
                (unsigned long)r->recovery_success, (unsigned long)r->recovery_time,
                (unsigned long)r->missed_deadlines_after_fault,
                (unsigned long)r->task_recovery_count, (unsigned long)r->task_restart_count,
                (float)r->system_availability_bp / 100.0f, (float)r->fault_coverage_bp / 100.0f,
                (unsigned long)r->adaptive_decisions, (unsigned long)r->adaptive_switches,
                (unsigned long)r->adaptive_decision_latency,
                (unsigned long)r->adaptive_overhead_ticks, (float)r->adaptive_accuracy_bp / 100.0f,
                is_last ? "" : ",");
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
