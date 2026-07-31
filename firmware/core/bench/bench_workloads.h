/**
 * @file    bench_workloads.h
 * @brief   Scheduler-Independent Benchmark Workloads.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#ifndef SCHEDTINY_BENCH_WORKLOADS_H
#define SCHEDTINY_BENCH_WORKLOADS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        BENCH_WORKLOAD_EMPTY,
        BENCH_WORKLOAD_BUSY_LOOP,
        BENCH_WORKLOAD_MATRIX_MATH,
        BENCH_WORKLOAD_MEMORY_COPY,
        BENCH_WORKLOAD_DELAY
    } BenchWorkloadType_t;

    /**
     * @brief Universal configuration block for a benchmark workload.
     */
    typedef struct
    {
        uint32_t iterations;   ///< Number of inner loops (for busy loop / math)
        uint32_t memory_size;  ///< Size in bytes for memcopy workloads
        uint32_t delay_us;     ///< Time in microseconds for delay workload
    } BenchWorkloadParams_t;

    /**
     * @brief Type signature for all workload functions.
     */
    typedef void (*BenchWorkloadFunc_t)(const BenchWorkloadParams_t *params);

    /* Workload Implementations */
    void bench_workload_empty(const BenchWorkloadParams_t *params);
    void bench_workload_busy_loop(const BenchWorkloadParams_t *params);
    void bench_workload_matrix_math(const BenchWorkloadParams_t *params);
    void bench_workload_memory_copy(const BenchWorkloadParams_t *params);
    void bench_workload_delay(const BenchWorkloadParams_t *params);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_BENCH_WORKLOADS_H */
