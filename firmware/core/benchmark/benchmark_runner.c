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

static int parse_workload_csv(const char *filename, sched_benchmark_t *ctx)
{
    FILE *f = fopen(filename, "r");
    if (!f)
    {
        fprintf(stderr, "Could not open workload CSV: %s\n", filename);
        return -1;
    }

    char line[256];
    // Skip header
    if (!fgets(line, sizeof(line), f))
    {
        fclose(f);
        return -1;
    }

    ctx->task_count = 0;
    while (fgets(line, sizeof(line), f) && ctx->task_count < SCHED_BENCHMARK_MAX_TASKS)
    {
        sched_benchmark_task_t *task = &ctx->tasks[ctx->task_count];
        // Expected format:
        // TaskID,ExecutionTime,Period,Deadline,ReleaseTime,Priority,WorkloadType,Criticality,LoWCET,HiWCET
        int parsed =
            sscanf(line, "%lu,%lu,%lu,%lu,%lu,%lu,%u,%hhu,%lu,%lu", (unsigned long *)&task->task_id,
                   (unsigned long *)&task->execution_time, (unsigned long *)&task->period,
                   (unsigned long *)&task->deadline, (unsigned long *)&task->release_time,
                   (unsigned long *)&task->priority, &task->workload_type, &task->criticality,
                   (unsigned long *)&task->lo_wcet, (unsigned long *)&task->hi_wcet);
        if (parsed >= 10)
        {
            ctx->task_count++;
        }
    }
    fclose(f);
    return 0;
}

int main(int argc, char *argv[])
{
    sched_benchmark_profile_t profile = SCHED_BENCHMARK_PROFILE_SMALL;
    uint32_t seed                     = 42;
    uint32_t sim_ticks                = 10000;
    const char *format                = "csv";
    const char *csv_file              = NULL;

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
        else if (strcmp(argv[i], "--workload-csv") == 0 && i + 1 < argc)
        {
            csv_file = argv[++i];
        }
    }

    sched_benchmark_t ctx;
    if (sched_benchmark_init(&ctx) != SCHED_OK)
    {
        fprintf(stderr, "Failed to init benchmark\n");
        return 1;
    }

    if (csv_file)
    {
        if (parse_workload_csv(csv_file, &ctx) != 0)
        {
            fprintf(stderr, "Failed to load CSV workload\n");
            return 1;
        }
    }
    else
    {
        if (sched_benchmark_load_workload(&ctx, profile, seed) != SCHED_OK)
        {
            fprintf(stderr, "Failed to load workload\n");
            return 1;
        }
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
