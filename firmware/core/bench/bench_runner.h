/**
 * @file    bench_runner.h
 * @brief   Generic FreeRTOS Task Wrapper for Benchmark Workloads.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#ifndef SCHEDTINY_BENCH_RUNNER_H
#define SCHEDTINY_BENCH_RUNNER_H

#include "bench_measure.h"
#include "bench_workloads.h"
#include "task.h"

#include "FreeRTOS.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Complete configuration for a benchmark task.
     */
    typedef struct
    {
        uint8_t task_id;
        BenchWorkloadType_t type;
        uint32_t period_ms;
        UBaseType_t priority;
        BenchWorkloadParams_t params;

        // Internal RTOS State
        StaticTask_t tcb;
        StackType_t *stack;
        uint32_t stack_size;
    } BenchTaskConfig_t;

    /**
     * @brief The universal FreeRTOS task function for benchmark workloads.
     *
     * Takes a pointer to a BenchTaskConfig_t as its parameter.
     */
    void vBenchmarkTaskWrapper(void *pvParameters);

    /**
     * @brief Initialize and spawn all benchmark tasks defined in an array.
     *
     * @param configs Array of task configurations.
     * @param count   Number of tasks in the array.
     */
    void bench_runner_spawn_all(BenchTaskConfig_t *configs, uint32_t count);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_BENCH_RUNNER_H */
