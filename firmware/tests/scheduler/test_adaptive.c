/**
 * @file    test_adaptive.c
 * @brief   CMocka unit tests for SchedTiny Adaptive Scheduling Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

#include "sched_adaptive.h"
#include "sched_benchmark.h"

#include <string.h>

/* -------------------------------------------------------------------------
 * Fixture
 * ---------------------------------------------------------------------- */

static sched_adaptive_t g_adaptive;
static sched_benchmark_t g_bench;

static int setup(void **state)
{
    (void)state;
    sched_adaptive_init(&g_adaptive);
    sched_benchmark_init(&g_bench);
    return 0;
}

static int teardown(void **state)
{
    (void)state;
    sched_adaptive_reset(&g_adaptive);
    sched_benchmark_reset(&g_bench);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialisation & Reset
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_adaptive_t ctx;
    assert_int_equal(sched_adaptive_init(&ctx), SCHED_OK);
    assert_true(ctx.initialized);
    assert_int_equal(ctx.active_policy, SCHED_BENCHMARK_POLICY_HPF);
    assert_int_equal(ctx.config.engine, SCHED_ADAPTIVE_ENGINE_RULE_BASED);
    assert_int_equal(ctx.config.eval_interval, SCHED_ADAPTIVE_DEFAULT_INTERVAL);
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_adaptive_init(NULL), SCHED_ERR_PARAM);
}

static void test_reset_success(void **state)
{
    (void)state;
    sched_adaptive_t ctx;
    sched_adaptive_init(&ctx);
    ctx.stats.total_decisions = 42;
    ctx.active_policy         = SCHED_BENCHMARK_POLICY_EDF;

    assert_int_equal(sched_adaptive_reset(&ctx), SCHED_OK);
    assert_int_equal(ctx.stats.total_decisions, 0);
    assert_int_equal(ctx.active_policy, SCHED_BENCHMARK_POLICY_HPF);
    assert_true(ctx.initialized);
}

static void test_reset_null(void **state)
{
    (void)state;
    assert_int_equal(sched_adaptive_reset(NULL), SCHED_ERR_PARAM);
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_adaptive_t ctx;
    memset(&ctx, 0, sizeof(ctx));
    ctx.initialized = false;
    assert_int_equal(sched_adaptive_reset(&ctx), SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Configuration
 * ---------------------------------------------------------------------- */

static void test_set_config(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_DECISION_TREE,
                                   .eval_interval         = 50,
                                   .hysteresis_bp         = 1000,
                                   .max_switches_per_1000 = 5};

    assert_int_equal(sched_adaptive_set_config(&g_adaptive, &cfg), SCHED_OK);
    assert_int_equal(g_adaptive.config.engine, SCHED_ADAPTIVE_ENGINE_DECISION_TREE);
    assert_int_equal(g_adaptive.config.eval_interval, 50);
    assert_int_equal(g_adaptive.config.hysteresis_bp, 1000);
}

static void test_set_config_null(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {0};
    assert_int_equal(sched_adaptive_set_config(NULL, &cfg), SCHED_ERR_PARAM);
    assert_int_equal(sched_adaptive_set_config(&g_adaptive, NULL), SCHED_ERR_PARAM);
}

static void test_set_config_zero_interval(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_RULE_BASED,
                                   .eval_interval         = 0,
                                   .hysteresis_bp         = 500,
                                   .max_switches_per_1000 = 10};

    assert_int_equal(sched_adaptive_set_config(&g_adaptive, &cfg), SCHED_OK);
    /* Zero interval should be clamped to 1 */
    assert_int_equal(g_adaptive.config.eval_interval, 1);
}

/* -------------------------------------------------------------------------
 * Feature Extraction
 * ---------------------------------------------------------------------- */

static void test_extract_features_null(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    assert_int_equal(sched_adaptive_extract_features(NULL, SCHED_BENCHMARK_POLICY_HPF, &f),
                     SCHED_ERR_PARAM);
    assert_int_equal(sched_adaptive_extract_features(&g_bench, SCHED_BENCHMARK_POLICY_HPF, NULL),
                     SCHED_ERR_PARAM);
}

