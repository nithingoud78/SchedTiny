/**
 * @file    bench_workloads.c
 * @brief   Implementations of the generic benchmark workloads.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "bench_workloads.h"

#include <string.h>

#include "drv_timer.h"

#define MAX_MATRIX_SIZE 10
#define MAX_MEM_BUFFER  1024

static uint32_t matrix_a[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];
static uint32_t matrix_b[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];
static uint32_t matrix_c[MAX_MATRIX_SIZE][MAX_MATRIX_SIZE];

static uint8_t mem_src[MAX_MEM_BUFFER];
static uint8_t mem_dst[MAX_MEM_BUFFER];

void bench_workload_empty(const BenchWorkloadParams_t *params)
{
    (void)params;
    // Do absolutely nothing. Used for base scheduler overhead profiling.
}

void bench_workload_busy_loop(const BenchWorkloadParams_t *params)
{
    // Volatile to prevent compiler from optimizing the empty loop away
    for (volatile uint32_t i = 0; i < params->iterations; i++)
    {
        __asm volatile("nop");
    }
}

void bench_workload_matrix_math(const BenchWorkloadParams_t *params)
{
    uint32_t dim = params->memory_size;
    if (dim > MAX_MATRIX_SIZE)
        dim = MAX_MATRIX_SIZE;
    if (dim == 0)
        dim = 2;  // Fallback

    for (uint32_t iter = 0; iter < params->iterations; iter++)
    {
        for (uint32_t i = 0; i < dim; i++)
        {
            for (uint32_t j = 0; j < dim; j++)
            {
                matrix_c[i][j] = 0;
                for (uint32_t k = 0; k < dim; k++)
                {
                    matrix_c[i][j] += matrix_a[i][k] * matrix_b[k][j];
                }
            }
        }
    }
}

void bench_workload_memory_copy(const BenchWorkloadParams_t *params)
{
    uint32_t sz = params->memory_size;
    if (sz > MAX_MEM_BUFFER)
        sz = MAX_MEM_BUFFER;
    if (sz == 0)
        sz = 64;  // Fallback

    for (uint32_t i = 0; i < params->iterations; i++)
    {
        // Volatile write to ensure actual memory bus traffic
        memcpy(mem_dst, mem_src, sz);
        // Prevent complete optimization out
        *(volatile uint8_t *)&mem_dst[0] = mem_src[0];
    }
}

void bench_workload_delay(const BenchWorkloadParams_t *params)
{
    (void)params;
    // Uses the hardware DWT timer to block without yielding the CPU to the OS
    if (params->delay_us > 0)
    {
        drv_timer_delay_us(params->delay_us);
    }
}
