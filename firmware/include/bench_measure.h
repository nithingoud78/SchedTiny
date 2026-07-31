/**
 * @file    bench_measure.h
 * @brief   SchedTiny benchmark measurement primitives.
 *
 * This module provides the primary measurement API for SchedTiny experiments.
 * It is a first-class firmware component, not an afterthought, and forms the
 * data collection foundation for all journal figures.
 *
 * Design principles:
 *   - Zero interrupt overhead: DWT->CYCCNT reads only (no SysTick, no ITM)
 *   - Non-blocking output: measurements stored in ring buffer; flushed via DMA
 *   - Deterministic: measurement code has a fixed, analyzable execution time
 *   - Overflow-safe: 32-bit DWT wraps at ~8.9s @ 480 MHz; handled explicitly
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_BENCH_MEASURE_H
#define SCHEDTINY_BENCH_MEASURE_H

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /* =========================================================================
     * Event Types (logged in JSON output)
     * ========================================================================= */

    /**
     * @brief   Event type identifiers for structured UART log output.
     *
     * Each event appears as a JSON record:
     * {"ts":123456,"ev":"TASK_START","tid":1,"val":0}
     */
    typedef enum
    {
        BENCH_EV_TASK_START    = 0, /**< Task execution started */
        BENCH_EV_TASK_END      = 1, /**< Task execution ended */
        BENCH_EV_INFER_START   = 2, /**< TFLM inference started */
        BENCH_EV_INFER_END     = 3, /**< TFLM inference ended */
        BENCH_EV_ISR_ENTRY     = 4, /**< ISR entered (source in val field) */
        BENCH_EV_ISR_EXIT      = 5, /**< ISR exited */
        BENCH_EV_DEADLINE_MISS = 6, /**< Task missed its deadline */
        BENCH_EV_WINDOW_START  = 7, /**< Measurement window start marker */
        BENCH_EV_WINDOW_END    = 8, /**< Measurement window end marker */
    } BenchEvent_t;

    /**
     * @brief   Single event record stored in the binary ring buffer.
     */
    typedef struct
    {
        uint64_t timestamp; /**< 64-bit cycle count timestamp */
        uint32_t value;     /**< Optional value payload */
        uint8_t event;      /**< Event type (BenchEvent_t) */
        uint8_t task_id;    /**< Task or ISR identifier */
    } BenchEventRecord_t;

    /* =========================================================================
     * Public API
     * ========================================================================= */

    /**
     * @brief   Initialize the benchmark measurement subsystem.
     *
     * Enables DWT cycle counter, clears ring buffer, resets statistics.
     * Must be called before any task is created.
     * Thread-safe: No.
     *
     * @return  SCHED_OK on success.
     */
    SchedStatus_t bench_init(void);

    /**
     * @brief   Read the current DWT cycle counter, handling 32-bit overflow.
     *
     * Internally tracks overflow count. At 480 MHz, DWT wraps at ~8.9 seconds.
     * This function returns a monotonically increasing 64-bit cycle count.
     * Must be called more frequently than the 8.9s rollover period to accurately
     * track time.
     *
     * @return  64-bit cycle count since bench_init().
     */
    uint64_t bench_dwt_cycles64(void);

    /**
     * @brief   Convert CPU cycles to microseconds.
     *
     * @param[in] cycles  CPU cycle count.
     * @return            Time in microseconds (truncated, not rounded).
     */
    uint32_t bench_cycles_to_us(uint64_t cycles);

    /**
     * @brief   Log a benchmark event to the ring buffer.
     *
     * This is the primary logging call. It is interrupt-safe and has bounded
     * execution time (< 20 CPU cycles on M7).
     *
     * @param[in] event    Event type (BenchEvent_t).
     * @param[in] task_id  Task or ISR source identifier.
     * @param[in] value    Optional value (e.g., latency in us, ISR source).
     */
    void bench_log_event(BenchEvent_t event, uint8_t task_id, uint32_t value);

    /**
     * @brief   Extract a batch of records from the ring buffer.
     *
     * @param[out] out_records Pointer to array to hold extracted records.
     * @param[in] max_records Maximum number of records to extract.
     * @return Number of records actually extracted.
     */
    uint32_t bench_get_log_records(BenchEventRecord_t *out_records, uint32_t max_records);

    /**
     * @brief   Get the number of dropped log events due to ring buffer overflow.
     * @return  Dropped event count.
     */
    uint32_t bench_get_dropped_events(void);

    /**
     * @brief   Record an inference latency sample.
     *
     * Adds a sample to the sliding window for latency statistics.
     * Statistics (min, max, mean, p95, p99) are computed on demand.
     *
     * @param[in] latency_us  Inference latency in microseconds.
     */
    void bench_record_latency(uint32_t latency_us);

    /**
     * @brief   Record a deadline miss for a task.
     * @param[in] task_id  Task identifier.
     */
    void bench_record_deadline_miss(uint8_t task_id);

    /**
     * @brief   Compute latency statistics over the current window.
     *
     * @param[out] out_min_us   Minimum latency (microseconds).
     * @param[out] out_max_us   Maximum latency (microseconds).
     * @param[out] out_mean_us  Mean latency (microseconds, rounded).
     * @param[out] out_p95_us   95th percentile (microseconds).
     * @param[out] out_p99_us   99th percentile (microseconds).
     *
     * @return  true if statistics are valid (window has enough samples), false otherwise.
     */
    bool bench_get_latency_stats(uint32_t *out_min_us,
                                 uint32_t *out_max_us,
                                 uint32_t *out_mean_us,
                                 uint32_t *out_p95_us,
                                 uint32_t *out_p99_us);

    /**
     * @brief   Return the deadline miss count for a task.
     * @param[in] task_id  Task identifier.
     * @return             Number of deadline misses since last reset.
     */
    uint32_t bench_get_deadline_miss_count(uint8_t task_id);

    /**
     * @brief   Reset all measurement statistics (but not the log buffer).
     *
     * Call at the start of each measurement window.
     */
    void bench_reset_stats(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_BENCH_MEASURE_H */
