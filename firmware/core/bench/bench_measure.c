/**
 * @file    bench_measure.c
 * @brief   Implementation of benchmark measurement primitives.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "bench_measure.h"

#include "bench_config.h"

#include "drv_timer.h"

// 64-bit cycle counter tracking
static uint32_t last_dwt_value  = 0;
static uint64_t cycle_overflows = 0;

// Lock-free ring buffer
static BenchEventRecord_t event_buffer[BENCH_RING_BUFFER_SIZE];
static volatile uint32_t buffer_head    = 0;
static volatile uint32_t buffer_tail    = 0;
static volatile uint32_t dropped_events = 0;

// Latency statistics
static uint32_t latency_samples[BENCH_LATENCY_WINDOW];
static uint32_t latency_count = 0;
static uint32_t latency_idx   = 0;

// Deadline misses
static uint32_t deadline_misses[BENCH_MAX_TASKS] = {0};

/* Mock critical section macros. In Milestone 2, these will map to FreeRTOS
   taskENTER_CRITICAL_FROM_ISR or ARM __disable_irq(). */
#ifndef BENCH_ENTER_CRITICAL
#define BENCH_ENTER_CRITICAL()
#define BENCH_EXIT_CRITICAL()
#endif

SchedStatus_t bench_init(void)
{
    drv_timer_dwt_init();
    last_dwt_value  = drv_timer_dwt_now();
    cycle_overflows = 0;

    buffer_head    = 0;
    buffer_tail    = 0;
    dropped_events = 0;

    bench_reset_stats();
    return SCHED_OK;
}

uint64_t bench_dwt_cycles64(void)
{
    BENCH_ENTER_CRITICAL();
    uint32_t now = drv_timer_dwt_now();
    if (now < last_dwt_value)
    {
        cycle_overflows++;
    }
    last_dwt_value     = now;
    uint64_t overflows = cycle_overflows;
    BENCH_EXIT_CRITICAL();

    return ((uint64_t)overflows << 32) | now;
}

uint32_t bench_cycles_to_us(uint64_t cycles)
{
    return (uint32_t)(cycles / 480);  // 480 MHz core clock
}

void bench_log_event(BenchEvent_t event, uint8_t task_id, uint32_t value)
{
    // Calculate timestamp early outside of critical section to minimize jitter
    uint64_t timestamp = bench_dwt_cycles64();

    BENCH_ENTER_CRITICAL();
    uint32_t next_head = (buffer_head + 1) % BENCH_RING_BUFFER_SIZE;

    if (next_head == buffer_tail)
    {
        // Buffer is full. Drop event to maintain tail integrity for readers.
        dropped_events++;
        BENCH_EXIT_CRITICAL();
        return;
    }

    event_buffer[buffer_head].timestamp = timestamp;
    event_buffer[buffer_head].event     = (uint8_t)event;
    event_buffer[buffer_head].task_id   = task_id;
    event_buffer[buffer_head].value     = value;

    // Publish the new head atomically
    buffer_head = next_head;
    BENCH_EXIT_CRITICAL();
}

uint32_t bench_get_log_records(BenchEventRecord_t *out_records, uint32_t max_records)
{
    uint32_t count = 0;

    while (buffer_tail != buffer_head && count < max_records)
    {
        out_records[count] = event_buffer[buffer_tail];
        // Consume the tail atomically
        BENCH_ENTER_CRITICAL();
        buffer_tail = (buffer_tail + 1) % BENCH_RING_BUFFER_SIZE;
        BENCH_EXIT_CRITICAL();
        count++;
    }

    return count;
}

uint32_t bench_get_dropped_events(void)
{
    return dropped_events;
}

void bench_record_latency(uint32_t latency_us)
{
    BENCH_ENTER_CRITICAL();
    latency_samples[latency_idx] = latency_us;
    latency_idx                  = (latency_idx + 1) % BENCH_LATENCY_WINDOW;
    if (latency_count < BENCH_LATENCY_WINDOW)
    {
        latency_count++;
    }
    BENCH_EXIT_CRITICAL();
}

void bench_record_deadline_miss(uint8_t task_id)
{
    if (task_id < BENCH_MAX_TASKS)
    {
        BENCH_ENTER_CRITICAL();
        deadline_misses[task_id]++;
        BENCH_EXIT_CRITICAL();
    }
}

// Simple selection sort for percentiles (modifies array in place)
static void sort_samples(uint32_t *arr, uint32_t n)
{
    for (uint32_t i = 0; i < n - 1; i++)
    {
        uint32_t min_idx = i;
        for (uint32_t j = i + 1; j < n; j++)
        {
            if (arr[j] < arr[min_idx])
            {
                min_idx = j;
            }
        }
        uint32_t temp = arr[min_idx];
        arr[min_idx]  = arr[i];
        arr[i]        = temp;
    }
}

bool bench_get_latency_stats(uint32_t *out_min_us,
                             uint32_t *out_max_us,
                             uint32_t *out_mean_us,
                             uint32_t *out_p95_us,
                             uint32_t *out_p99_us)
{
    BENCH_ENTER_CRITICAL();
    uint32_t count = latency_count;
    BENCH_EXIT_CRITICAL();

    if (count == 0)
    {
        return false;
    }

    // Allocate locally to avoid static variable data races
    uint32_t sorted[BENCH_LATENCY_WINDOW];
    uint64_t sum = 0;

    BENCH_ENTER_CRITICAL();
    for (uint32_t i = 0; i < count; i++)
    {
        sorted[i] = latency_samples[i];
        sum += sorted[i];
    }
    BENCH_EXIT_CRITICAL();

    sort_samples(sorted, count);

    *out_min_us  = sorted[0];
    *out_max_us  = sorted[count - 1];
    *out_mean_us = (uint32_t)(sum / count);

    uint32_t p95_idx = (count * 95) / 100;
    uint32_t p99_idx = (count * 99) / 100;

    *out_p95_us = sorted[p95_idx];
    *out_p99_us = sorted[p99_idx];

    return true;
}

uint32_t bench_get_deadline_miss_count(uint8_t task_id)
{
    if (task_id < BENCH_MAX_TASKS)
    {
        return deadline_misses[task_id];
    }
    return 0;
}

void bench_reset_stats(void)
{
    BENCH_ENTER_CRITICAL();
    latency_count = 0;
    latency_idx   = 0;
    for (int i = 0; i < BENCH_MAX_TASKS; i++)
    {
        deadline_misses[i] = 0;
    }
    BENCH_EXIT_CRITICAL();
}
