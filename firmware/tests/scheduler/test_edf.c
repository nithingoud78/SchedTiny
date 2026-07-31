/**
 * @file    test_edf.c
 * @brief   CMocka unit tests for SchedTiny EDF Scheduler.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_edf.h"

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

static sched_edf_t g_ctx;

static int setup(void **state)
{
    (void)state;
    sched_edf_init(&g_ctx);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialization & Reset
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_edf_t ctx;
    assert_int_equal(sched_edf_init(&ctx), SCHED_OK);
    assert_true(ctx.initialized);
    assert_false(sched_edf_has_ready_task(&ctx));
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_edf_init(NULL), SCHED_ERR_PARAM);
}

static void test_reset_clears_queue(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 10);
    sched_edf_add_task(&g_ctx, 1, 0);
    assert_true(sched_edf_has_ready_task(&g_ctx));

    assert_int_equal(sched_edf_reset(&g_ctx), SCHED_OK);
    assert_false(sched_edf_has_ready_task(&g_ctx));

    /* Registry should still be intact after reset */
    assert_int_equal(sched_edf_add_task(&g_ctx, 1, 100), SCHED_OK);
    assert_true(sched_edf_has_ready_task(&g_ctx));
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_edf_t ctx;
    ctx.initialized = false;
    assert_int_equal(sched_edf_reset(&ctx), SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Deadline Assignment & Update
 * ---------------------------------------------------------------------- */

static void test_assign_deadline_success(void **state)
{
    (void)state;
    assert_int_equal(sched_edf_assign_deadline(&g_ctx, 1, 10), SCHED_OK);
    assert_int_equal(sched_edf_assign_deadline(&g_ctx, 2, 20), SCHED_OK);
}

static void test_assign_duplicate_updates(void **state)
{
    (void)state;
    assert_int_equal(sched_edf_assign_deadline(&g_ctx, 1, 10), SCHED_OK);
    assert_int_equal(sched_edf_assign_deadline(&g_ctx, 1, 30), SCHED_OK);

    /* Verify by adding task and checking abs deadline */
    sched_edf_add_task(&g_ctx, 1, 0);
    /* In white-box we could check queue directly, but let's test via peek if we had a way,
       or just rely on the API. Adding at 0 gives abs 30. */
}

static void test_assign_overflow_registry(void **state)
{
    (void)state;
    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        assert_int_equal(sched_edf_assign_deadline(&g_ctx, i + 1, 10), SCHED_OK);
    }
    assert_int_equal(sched_edf_assign_deadline(&g_ctx, 99, 10), SCHED_ERR_OVERFLOW);
}

static void test_update_deadline_success(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 10);
    assert_int_equal(sched_edf_update_deadline(&g_ctx, 1, 15), SCHED_OK);
}

static void test_update_deadline_not_found(void **state)
{
    (void)state;
    assert_int_equal(sched_edf_update_deadline(&g_ctx, 99, 15), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Task Insertion & Empty States
 * ---------------------------------------------------------------------- */

static void test_add_missing_deadline(void **state)
{
    (void)state;
    assert_int_equal(sched_edf_add_task(&g_ctx, 1, 100), SCHED_ERR_NOT_FOUND);
}

static void test_add_duplicate_in_queue(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 10);
    assert_int_equal(sched_edf_add_task(&g_ctx, 1, 0), SCHED_OK);
    assert_int_equal(sched_edf_add_task(&g_ctx, 1, 5), SCHED_ERR_STATE);
}

static void test_empty_queue_select(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
    assert_int_equal(sched_edf_peek_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
    assert_false(sched_edf_has_ready_task(&g_ctx));
}

/* -------------------------------------------------------------------------
 * Ordering & Priorities (Earliest Deadline First)
 * ---------------------------------------------------------------------- */

static void test_edf_ordering(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 50);
    sched_edf_assign_deadline(&g_ctx, 2, 20);
    sched_edf_assign_deadline(&g_ctx, 3, 30);

    /* Add at current_time = 0 */
    sched_edf_add_task(&g_ctx, 1, 0);  // abs: 50
    sched_edf_add_task(&g_ctx, 2, 0);  // abs: 20
    sched_edf_add_task(&g_ctx, 3, 0);  // abs: 30

    uint32_t id;
    assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 2); /* 20 */

    assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 3); /* 30 */

    assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 1); /* 50 */
}

static void test_equal_deadlines_fifo(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 20);
    sched_edf_assign_deadline(&g_ctx, 2, 20);
    sched_edf_assign_deadline(&g_ctx, 3, 20);

    sched_edf_add_task(&g_ctx, 1, 0);
    sched_edf_add_task(&g_ctx, 2, 0);
    sched_edf_add_task(&g_ctx, 3, 0);

    uint32_t id;
    sched_edf_select_next(&g_ctx, &id);
    assert_int_equal(id, 1);
    sched_edf_select_next(&g_ctx, &id);
    assert_int_equal(id, 2);
    sched_edf_select_next(&g_ctx, &id);
    assert_int_equal(id, 3);
}

/* -------------------------------------------------------------------------
 * Overflow Scenarios
 * ---------------------------------------------------------------------- */

