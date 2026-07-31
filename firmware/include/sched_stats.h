/**
 * @file    sched_stats.h
 * @brief   SchedTiny Runtime Statistics & Benchmark Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_STATS_H
#define SCHEDTINY_SCHED_STATS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

    /**
     * @brief   Data structure holding the gathered statistics.
     */
    typedef struct
    {
        uint32_t scheduler_execution_count;
        uint32_t task_execution_count;
        uint32_t context_switch_count;
        uint32_t scheduler_invocation_count;
        uint32_t idle_time;
        uint32_t busy_time;
        uint32_t cpu_utilization; /**< CPU Utilization in basis points (0-10000, where 10000 =
                                     100.00%) */
        uint32_t avg_scheduling_latency; /**< Average latency in ticks */
        uint32_t max_scheduling_latency; /**< Maximum latency in ticks */
        uint32_t min_scheduling_latency; /**< Minimum latency in ticks */

        /* Internal fields for calculation */
        uint32_t _total_latency;
        uint32_t _latency_samples;
    } sched_stats_data_t;

    /**
     * @brief   Runtime Statistics Context.
     */
    typedef struct
    {
        sched_stats_data_t data;
        bool active;
        bool initialized;
        uint32_t start_time;
        uint32_t end_time;
    } sched_stats_t;

    /**
     * @brief   Initialize the statistics context.
     * @param   ctx Pointer to statistics context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if NULL.
     */
    SchedStatus_t sched_stats_init(sched_stats_t *ctx);

    /**
     * @brief   Reset all counters in the statistics context.
     * @param   ctx Pointer to statistics context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if NULL.
     */
    SchedStatus_t sched_stats_reset(sched_stats_t *ctx);

    /**
     * @brief   Start the benchmarking interval.
     * @param   ctx          Pointer to statistics context.
     * @param   current_time The current time in ticks.
     * @return  SCHED_OK on success, SCHED_ERR_STATE if already active.
     */
    SchedStatus_t sched_stats_start(sched_stats_t *ctx, uint32_t current_time);

    /**
     * @brief   Stop the benchmarking interval and compute final metrics.
     * @param   ctx          Pointer to statistics context.
     * @param   current_time The current time in ticks.
     * @return  SCHED_OK on success, SCHED_ERR_STATE if not active.
     */
    SchedStatus_t sched_stats_stop(sched_stats_t *ctx, uint32_t current_time);

    /**
     * @brief   Record a scheduler invocation.
     */
    SchedStatus_t sched_stats_record_invocation(sched_stats_t *ctx);

    /**
     * @brief   Record a scheduler execution (alias/variant of invocation).
     */
    SchedStatus_t sched_stats_record_execution(sched_stats_t *ctx);

    /**
     * @brief   Record a task execution.
     */
    SchedStatus_t sched_stats_record_task_exec(sched_stats_t *ctx);

    /**
     * @brief   Record a context switch.
     */
    SchedStatus_t sched_stats_record_context_switch(sched_stats_t *ctx);

    /**
     * @brief   Record idle time accumulation.
     */
    SchedStatus_t sched_stats_record_idle_time(sched_stats_t *ctx, uint32_t idle_ticks);

    /**
     * @brief   Record busy time accumulation.
     */
    SchedStatus_t sched_stats_record_busy_time(sched_stats_t *ctx, uint32_t busy_ticks);

    /**
     * @brief   Record a new scheduling latency sample.
     */
    SchedStatus_t sched_stats_record_latency(sched_stats_t *ctx, uint32_t latency_ticks);

    /**
     * @brief   Take a snapshot of the current metrics.
     *          Will compute derived metrics like averages and utilization.
     * @param   ctx          Pointer to statistics context.
     * @param   out_snapshot Pointer to store the result.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_stats_snapshot(sched_stats_t *ctx, sched_stats_data_t *out_snapshot);

    /**
     * @brief   Export statistics to a string buffer (e.g., CSV or JSON format).
     * @param   snapshot Pointer to the gathered stats.
     * @param   buffer   String buffer to write to.
     * @param   max_len  Maximum length of the buffer.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if arguments invalid.
     */
    SchedStatus_t sched_stats_export(const sched_stats_data_t *snapshot,
                                     char *buffer,
                                     size_t max_len);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_STATS_H */