static void test_extract_features_from_bench(void **state)
{
    (void)state;
    sched_benchmark_load_workload(&g_bench, SCHED_BENCHMARK_PROFILE_SMALL, 42);
    sched_benchmark_run(&g_bench, SCHED_BENCHMARK_POLICY_HPF, 500);

    sched_adaptive_features_t f;
    assert_int_equal(sched_adaptive_extract_features(&g_bench, SCHED_BENCHMARK_POLICY_HPF, &f),
                     SCHED_OK);

    /* Should have extracted meaningful values */
    assert_int_equal(f.task_count, 10);
    assert_true(f.cpu_utilization_bp > 0);
}

/* -------------------------------------------------------------------------
 * Rule-Based Engine
 * ---------------------------------------------------------------------- */

static void test_rule_engine_high_util_edf(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 9000; /* Very high utilization */
    f.deadline_miss_rate_bp   = 1000; /* Significant deadline misses */
    f.hi_criticality_ratio_bp = 500;  /* Few HI-crit tasks */
    f.task_count              = 10;

    sched_adaptive_decision_t d;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_EDF);
}

static void test_rule_engine_hi_crit_mc(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 6000;
    f.deadline_miss_rate_bp   = 0;
    f.hi_criticality_ratio_bp = 5000; /* Many HI-crit tasks */
    f.task_count              = 10;

    sched_adaptive_decision_t d;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_MC);
}

static void test_rule_engine_low_util_rms(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 3000; /* Low utilization */
    f.deadline_miss_rate_bp   = 0;
    f.hi_criticality_ratio_bp = 0;    /* No HI-crit tasks */
    f.task_count              = 10;

    sched_adaptive_decision_t d;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_RMS);
}

/* -------------------------------------------------------------------------
 * Decision Tree Engine
 * ---------------------------------------------------------------------- */

static void test_tree_engine_high_util(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_DECISION_TREE,
                                   .eval_interval         = 100,
                                   .hysteresis_bp         = 500,
                                   .max_switches_per_1000 = 10};
    sched_adaptive_set_config(&g_adaptive, &cfg);

    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 8000;
    f.deadline_miss_rate_bp   = 600;
    f.hi_criticality_ratio_bp = 5000;
    f.task_count              = 10;

    sched_adaptive_decision_t d;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_MC);
}

static void test_tree_engine_low_util(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_DECISION_TREE,
                                   .eval_interval         = 100,
                                   .hysteresis_bp         = 500,
                                   .max_switches_per_1000 = 10};
    sched_adaptive_set_config(&g_adaptive, &cfg);

    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 2000;
    f.deadline_miss_rate_bp   = 0;
    f.hi_criticality_ratio_bp = 0;
    f.task_count              = 8;

    sched_adaptive_decision_t d;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_RMS);
}

/* -------------------------------------------------------------------------
 * Lookup Table Engine
 * ---------------------------------------------------------------------- */

static void test_lookup_engine(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_LOOKUP_TABLE,
                                   .eval_interval         = 100,
                                   .hysteresis_bp         = 500,
                                   .max_switches_per_1000 = 10};
    sched_adaptive_set_config(&g_adaptive, &cfg);

    /* Low utilization, high criticality → MC */
    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 1000;
    f.hi_criticality_ratio_bp = 8000;

    sched_adaptive_decision_t d;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_MC);

    /* Low utilization, low criticality → RMS */
    f.hi_criticality_ratio_bp = 500;
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, &d), SCHED_OK);
    assert_int_equal(d.recommended, SCHED_BENCHMARK_POLICY_RMS);
}

/* -------------------------------------------------------------------------
 * Tick & Evaluation Interval
 * ---------------------------------------------------------------------- */

static void test_tick_interval(void **state)
{
    (void)state;
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_RULE_BASED,
                                   .eval_interval         = 10,
                                   .hysteresis_bp         = 500,
                                   .max_switches_per_1000 = 10};
    sched_adaptive_set_config(&g_adaptive, &cfg);

    /* First 9 ticks should not trigger evaluation */
    for (int i = 0; i < 9; i++)
    {
        assert_false(sched_adaptive_tick(&g_adaptive));
    }

    /* 10th tick should trigger */
    assert_true(sched_adaptive_tick(&g_adaptive));

    /* Should reset and count again */
    for (int i = 0; i < 9; i++)
    {
        assert_false(sched_adaptive_tick(&g_adaptive));
    }
    assert_true(sched_adaptive_tick(&g_adaptive));
}