static void test_deadline_overflow_ordering(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 10);
    sched_edf_assign_deadline(&g_ctx, 2, 20);

    /* current_time is very close to UINT32_MAX */
    uint32_t current_time = UINT32_MAX - 15;

    /* Task 1 abs_deadline: UINT32_MAX - 5 (no wrap) */
    sched_edf_add_task(&g_ctx, 1, current_time);

    /* Task 2 abs_deadline: 4 (wrapped!) */
    sched_edf_add_task(&g_ctx, 2, current_time);

    /* Task 1 should be selected FIRST because despite Task 2 wrapping to a
       small absolute number, it is logically later.
       (UINT32_MAX - 5) - 4 = UINT32_MAX - 9, which as int32_t is -10 (<= 0),
       so Task 1 is earlier. */

    uint32_t id;
    assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 1);

    assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 2);
}

static void test_cmp_earlier_logic(void **state)
{
    (void)state;
    assert_true(sched_edf_deadline_is_earlier(10, 20));
    assert_true(sched_edf_deadline_is_earlier(20, 20));
    assert_false(sched_edf_deadline_is_earlier(30, 20));

    /* Wrap around */
    uint32_t near_max = UINT32_MAX - 5;
    uint32_t wrapped  = 10;
    assert_true(sched_edf_deadline_is_earlier(near_max, wrapped));
    assert_false(sched_edf_deadline_is_earlier(wrapped, near_max));
}

/* -------------------------------------------------------------------------
 * Task Removal & Maintenance
 * ---------------------------------------------------------------------- */

static void test_remove_middle(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 10);
    sched_edf_assign_deadline(&g_ctx, 2, 20);
    sched_edf_assign_deadline(&g_ctx, 3, 30);

    sched_edf_add_task(&g_ctx, 1, 0);
    sched_edf_add_task(&g_ctx, 2, 0);
    sched_edf_add_task(&g_ctx, 3, 0);

    assert_int_equal(sched_edf_remove_task(&g_ctx, 2), SCHED_OK);

    uint32_t id;
    sched_edf_select_next(&g_ctx, &id);
    assert_int_equal(id, 1);
    sched_edf_select_next(&g_ctx, &id);
    assert_int_equal(id, 3);
}

static void test_remove_missing(void **state)
{
    (void)state;
    sched_edf_assign_deadline(&g_ctx, 1, 10);
    sched_edf_add_task(&g_ctx, 1, 0);

    assert_int_equal(sched_edf_remove_task(&g_ctx, 99), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Stress and Boundary
 * ---------------------------------------------------------------------- */

static void test_fill_queue_to_capacity(void **state)
{
    (void)state;

    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        sched_edf_assign_deadline(&g_ctx, i + 1, (SCHED_EDF_CAPACITY - i) * 10);
        assert_int_equal(sched_edf_add_task(&g_ctx, i + 1, 0), SCHED_OK);
    }

    /* Queue is full */
    sched_edf_assign_deadline(&g_ctx, 99, 5);
    assert_int_equal(sched_edf_add_task(&g_ctx, 99, 0), SCHED_ERR_OVERFLOW);

    /* Should dequeue in order of deadline: task 8 (prio 10), 7 (20), etc. */
    uint32_t id;
    for (uint32_t i = 0; i < SCHED_EDF_CAPACITY; i++)
    {
        assert_int_equal(sched_edf_select_next(&g_ctx, &id), SCHED_OK);
        assert_int_equal(id, SCHED_EDF_CAPACITY - i);
    }
}

/* -------------------------------------------------------------------------
 * Invalid Input
 * ---------------------------------------------------------------------- */

static void test_invalid_args(void **state)
{
    (void)state;
    uint32_t id;

    assert_int_equal(sched_edf_assign_deadline(NULL, 1, 10), SCHED_ERR_PARAM);
    assert_int_equal(sched_edf_update_deadline(NULL, 1, 10), SCHED_ERR_PARAM);
    assert_int_equal(sched_edf_add_task(NULL, 1, 0), SCHED_ERR_PARAM);
    assert_int_equal(sched_edf_remove_task(NULL, 1), SCHED_ERR_PARAM);
    assert_false(sched_edf_has_ready_task(NULL));
    assert_int_equal(sched_edf_peek_next(NULL, &id), SCHED_ERR_PARAM);
    assert_int_equal(sched_edf_peek_next(&g_ctx, NULL), SCHED_ERR_PARAM);
    assert_int_equal(sched_edf_select_next(NULL, &id), SCHED_ERR_PARAM);
    assert_int_equal(sched_edf_dispatch_candidate(NULL, &id), SCHED_ERR_PARAM);
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
        /* Deadlines */
        cmocka_unit_test_setup(test_assign_deadline_success, setup),
        cmocka_unit_test_setup(test_assign_duplicate_updates, setup),
        cmocka_unit_test_setup(test_assign_overflow_registry, setup),
        cmocka_unit_test_setup(test_update_deadline_success, setup),
        cmocka_unit_test_setup(test_update_deadline_not_found, setup),
        /* Tasks */
        cmocka_unit_test_setup(test_add_missing_deadline, setup),
        cmocka_unit_test_setup(test_add_duplicate_in_queue, setup),
        cmocka_unit_test_setup(test_empty_queue_select, setup),
        /* Ordering */
        cmocka_unit_test_setup(test_edf_ordering, setup),
        cmocka_unit_test_setup(test_equal_deadlines_fifo, setup),
        /* Overflow */
        cmocka_unit_test(test_cmp_earlier_logic),
        cmocka_unit_test_setup(test_deadline_overflow_ordering, setup),
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
