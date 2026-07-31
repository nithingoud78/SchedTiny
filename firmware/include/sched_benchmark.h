/**
 * @file    sched_benchmark.h
 * @brief   SchedTiny Benchmark Harness & Experimental Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_BENCHMARK_H
#define SCHEDTINY_SCHED_BENCHMARK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Maximum number of tasks the benchmark can hold (Stress profile).
 */
#ifndef SCHED_BENCHMARK_MAX_TASKS
#define SCHED_BENCHMARK_MAX_TASKS 128
#endif

    /**
     * @brief   Supported benchmark profiles defining workload sizes.
     */
    typedef enum
    {
        SCHED_BENCHMARK_PROFILE_SMALL = 0, /* 10 tasks */
        SCHED_BENCHMARK_PROFILE_MEDIUM,    /* 25 tasks */
        SCHED_BENCHMARK_PROFILE_LARGE,     /* 50 tasks */
        SCHED_BENCHMARK_PROFILE_STRESS     /* 100 tasks */
    } sched_benchmark_profile_t;

    /**
     * @brief   Supported scheduling policies for benchmarking.
     */
    typedef enum
    {
        SCHED_BENCHMARK_POLICY_HPF = 0,
        SCHED_BENCHMARK_POLICY_EDF,
        SCHED_BENCHMARK_POLICY_RMS
    } sched_benchmark_policy_t;

    /**
     * @brief   Workload types for generated tasks.
     */
    typedef enum
    {
        SCHED_WORKLOAD_PERIODIC = 0,
        SCHED_WORKLOAD_APERIODIC,
        SCHED_WORKLOAD_CPU_BOUND,
        SCHED_WORKLOAD_DEADLINE_SENSITIVE
    } sched_benchmark_workload_t;

    /**
     * @brief   Definition of a single benchmark task.
     */
    typedef struct
    {
        uint32_t task_id;
        uint32_t execution_time;
        uint32_t period;
        uint32_t deadline;
        uint32_t release_time;
        uint32_t priority;
        sched_benchmark_workload_t workload_type;

        /* Runtime tracking */
        uint32_t remaining_time;
        uint32_t last_release;
        uint32_t completion_count;
        uint32_t deadline_misses;
        uint32_t response_time_sum;
        uint32_t response_time_max;
        uint32_t waiting_time_sum;
        uint32_t waiting_time_max;
    } sched_benchmark_task_t;

    /**
     * @brief   Aggregated benchmark results for a single policy run.
     */
    typedef struct
    {
        sched_benchmark_policy_t policy;
        uint32_t total_tasks;
        uint32_t cpu_utilization_bp; /* Basis points (10000 = 100.00%) */
        uint32_t avg_scheduling_latency;
        uint32_t max_scheduling_latency;
        uint32_t min_scheduling_latency;
        uint32_t context_switches;
        uint32_t deadline_misses;
        uint32_t task_completion_count;
        uint32_t avg_response_time;
        uint32_t max_response_time;
        uint32_t avg_waiting_time;
        uint32_t max_waiting_time;
        uint32_t idle_time;
        uint32_t busy_time;
        uint32_t scheduler_invocation_count;
        uint32_t task_execution_count;
        uint32_t throughput; /* Completions per 10000 ticks */
        uint32_t estimated_energy_uj;
        uint32_t estimated_power_uw;
        uint32_t energy_per_task_uj;
        uint32_t energy_per_cs_uj;
    } sched_benchmark_results_t;

    /**
     * @brief   Benchmark engine context.
     */
    typedef struct
    {
        sched_benchmark_task_t tasks[SCHED_BENCHMARK_MAX_TASKS];
        uint32_t task_count;

        sched_benchmark_results_t results[3]; /* One for each policy */
        bool has_results[3];

        bool initialized;
    } sched_benchmark_t;

    /**
     * @brief   Initialize the benchmark engine.
     * @param   ctx Pointer to benchmark context.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_init(sched_benchmark_t *ctx);

    /**
     * @brief   Reset the benchmark engine.
     * @param   ctx Pointer to benchmark context.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_reset(sched_benchmark_t *ctx);

    /**
     * @brief   Load a predefined workload profile.
     * @param   ctx     Pointer to benchmark context.
     * @param   profile Profile to load (Small, Medium, Large, Stress).
     * @param   seed    Random seed for deterministic generation.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_load_workload(sched_benchmark_t *ctx,
                                                sched_benchmark_profile_t profile,
                                                uint32_t seed);

    /**
     * @brief   Run the currently loaded workload for a specific policy.
     * @param   ctx        Pointer to benchmark context.
     * @param   policy     The policy to execute.
     * @param   sim_ticks  Number of simulated ticks to run.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_run(sched_benchmark_t *ctx,
                                      sched_benchmark_policy_t policy,
                                      uint32_t sim_ticks);

    /**
     * @brief   Run the loaded workload for all supported policies sequentially.
     * @param   ctx        Pointer to benchmark context.
     * @param   sim_ticks  Number of simulated ticks per run.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_run_all(sched_benchmark_t *ctx, uint32_t sim_ticks);

    /**
     * @brief   Stop a running benchmark early.
     * @param   ctx Pointer to benchmark context.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_stop(sched_benchmark_t *ctx);

    /**
     * @brief   Get the results for a specific policy.
     * @param   ctx         Pointer to benchmark context.
     * @param   policy      The policy to retrieve results for.
     * @param   out_results Pointer to store the results.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if not run yet.
     */
    SchedStatus_t sched_benchmark_get_results(const sched_benchmark_t *ctx,
                                              sched_benchmark_policy_t policy,
                                              sched_benchmark_results_t *out_results);

    /**
     * @brief   Export all gathered results to JSON format.
     * @param   ctx     Pointer to benchmark context.
     * @param   buffer  Buffer to write to.
     * @param   max_len Maximum length.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_export_json(const sched_benchmark_t *ctx,
                                              char *buffer,
                                              size_t max_len);

    /**
     * @brief   Export all gathered results to CSV format.
     * @param   ctx     Pointer to benchmark context.
     * @param   buffer  Buffer to write to.
     * @param   max_len Maximum length.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_benchmark_export_csv(const sched_benchmark_t *ctx,
                                             char *buffer,
                                             size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_BENCHMARK_H */
