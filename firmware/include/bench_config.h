/**
 * @file    bench_config.h
 * @brief   Compile-time configuration limits for the measurement subsystem.
 *
 * Defines static sizing limits decoupled from experiment configs to ensure
 * stable firmware sizing across tests.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_BENCH_CONFIG_H
#define SCHEDTINY_BENCH_CONFIG_H

/**
 * @brief   Maximum number of events held in the lock-free ring buffer.
 * Memory footprint: BENCH_RING_BUFFER_SIZE * sizeof(BenchEventRecord_t) (usually 8 bytes).
 */
#define BENCH_RING_BUFFER_SIZE 256

/**
 * @brief   Maximum size of the JSON transmission buffer.
 * Used for DMA UART transmission. Must be large enough for BENCH_RING_BUFFER_SIZE events
 * serialized to JSON if fully flushed.
 */
#define BENCH_JSON_BUFFER_SIZE 2048

/**
 * @brief   Maximum number of tasks the measurement subsystem tracks stats for.
 */
#define BENCH_MAX_TASKS 8

/**
 * @brief   Default sliding window size for latency stats (min, max, p95, p99).
 */
#define BENCH_LATENCY_WINDOW 1000

#endif /* SCHEDTINY_BENCH_CONFIG_H */
