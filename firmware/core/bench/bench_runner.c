/**
 * @file    bench_runner.c
 * @brief   Generic FreeRTOS Task Wrapper for Benchmark Workloads.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "bench_runner.h"

static BenchWorkloadFunc_t resolve_workload_func(BenchWorkloadType_t type)
{
    switch (type)
    {
        case BENCH_WORKLOAD_BUSY_LOOP:
            return bench_workload_busy_loop;
        case BENCH_WORKLOAD_MATRIX_MATH:
            return bench_workload_matrix_math;
        case BENCH_WORKLOAD_MEMORY_COPY:
            return bench_workload_memory_copy;
        case BENCH_WORKLOAD_DELAY:
            return bench_workload_delay;
        case BENCH_WORKLOAD_EMPTY:
        default:
            return bench_workload_empty;
    }
}

void vBenchmarkTaskWrapper(void *pvParameters)
{
    BenchTaskConfig_t *config = (BenchTaskConfig_t *)pvParameters;

    const TickType_t xFrequency = pdMS_TO_TICKS(config->period_ms);
    TickType_t xLastWakeTime    = xTaskGetTickCount();

    BenchWorkloadFunc_t workload_func = resolve_workload_func(config->type);

    for (;;)
    {
        // Enforce periodicity
        vTaskDelayUntil(&xLastWakeTime, xFrequency);

        // Mark execution start
        bench_log_event(BENCH_EV_TASK_START, config->task_id, 0);
        uint64_t start_cycles = bench_dwt_cycles64();

        // Execute the configured generic workload
        workload_func(&config->params);

        // Mark execution end
        uint64_t end_cycles   = bench_dwt_cycles64();
        uint32_t exec_time_us = bench_cycles_to_us(end_cycles - start_cycles);

        bench_record_latency(exec_time_us);
        bench_log_event(BENCH_EV_TASK_END, config->task_id, exec_time_us);

        // Jitter / Deadline miss checking can be evaluated via host-side analysis of UART logs.
    }
}

void bench_runner_spawn_all(BenchTaskConfig_t *configs, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++)
    {
        xTaskCreateStatic(vBenchmarkTaskWrapper,
                          "BenchTask",  // Could dynamically generate name based on ID later
                          configs[i].stack_size, (void *)&configs[i], configs[i].priority,
                          configs[i].stack, &configs[i].tcb);
    }
}
