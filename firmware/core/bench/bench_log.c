/**
 * @file    bench_log.c
 * @brief   Implementation of JSON structured logging over UART DMA.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "bench_log.h"

#include "bench_config.h"
#include "bench_measure.h"

#include <stdio.h>
#include <string.h>

#include "drv_uart.h"

// Pre-allocate a large buffer to hold the serialized JSON chunk
static char json_buffer[BENCH_JSON_BUFFER_SIZE];

SchedStatus_t bench_log_init(void)
{
    // 115200 is standard for the ST-Link Virtual COM port
    return drv_uart_init(115200);
}

SchedStatus_t bench_log_flush(void)
{
    if (drv_uart_is_tx_busy())
    {
        return SCHED_ERR_STATE;  // Still busy, try again later
    }

    // We fetch a small batch to avoid blocking the idle task for too long.
    // 16 records at a time is a safe limit to stay within the JSON buffer.
    BenchEventRecord_t records[16];
    uint32_t count = bench_get_log_records(records, 16);

    if (count == 0)
    {
        return SCHED_OK;  // Nothing to do
    }

    uint32_t offset = 0;
    for (uint32_t i = 0; i < count; i++)
    {
        // Format: {"ts":123456,"ev":0,"tid":1,"val":0}\n
        int written = snprintf(json_buffer + offset, BENCH_JSON_BUFFER_SIZE - offset,
                               "{\"ts\":%llu,\"ev\":%u,\"tid\":%u,\"val\":%lu}\n",
                               (unsigned long long)records[i].timestamp, records[i].event,
                               records[i].task_id, (unsigned long)records[i].value);
        if (written > 0 && offset + written < BENCH_JSON_BUFFER_SIZE)
        {
            offset += written;
        }
        else
        {
            break;  // Truncated, shouldn't happen with 16 records in 2048 bytes
        }
    }

    if (offset > 0)
    {
        return drv_uart_tx_dma((const uint8_t *)json_buffer, (uint16_t)offset);
    }

    return SCHED_OK;
}