static void test_tick_null(void **state)
{
    (void)state;
    assert_false(sched_adaptive_tick(NULL));
}

/* -------------------------------------------------------------------------
 * Hysteresis
 * ---------------------------------------------------------------------- */

static void test_hysteresis_suppresses_thrashing(void **state)
{
    (void)state;
    /* Set a high hysteresis threshold */
    sched_adaptive_config_t cfg = {.engine                = SCHED_ADAPTIVE_ENGINE_RULE_BASED,
                                   .eval_interval         = 1,    /* Evaluate every tick */
                                   .hysteresis_bp         = 9000, /* Very high threshold */
                                   .max_switches_per_1000 = 10};
    sched_adaptive_set_config(&g_adaptive, &cfg);

    /* Active policy starts as HPF */
    assert_int_equal(sched_adaptive_get_active_policy(&g_adaptive), SCHED_BENCHMARK_POLICY_HPF);

    /* Even with features favouring another policy, high hysteresis prevents switch */
    sched_benchmark_load_workload(&g_bench, SCHED_BENCHMARK_PROFILE_SMALL, 42);
    sched_benchmark_run(&g_bench, SCHED_BENCHMARK_POLICY_HPF, 500);

    sched_adaptive_select_policy(&g_adaptive, &g_bench, SCHED_BENCHMARK_POLICY_HPF);

    /* Due to high hysteresis, the policy should NOT have switched */
    /* (The score delta needs to exceed 9000bp to switch) */
    sched_adaptive_stats_t stats;
    sched_adaptive_get_stats(&g_adaptive, &stats);
    assert_int_equal(stats.total_decisions, 1);
}

/* -------------------------------------------------------------------------
 * Statistics
 * ---------------------------------------------------------------------- */

static void test_stats_tracking(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp = 5000;
    f.task_count         = 10;

    sched_adaptive_decision_t d;

    /* Make multiple evaluations */
    for (int i = 0; i < 5; i++)
    {
        sched_adaptive_evaluate(&g_adaptive, &f, &d);
    }

    sched_adaptive_stats_t stats;
    assert_int_equal(sched_adaptive_get_stats(&g_adaptive, &stats), SCHED_OK);
    assert_int_equal(stats.total_decisions, 5);
}

