/**
 * @file    benchmark_runner.c
 * @brief   CLI executable for SchedTiny benchmark harness
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_benchmark.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    sched_benchmark_profile_t profile = SCHED_BENCHMARK_PROFILE_SMALL;
    uint32_t seed                     = 42;
    uint32_t sim_ticks                = 10000;
    const char *format                = "csv";

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--profile") == 0 && i + 1 < argc)
        {
            const char *p = argv[++i];
            if (strcmp(p, "small") == 0)
                profile = SCHED_BENCHMARK_PROFILE_SMALL;
            else if (strcmp(p, "medium") == 0)
                profile = SCHED_BENCHMARK_PROFILE_MEDIUM;
            else if (strcmp(p, "large") == 0)
                profile = SCHED_BENCHMARK_PROFILE_LARGE;
            else if (strcmp(p, "stress") == 0)
                profile = SCHED_BENCHMARK_PROFILE_STRESS;
        }
        else if (strcmp(argv[i], "--seed") == 0 && i + 1 < argc)
        {
            seed = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--ticks") == 0 && i + 1 < argc)
        {
            sim_ticks = (uint32_t)atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--out") == 0 && i + 1 < argc)
        {
            format = argv[++i];
        }
    }

    sched_benchmark_t ctx;
    if (sched_benchmark_init(&ctx) != SCHED_OK)
    {
        fprintf(stderr, "Failed to init benchmark\n");
        return 1;
    }

    if (sched_benchmark_load_workload(&ctx, profile, seed) != SCHED_OK)
    {
        fprintf(stderr, "Failed to load workload\n");
        return 1;
    }

    if (sched_benchmark_run_all(&ctx, sim_ticks) != SCHED_OK)
    {
        fprintf(stderr, "Failed to run benchmark\n");
        return 1;
    }

    char buffer[4096];
    if (strcmp(format, "json") == 0)
    {
        if (sched_benchmark_export_json(&ctx, buffer, sizeof(buffer)) != SCHED_OK)
        {
            fprintf(stderr, "Failed to export json\n");
            return 1;
        }
    }
    else
    {
        if (sched_benchmark_export_csv(&ctx, buffer, sizeof(buffer)) != SCHED_OK)
        {
            fprintf(stderr, "Failed to export csv\n");
            return 1;
        }
    }

    printf("%s", buffer);
    return 0;
}
