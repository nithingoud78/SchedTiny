/**
 * @file    test_stats.c
 * @brief   CMocka unit tests for SchedTiny Statistics Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_stats.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>
// clang-format on

/* -------------------------------------------------------------------------
 * Fixture
 * ---------------------------------------------------------------------- */

static sched_stats_t g_ctx;

static int setup(void **state)
{
    (void)state;
    sched_stats_init(&g_ctx);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialization & Reset
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_stats_t ctx;
    assert_int_equal(sched_stats_init(&ctx), SCHED_OK);
    assert_true(ctx.initialized);
    assert_int_equal(ctx.data.min_scheduling_latency, UINT32_MAX);
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_stats_init(NULL), SCHED_ERR_PARAM);
}

static void test_reset_success(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 10);
    sched_stats_record_invocation(&g_ctx);
    sched_stats_stop(&g_ctx, 20);

    assert_int_equal(sched_stats_reset(&g_ctx), SCHED_OK);
    assert_false(g_ctx.active);
    assert_int_equal(g_ctx.data.scheduler_invocation_count, 0);
    assert_int_equal(g_ctx.data.min_scheduling_latency, UINT32_MAX);
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_stats_t ctx;
    ctx.initialized = false;
    assert_int_equal(sched_stats_reset(&ctx), SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Start / Stop state
 * ---------------------------------------------------------------------- */

static void test_start_stop(void **state)
{
    (void)state;
    assert_int_equal(sched_stats_start(&g_ctx, 100), SCHED_OK);
    assert_true(g_ctx.active);
    assert_int_equal(g_ctx.start_time, 100);

    assert_int_equal(sched_stats_start(&g_ctx, 200), SCHED_ERR_STATE);  // already active

    assert_int_equal(sched_stats_stop(&g_ctx, 300), SCHED_OK);
    assert_false(g_ctx.active);
    assert_int_equal(g_ctx.end_time, 300);

    assert_int_equal(sched_stats_stop(&g_ctx, 400), SCHED_ERR_STATE);  // not active
}

/* -------------------------------------------------------------------------
 * Counter Updates
 * ---------------------------------------------------------------------- */

static void test_counter_updates(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);

    for (int i = 0; i < 5; i++)
    {
        assert_int_equal(sched_stats_record_invocation(&g_ctx), SCHED_OK);
        assert_int_equal(sched_stats_record_execution(&g_ctx), SCHED_OK);
        assert_int_equal(sched_stats_record_task_exec(&g_ctx), SCHED_OK);
        assert_int_equal(sched_stats_record_context_switch(&g_ctx), SCHED_OK);
    }

    sched_stats_data_t snap;
    assert_int_equal(sched_stats_snapshot(&g_ctx, &snap), SCHED_OK);

    assert_int_equal(snap.scheduler_invocation_count, 5);
    assert_int_equal(snap.scheduler_execution_count, 5);
    assert_int_equal(snap.task_execution_count, 5);
    assert_int_equal(snap.context_switch_count, 5);
}

static void test_updates_when_inactive(void **state)
{
    (void)state;
    /* Not active initially */
    assert_int_equal(sched_stats_record_invocation(&g_ctx), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Latency Calculations
 * ---------------------------------------------------------------------- */

static void test_latency_calculations(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);

    sched_stats_record_latency(&g_ctx, 10);
    sched_stats_record_latency(&g_ctx, 50);
    sched_stats_record_latency(&g_ctx, 30);

    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);

    assert_int_equal(snap.min_scheduling_latency, 10);
    assert_int_equal(snap.max_scheduling_latency, 50);
    assert_int_equal(snap.avg_scheduling_latency, (10 + 50 + 30) / 3);
}

static void test_latency_no_samples(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);
    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);

    /* Should handle zero samples cleanly */
    assert_int_equal(snap.avg_scheduling_latency, 0);
    assert_int_equal(snap.min_scheduling_latency, 0);
    assert_int_equal(snap.max_scheduling_latency, 0);
}

/* -------------------------------------------------------------------------
 * Utilization Calculations
 * ---------------------------------------------------------------------- */

static void test_utilization_calculations(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);

    sched_stats_record_busy_time(&g_ctx, 200);
    sched_stats_record_idle_time(&g_ctx, 800);

    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);

    /* 200 out of 1000 = 20%. Basis points = 2000. */
    assert_int_equal(snap.cpu_utilization, 2000);
}

static void test_utilization_zero_time(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);

    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);

    assert_int_equal(snap.cpu_utilization, 0);
}

