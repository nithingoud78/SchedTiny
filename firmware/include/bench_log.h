/**
 * @file    bench_log.h
 * @brief   Structured logging and serialization for SchedTiny benchmarks.
 *
 * Consumes the lock-free binary ring buffer from `bench_measure.c` and
 * serializes records into newline-delimited JSON format for UART DMA TX.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_BENCH_LOG_H
#define SCHEDTINY_BENCH_LOG_H

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief   Initialize the benchmark logging subsystem.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t bench_log_init(void);

    /**
     * @brief   Flush pending log entries from the ring buffer to UART via DMA.
     *
     * Typically called from a low-priority idle task. Extracts as many records
     * from the ring buffer as possible, formats them into a JSON chunk, and triggers
     * a DMA transfer. Does nothing if a previous DMA transfer is still busy.
     *
     * @return  SCHED_OK if items were flushed or no items pending.
     *          SCHED_ERR_STATE if UART DMA is busy.
     */
    SchedStatus_t bench_log_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_BENCH_LOG_H */
