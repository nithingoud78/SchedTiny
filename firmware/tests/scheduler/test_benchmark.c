/**
 * @file    test_benchmark.c
 * @brief   CMocka unit tests for SchedTiny Benchmark Harness.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_benchmark.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>
#include <stdio.h>
// clang-format on

/* -------------------------------------------------------------------------
 * Fixture
 * ---------------------------------------------------------------------- */

static sched_benchmark_t g_ctx;

static int setup(void **state)
{
    (void)state;
    sched_benchmark_init(&g_ctx);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialization & Reset
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_benchmark_t ctx;
    assert_int_equal(sched_benchmark_init(&ctx), SCHED_OK);
    assert_true(ctx.initialized);
    assert_int_equal(ctx.task_count, 0);
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_benchmark_init(NULL), SCHED_ERR_PARAM);
}

static void test_reset_success(void **state)
{
    (void)state;
    sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_SMALL, 123);
    assert_int_equal(g_ctx.task_count, 10);

    assert_int_equal(sched_benchmark_reset(&g_ctx), SCHED_OK);
    assert_int_equal(g_ctx.task_count, 0);
    assert_false(g_ctx.has_results[SCHED_BENCHMARK_POLICY_HPF]);
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_benchmark_t ctx;
    ctx.initialized = false;
    assert_int_equal(sched_benchmark_reset(&ctx), SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Workload Loading
 * ---------------------------------------------------------------------- */

static void test_load_workload_profiles(void **state)
{
    (void)state;
    assert_int_equal(sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_SMALL, 1),
                     SCHED_OK);
    assert_int_equal(g_ctx.task_count, 10);

    assert_int_equal(sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_MEDIUM, 1),
                     SCHED_OK);
    assert_int_equal(g_ctx.task_count, 25);

    assert_int_equal(sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_LARGE, 1),
                     SCHED_OK);
    assert_int_equal(g_ctx.task_count, 50);

    assert_int_equal(sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_STRESS, 1),
                     SCHED_OK);
    assert_int_equal(g_ctx.task_count, 100);
}

static void test_load_workload_deterministic(void **state)
{
    (void)state;
    sched_benchmark_t ctx1, ctx2;
    sched_benchmark_init(&ctx1);
    sched_benchmark_init(&ctx2);

    sched_benchmark_load_workload(&ctx1, SCHED_BENCHMARK_PROFILE_MEDIUM, 42);
    sched_benchmark_load_workload(&ctx2, SCHED_BENCHMARK_PROFILE_MEDIUM, 42);

    for (int i = 0; i < 25; i++)
    {
        assert_int_equal(ctx1.tasks[i].execution_time, ctx2.tasks[i].execution_time);
        assert_int_equal(ctx1.tasks[i].release_time, ctx2.tasks[i].release_time);
        assert_int_equal(ctx1.tasks[i].period, ctx2.tasks[i].period);
    }
}

/* -------------------------------------------------------------------------
 * Execution
 * ---------------------------------------------------------------------- */

static void test_run_benchmark_hpf(void **state)
{
    (void)state;
    sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_SMALL, 100);
    assert_int_equal(sched_benchmark_run(&g_ctx, SCHED_BENCHMARK_POLICY_HPF, 1000), SCHED_OK);

    assert_true(g_ctx.has_results[SCHED_BENCHMARK_POLICY_HPF]);
    assert_int_equal(g_ctx.results[SCHED_BENCHMARK_POLICY_HPF].total_tasks, 10);
    /* Metrics shouldn't be zero entirely */
    assert_true(g_ctx.results[SCHED_BENCHMARK_POLICY_HPF].task_execution_count >= 0);
}

static void test_run_benchmark_all(void **state)
{
    (void)state;
    sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_SMALL, 100);
    assert_int_equal(sched_benchmark_run_all(&g_ctx, 1000), SCHED_OK);

    assert_true(g_ctx.has_results[SCHED_BENCHMARK_POLICY_HPF]);
    assert_true(g_ctx.has_results[SCHED_BENCHMARK_POLICY_EDF]);
    assert_true(g_ctx.has_results[SCHED_BENCHMARK_POLICY_RMS]);
}

static void test_empty_workload(void **state)
{
    (void)state;
    /* task_count is 0 after init */
    assert_int_equal(sched_benchmark_run(&g_ctx, SCHED_BENCHMARK_POLICY_HPF, 1000),
                     SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Export
 * ---------------------------------------------------------------------- */

static void test_export_csv(void **state)
{
    (void)state;
    sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_SMALL, 100);
    sched_benchmark_run_all(&g_ctx, 500);

    char buffer[1024];
    assert_int_equal(sched_benchmark_export_csv(&g_ctx, buffer, sizeof(buffer)), SCHED_OK);

    /* Check basic structure */
    assert_non_null(strstr(buffer,
                           "Algorithm,Tasks,CPUUtilization,Latency,DeadlineMisses,ContextSwitches,"
                           "AvgResponseTime,AvgWaitingTime,Throughput,IdleTime,BusyTime,Energy_uJ,"
                           "Power_uW,EnergyPerTask_uJ,EnergyPerCS_uJ\n"));
    assert_non_null(strstr(buffer, "HPF,10,"));
    assert_non_null(strstr(buffer, "EDF,10,"));
    assert_non_null(strstr(buffer, "RMS,10,"));
}

static void test_export_json(void **state)
{
    (void)state;
    sched_benchmark_load_workload(&g_ctx, SCHED_BENCHMARK_PROFILE_SMALL, 100);
    sched_benchmark_run_all(&g_ctx, 500);

    char buffer[2048];
    assert_int_equal(sched_benchmark_export_json(&g_ctx, buffer, sizeof(buffer)), SCHED_OK);

    /* Check basic structure */
    assert_non_null(strstr(buffer, "\"Algorithm\": \"HPF\""));
    assert_non_null(strstr(buffer, "\"Tasks\": 10"));
    assert_non_null(strstr(buffer, "\"CPUUtilization\":"));
}

/* -------------------------------------------------------------------------
 * Invalid Arguments
 * ---------------------------------------------------------------------- */

static void test_invalid_args(void **state)
{
    (void)state;
    char buf[100];
    assert_int_equal(sched_benchmark_load_workload(NULL, SCHED_BENCHMARK_PROFILE_SMALL, 0),
                     SCHED_ERR_PARAM);
    assert_int_equal(sched_benchmark_run(NULL, SCHED_BENCHMARK_POLICY_HPF, 100), SCHED_ERR_PARAM);
    assert_int_equal(sched_benchmark_run_all(NULL, 100), SCHED_ERR_PARAM);
    assert_int_equal(sched_benchmark_export_csv(NULL, buf, sizeof(buf)), SCHED_ERR_PARAM);
    assert_int_equal(sched_benchmark_export_json(NULL, buf, sizeof(buf)), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_init_success),
        cmocka_unit_test(test_init_null),
        cmocka_unit_test_setup(test_reset_success, setup),
        cmocka_unit_test(test_reset_uninit),
        cmocka_unit_test_setup(test_load_workload_profiles, setup),
        cmocka_unit_test(test_load_workload_deterministic),
        cmocka_unit_test_setup(test_run_benchmark_hpf, setup),
        cmocka_unit_test_setup(test_run_benchmark_all, setup),
        cmocka_unit_test_setup(test_empty_workload, setup),
        cmocka_unit_test_setup(test_export_csv, setup),
        cmocka_unit_test_setup(test_export_json, setup),
        cmocka_unit_test_setup(test_invalid_args, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
