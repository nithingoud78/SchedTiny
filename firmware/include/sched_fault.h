/**
 * @file    sched_fault.h
 * @brief   SchedTiny Fault Injection & Reliability Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_FAULT_H
#define SCHEDTINY_SCHED_FAULT_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

    /**
     * @brief   Supported fault types for injection.
     */
    typedef enum
    {
        SCHED_FAULT_EXECUTION_OVERRUN = 0,
        SCHED_FAULT_DEADLINE_MISS,
        SCHED_FAULT_STARVATION,
        SCHED_FAULT_PRIORITY_INVERSION,
        SCHED_FAULT_RANDOM_FAILURE,
        SCHED_FAULT_OMISSION,
        SCHED_FAULT_RESTART,
        SCHED_FAULT_DISPATCHER_FAILURE,
        SCHED_FAULT_READY_QUEUE_CORRUPTION,
        SCHED_FAULT_TICK_JITTER,
        SCHED_FAULT_INTERRUPT_LATENCY,
        SCHED_FAULT_CONTEXT_SWITCH_DELAY
    } sched_fault_type_t;

    /**
     * @brief   Aggregated statistics for fault injection.
     */
    typedef struct
    {
        uint32_t faults_injected;
        uint32_t faults_triggered;
        uint32_t recovery_success;
        uint32_t recovery_time;
        uint32_t missed_deadlines_after_fault;
        uint32_t cpu_utilization_during_fault;
        uint32_t scheduler_latency_during_fault;
        uint32_t task_recovery_count;
        uint32_t task_restart_count;
        uint32_t system_availability;
        uint32_t fault_coverage;
    } sched_fault_stats_t;

    /**
     * @brief   Initialize the fault injection framework.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_init(void);

    /**
     * @brief   Reset the fault injection framework and statistics.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_reset(void);

    /**
     * @brief   Enable a specific fault type for injection.
     * @param   fault Fault type to enable.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_enable(sched_fault_type_t fault);

    /**
     * @brief   Disable a specific fault type.
     * @param   fault Fault type to disable.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_disable(sched_fault_type_t fault);

    /**
     * @brief   Inject or configure a fault explicitly.
     * @param   fault Fault type to inject.
     * @param   parameter Fault-specific parameter (e.g., duration).
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_inject(sched_fault_type_t fault, uint32_t parameter);

    /**
     * @brief   Clear pending occurrences of a specific fault type.
     * @param   fault Fault type to clear.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_clear(sched_fault_type_t fault);

    /**
     * @brief   Get the current fault injection statistics.
     * @param   stats Pointer to store the statistics.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_fault_statistics(sched_fault_stats_t *stats);

    /**
     * @brief   Hook called on every simulation tick to process deterministic faults.
     * @param   benchmark_ctx Pointer to the benchmark context.
     * @param   current_task The currently executing task ID.
     * @param   tick The current simulation tick.
     */
    void sched_fault_tick_hook(void *benchmark_ctx, uint32_t current_task, uint32_t tick);

    /**
     * @brief   Hook called before dispatching to allow fault intervention.
     * @param   benchmark_ctx Pointer to the benchmark context.
     * @param   next_task Pointer to the task ID to be dispatched.
     */
    void sched_fault_dispatch_hook(void *benchmark_ctx, uint32_t *next_task);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_FAULT_H */
