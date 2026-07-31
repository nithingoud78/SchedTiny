/**
 * @file    test_rms.c
 * @brief   CMocka unit tests for SchedTiny RMS Scheduler.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_rms.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

/* -------------------------------------------------------------------------
 * Fixture
 * ---------------------------------------------------------------------- */

static sched_rms_t g_ctx;

static int setup(void **state)
{
    (void)state;
    sched_rms_init(&g_ctx);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialization & Reset
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_rms_t ctx;
    assert_int_equal(sched_rms_init(&ctx), SCHED_OK);
    assert_true(ctx.initialized);
    assert_false(sched_rms_has_ready_task(&ctx));
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_rms_init(NULL), SCHED_ERR_PARAM);
}

static void test_reset_clears_queue(void **state)
{
    (void)state;
    sched_rms_assign_period(&g_ctx, 1, 10);
    sched_rms_add_task(&g_ctx, 1);
    assert_true(sched_rms_has_ready_task(&g_ctx));

    assert_int_equal(sched_rms_reset(&g_ctx), SCHED_OK);
    assert_false(sched_rms_has_ready_task(&g_ctx));

    /* Registry should still be intact after reset */
    assert_int_equal(sched_rms_add_task(&g_ctx, 1), SCHED_OK);
    assert_true(sched_rms_has_ready_task(&g_ctx));
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_rms_t ctx;
    ctx.initialized = false;
    assert_int_equal(sched_rms_reset(&ctx), SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Period Assignment & Update
 * ---------------------------------------------------------------------- */

static void test_assign_period_success(void **state)
{
    (void)state;
    assert_int_equal(sched_rms_assign_period(&g_ctx, 1, 10), SCHED_OK);
    assert_int_equal(sched_rms_assign_period(&g_ctx, 2, 20), SCHED_OK);
}

static void test_assign_duplicate_updates(void **state)
{
    (void)state;
    assert_int_equal(sched_rms_assign_period(&g_ctx, 1, 10), SCHED_OK);
    assert_int_equal(sched_rms_assign_period(&g_ctx, 1, 30), SCHED_OK);
}

static void test_assign_overflow_registry(void **state)
{
    (void)state;
    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        assert_int_equal(sched_rms_assign_period(&g_ctx, i + 1, 10), SCHED_OK);
    }
    assert_int_equal(sched_rms_assign_period(&g_ctx, 99, 10), SCHED_ERR_OVERFLOW);
}

static void test_update_period_success(void **state)
{
    (void)state;
    sched_rms_assign_period(&g_ctx, 1, 10);
    assert_int_equal(sched_rms_update_period(&g_ctx, 1, 15), SCHED_OK);
}

static void test_update_period_not_found(void **state)
{
    (void)state;
    assert_int_equal(sched_rms_update_period(&g_ctx, 99, 15), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Task Insertion & Empty States
 * ---------------------------------------------------------------------- */

static void test_add_missing_period(void **state)
{
    (void)state;
    assert_int_equal(sched_rms_add_task(&g_ctx, 1), SCHED_ERR_NOT_FOUND);
}

static void test_add_duplicate_in_queue(void **state)
{
    (void)state;
    sched_rms_assign_period(&g_ctx, 1, 10);
    assert_int_equal(sched_rms_add_task(&g_ctx, 1), SCHED_OK);
    assert_int_equal(sched_rms_add_task(&g_ctx, 1), SCHED_ERR_STATE);
}

static void test_empty_queue_select(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_rms_select_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
    assert_int_equal(sched_rms_peek_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
    assert_false(sched_rms_has_ready_task(&g_ctx));
}

/* -------------------------------------------------------------------------
 * Ordering & Priorities (Rate Monotonic)
 * ---------------------------------------------------------------------- */

static void test_rms_ordering(void **state)
{
    (void)state;
    /* Longer period = lower priority. Shorter period = higher priority. */
    sched_rms_assign_period(&g_ctx, 1, 50);
    sched_rms_assign_period(&g_ctx, 2, 20);
    sched_rms_assign_period(&g_ctx, 3, 30);

    sched_rms_add_task(&g_ctx, 1);
    sched_rms_add_task(&g_ctx, 2);
    sched_rms_add_task(&g_ctx, 3);

    uint32_t id;
    assert_int_equal(sched_rms_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 2); /* Period 20 (shortest) */

    assert_int_equal(sched_rms_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 3); /* Period 30 */

    assert_int_equal(sched_rms_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 1); /* Period 50 (longest) */
}

static void test_equal_periods_fifo(void **state)
{
    (void)state;
    sched_rms_assign_period(&g_ctx, 1, 20);
    sched_rms_assign_period(&g_ctx, 2, 20);
    sched_rms_assign_period(&g_ctx, 3, 20);

    sched_rms_add_task(&g_ctx, 1);
    sched_rms_add_task(&g_ctx, 2);
    sched_rms_add_task(&g_ctx, 3);

    uint32_t id;
    sched_rms_select_next(&g_ctx, &id);
    assert_int_equal(id, 1);
    sched_rms_select_next(&g_ctx, &id);
    assert_int_equal(id, 2);
    sched_rms_select_next(&g_ctx, &id);
    assert_int_equal(id, 3);
}

/* -------------------------------------------------------------------------
 * Task Removal & Maintenance
 * ---------------------------------------------------------------------- */

static void test_remove_middle(void **state)
{
    (void)state;
    sched_rms_assign_period(&g_ctx, 1, 10);
    sched_rms_assign_period(&g_ctx, 2, 20);
    sched_rms_assign_period(&g_ctx, 3, 30);

    sched_rms_add_task(&g_ctx, 1);
    sched_rms_add_task(&g_ctx, 2);
    sched_rms_add_task(&g_ctx, 3);

    assert_int_equal(sched_rms_remove_task(&g_ctx, 2), SCHED_OK);

    uint32_t id;
    sched_rms_select_next(&g_ctx, &id);
    assert_int_equal(id, 1);
    sched_rms_select_next(&g_ctx, &id);
    assert_int_equal(id, 3);
}

static void test_remove_missing(void **state)
{
    (void)state;
    sched_rms_assign_period(&g_ctx, 1, 10);
    sched_rms_add_task(&g_ctx, 1);

    assert_int_equal(sched_rms_remove_task(&g_ctx, 99), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Stress and Boundary
 * ---------------------------------------------------------------------- */

static void test_fill_queue_to_capacity(void **state)
{
    (void)state;

    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        sched_rms_assign_period(&g_ctx, i + 1, (SCHED_RMS_CAPACITY - i) * 10);
        assert_int_equal(sched_rms_add_task(&g_ctx, i + 1), SCHED_OK);
    }

    /* Queue is full */
    sched_rms_assign_period(&g_ctx, 99, 5);
    assert_int_equal(sched_rms_add_task(&g_ctx, 99), SCHED_ERR_OVERFLOW);

    /* Should dequeue in order of period (shortest first): task 8 (period 10), 7 (20), etc. */
    uint32_t id;
    for (uint32_t i = 0; i < SCHED_RMS_CAPACITY; i++)
    {
        assert_int_equal(sched_rms_select_next(&g_ctx, &id), SCHED_OK);
        assert_int_equal(id, SCHED_RMS_CAPACITY - i);
    }
}

/* -------------------------------------------------------------------------
 * Invalid Input
 * ---------------------------------------------------------------------- */

static void test_invalid_args(void **state)
{
    (void)state;
    uint32_t id;

    assert_int_equal(sched_rms_assign_period(NULL, 1, 10), SCHED_ERR_PARAM);
    assert_int_equal(sched_rms_assign_period(&g_ctx, 1, 0), SCHED_ERR_PARAM); /* period 0 invalid */
    assert_int_equal(sched_rms_update_period(NULL, 1, 10), SCHED_ERR_PARAM);
    assert_int_equal(sched_rms_add_task(NULL, 1), SCHED_ERR_PARAM);
    assert_int_equal(sched_rms_remove_task(NULL, 1), SCHED_ERR_PARAM);
    assert_false(sched_rms_has_ready_task(NULL));
    assert_int_equal(sched_rms_peek_next(NULL, &id), SCHED_ERR_PARAM);
    assert_int_equal(sched_rms_peek_next(&g_ctx, NULL), SCHED_ERR_PARAM);
    assert_int_equal(sched_rms_select_next(NULL, &id), SCHED_ERR_PARAM);
    assert_int_equal(sched_rms_dispatch_candidate(NULL, &id), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* Init */
        cmocka_unit_test(test_init_success),
        cmocka_unit_test(test_init_null),
        cmocka_unit_test_setup(test_reset_clears_queue, setup),
        cmocka_unit_test(test_reset_uninit),
        /* Periods */
        cmocka_unit_test_setup(test_assign_period_success, setup),
        cmocka_unit_test_setup(test_assign_duplicate_updates, setup),
        cmocka_unit_test_setup(test_assign_overflow_registry, setup),
        cmocka_unit_test_setup(test_update_period_success, setup),
        cmocka_unit_test_setup(test_update_period_not_found, setup),
        /* Tasks */
        cmocka_unit_test_setup(test_add_missing_period, setup),
        cmocka_unit_test_setup(test_add_duplicate_in_queue, setup),
        cmocka_unit_test_setup(test_empty_queue_select, setup),
        /* Ordering */
        cmocka_unit_test_setup(test_rms_ordering, setup),
        cmocka_unit_test_setup(test_equal_periods_fifo, setup),
        /* Removal */
        cmocka_unit_test_setup(test_remove_middle, setup),
        cmocka_unit_test_setup(test_remove_missing, setup),
        /* Stress / Boundary */
        cmocka_unit_test_setup(test_fill_queue_to_capacity, setup),
        /* Invalid */
        cmocka_unit_test_setup(test_invalid_args, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