static void test_utilization_overflow_handling(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);

    /* Use very large numbers that could overflow 32-bit math */
    uint32_t large_busy = 2000000000;
    uint32_t large_idle = 2000000000;
    sched_stats_record_busy_time(&g_ctx, large_busy);
    sched_stats_record_idle_time(&g_ctx, large_idle);

    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);

    /* Total is 4,000,000,000. Busy is 50%, so 5000 bp. */
    assert_int_equal(snap.cpu_utilization, 5000);
}

/* -------------------------------------------------------------------------
 * Export and Invalid Args
 * ---------------------------------------------------------------------- */

static void test_export_success(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);
    sched_stats_record_busy_time(&g_ctx, 100);
    sched_stats_record_idle_time(&g_ctx, 100);
    sched_stats_record_invocation(&g_ctx);
    sched_stats_record_latency(&g_ctx, 5);

    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);

    char buffer[256];
    assert_int_equal(sched_stats_export(&snap, buffer, sizeof(buffer)), SCHED_OK);

    /* Very basic sanity check on JSON output */
    assert_non_null(strstr(buffer, "\"cpu_utilization_bp\":5000"));
    assert_non_null(strstr(buffer, "\"scheduler_invocation_count\":1"));
}

static void test_export_overflow(void **state)
{
    (void)state;
    sched_stats_data_t snap;
    memset(&snap, 0, sizeof(snap));

    char small_buf[10];
    assert_int_equal(sched_stats_export(&snap, small_buf, sizeof(small_buf)), SCHED_ERR_OVERFLOW);
}

static void test_invalid_args(void **state)
{
    (void)state;
    sched_stats_data_t snap;
    char buf[100];
    assert_int_equal(sched_stats_export(NULL, buf, sizeof(buf)), SCHED_ERR_PARAM);
    assert_int_equal(sched_stats_export(&snap, NULL, sizeof(buf)), SCHED_ERR_PARAM);
    assert_int_equal(sched_stats_export(&snap, buf, 0), SCHED_ERR_PARAM);

    assert_int_equal(sched_stats_snapshot(NULL, &snap), SCHED_ERR_PARAM);
    assert_int_equal(sched_stats_snapshot(&g_ctx, NULL), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Repeated Benchmark Runs & Stress
 * ---------------------------------------------------------------------- */

static void test_repeated_runs(void **state)
{
    (void)state;
    for (int run = 0; run < 3; run++)
    {
        sched_stats_reset(&g_ctx);
        sched_stats_start(&g_ctx, 0);
        sched_stats_record_invocation(&g_ctx);
        sched_stats_record_busy_time(&g_ctx, 100);
        sched_stats_record_latency(&g_ctx, 10);
        sched_stats_stop(&g_ctx, 100);

        sched_stats_data_t snap;
        sched_stats_snapshot(&g_ctx, &snap);
        assert_int_equal(snap.scheduler_invocation_count, 1);
        assert_int_equal(snap.cpu_utilization, 10000);  // 100% since idle was 0
        assert_int_equal(snap.avg_scheduling_latency, 10);
    }
}

static void test_stress_counter_accumulation(void **state)
{
    (void)state;
    sched_stats_start(&g_ctx, 0);

    /* Simulate 10000 fast context switches */
    for (int i = 0; i < 10000; i++)
    {
        sched_stats_record_context_switch(&g_ctx);
        sched_stats_record_latency(&g_ctx, i % 100);
    }

    sched_stats_data_t snap;
    sched_stats_snapshot(&g_ctx, &snap);
    assert_int_equal(snap.context_switch_count, 10000);
    assert_int_equal(snap._latency_samples, 10000);
    assert_int_equal(snap.min_scheduling_latency, 0);
    assert_int_equal(snap.max_scheduling_latency, 99);
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
        cmocka_unit_test_setup(test_start_stop, setup),
        cmocka_unit_test_setup(test_counter_updates, setup),
        cmocka_unit_test_setup(test_updates_when_inactive, setup),
        cmocka_unit_test_setup(test_latency_calculations, setup),
        cmocka_unit_test_setup(test_latency_no_samples, setup),
        cmocka_unit_test_setup(test_utilization_calculations, setup),
        cmocka_unit_test_setup(test_utilization_zero_time, setup),
        cmocka_unit_test_setup(test_utilization_overflow_handling, setup),
        cmocka_unit_test_setup(test_export_success, setup),
        cmocka_unit_test(test_export_overflow),
        cmocka_unit_test_setup(test_invalid_args, setup),
        cmocka_unit_test_setup(test_repeated_runs, setup),
        cmocka_unit_test_setup(test_stress_counter_accumulation, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