static void test_stats_null(void **state)
{
    (void)state;
    sched_adaptive_stats_t stats;
    assert_int_equal(sched_adaptive_get_stats(NULL, &stats), SCHED_ERR_PARAM);
    assert_int_equal(sched_adaptive_get_stats(&g_adaptive, NULL), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Active Policy Accessor
 * ---------------------------------------------------------------------- */

static void test_get_active_policy_default(void **state)
{
    (void)state;
    assert_int_equal(sched_adaptive_get_active_policy(&g_adaptive), SCHED_BENCHMARK_POLICY_HPF);
}

static void test_get_active_policy_null(void **state)
{
    (void)state;
    assert_int_equal(sched_adaptive_get_active_policy(NULL), SCHED_BENCHMARK_POLICY_HPF);
}

/* -------------------------------------------------------------------------
 * Evaluate NULL Parameters
 * ---------------------------------------------------------------------- */

static void test_evaluate_null(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    sched_adaptive_decision_t d;
    memset(&f, 0, sizeof(f));

    assert_int_equal(sched_adaptive_evaluate(NULL, &f, &d), SCHED_ERR_PARAM);
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, NULL, &d), SCHED_ERR_PARAM);
    assert_int_equal(sched_adaptive_evaluate(&g_adaptive, &f, NULL), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Determinism
 * ---------------------------------------------------------------------- */

static void test_determinism(void **state)
{
    (void)state;
    sched_adaptive_features_t f;
    memset(&f, 0, sizeof(f));
    f.cpu_utilization_bp      = 7000;
    f.deadline_miss_rate_bp   = 300;
    f.hi_criticality_ratio_bp = 2000;
    f.task_count              = 15;

    sched_adaptive_decision_t d1, d2;

    /* First run */
    sched_adaptive_t ctx1;
    sched_adaptive_init(&ctx1);
    sched_adaptive_evaluate(&ctx1, &f, &d1);

    /* Second run with fresh context */
    sched_adaptive_t ctx2;
    sched_adaptive_init(&ctx2);
    sched_adaptive_evaluate(&ctx2, &f, &d2);

    /* Must produce identical results */
    assert_int_equal(d1.recommended, d2.recommended);
    assert_int_equal(d1.confidence_bp, d2.confidence_bp);
    for (int i = 0; i < SCHED_ADAPTIVE_NUM_POLICIES; i++)
    {
        assert_int_equal(d1.scores[i], d2.scores[i]);
    }
}

/* -------------------------------------------------------------------------
 * Benchmark Integration
 * ---------------------------------------------------------------------- */

static void test_adaptive_benchmark_run(void **state)
{
    (void)state;
    sched_benchmark_t bench;
    sched_benchmark_init(&bench);
    sched_benchmark_load_workload(&bench, SCHED_BENCHMARK_PROFILE_SMALL, 42);

    SchedStatus_t st = sched_benchmark_run(&bench, SCHED_BENCHMARK_POLICY_ADAPTIVE, 500);
    assert_int_equal(st, SCHED_OK);
    assert_true(bench.has_results[SCHED_BENCHMARK_POLICY_ADAPTIVE]);

    sched_benchmark_results_t res;
    st = sched_benchmark_get_results(&bench, SCHED_BENCHMARK_POLICY_ADAPTIVE, &res);
    assert_int_equal(st, SCHED_OK);
    assert_int_equal(res.total_tasks, 10);
    assert_true(res.adaptive_decisions > 0);
}

/* =========================================================================
 * Test Runner
 * ========================================================================= */

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* Init & Reset */
        cmocka_unit_test(test_init_success),
        cmocka_unit_test(test_init_null),
        cmocka_unit_test(test_reset_success),
        cmocka_unit_test(test_reset_null),
        cmocka_unit_test(test_reset_uninit),

        /* Config */
        cmocka_unit_test_setup_teardown(test_set_config, setup, teardown),
        cmocka_unit_test_setup_teardown(test_set_config_null, setup, teardown),
        cmocka_unit_test_setup_teardown(test_set_config_zero_interval, setup, teardown),

        /* Feature Extraction */
        cmocka_unit_test_setup_teardown(test_extract_features_null, setup, teardown),
        cmocka_unit_test_setup_teardown(test_extract_features_from_bench, setup, teardown),

        /* Rule Engine */
        cmocka_unit_test_setup_teardown(test_rule_engine_high_util_edf, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rule_engine_hi_crit_mc, setup, teardown),
        cmocka_unit_test_setup_teardown(test_rule_engine_low_util_rms, setup, teardown),

        /* Decision Tree */
        cmocka_unit_test_setup_teardown(test_tree_engine_high_util, setup, teardown),
        cmocka_unit_test_setup_teardown(test_tree_engine_low_util, setup, teardown),

        /* Lookup Table */
        cmocka_unit_test_setup_teardown(test_lookup_engine, setup, teardown),

        /* Tick & Interval */
        cmocka_unit_test_setup_teardown(test_tick_interval, setup, teardown),
        cmocka_unit_test(test_tick_null),

        /* Hysteresis */
        cmocka_unit_test_setup_teardown(test_hysteresis_suppresses_thrashing, setup, teardown),

        /* Statistics */
        cmocka_unit_test_setup_teardown(test_stats_tracking, setup, teardown),
        cmocka_unit_test_setup_teardown(test_stats_null, setup, teardown),

        /* Accessors */
        cmocka_unit_test_setup_teardown(test_get_active_policy_default, setup, teardown),
        cmocka_unit_test(test_get_active_policy_null),

        /* NULL Parameters */
        cmocka_unit_test_setup_teardown(test_evaluate_null, setup, teardown),

        /* Determinism */
        cmocka_unit_test(test_determinism),

        /* Benchmark Integration */
        cmocka_unit_test(test_adaptive_benchmark_run),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
