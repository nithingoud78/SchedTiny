/**
 * @file    test_scheduler_policy.c
 * @brief   CMocka unit tests for SchedTiny HPF Scheduling Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_policy.h"

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

static sched_policy_t g_ctx;

static int setup(void **state)
{
    (void)state;
    sched_policy_init(&g_ctx);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialization tests
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_policy_t ctx;
    assert_int_equal(sched_policy_init(&ctx), SCHED_OK);
    assert_int_equal(ctx.state, SCHED_POLICY_READY);
    assert_false(sched_policy_has_ready_task(&ctx));
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_init(NULL), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Empty scheduler tests
 * ---------------------------------------------------------------------- */

static void test_empty_no_ready_task(void **state)
{
    (void)state;
    assert_false(sched_policy_has_ready_task(&g_ctx));
}

static void test_empty_select_returns_not_found(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
}

static void test_empty_peek_returns_not_found(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_policy_peek_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
}

static void test_empty_dispatch_returns_not_found(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_policy_dispatch_candidate(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Single task tests
 * ---------------------------------------------------------------------- */

static void test_one_task_select(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_add_task(&g_ctx, 1, 10), SCHED_OK);
    assert_true(sched_policy_has_ready_task(&g_ctx));

    uint32_t id;
    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 1);

    assert_false(sched_policy_has_ready_task(&g_ctx));
}

static void test_one_task_peek_does_not_dequeue(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 5, 7);

    uint32_t id;
    assert_int_equal(sched_policy_peek_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 5);

    /* Still there */
    assert_true(sched_policy_has_ready_task(&g_ctx));

    /* Peek again — same result */
    assert_int_equal(sched_policy_peek_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 5);
}

/* -------------------------------------------------------------------------
 * Multiple priority tests
 * ---------------------------------------------------------------------- */

static void test_multi_priority_selects_highest(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 1, 3);
    sched_policy_add_task(&g_ctx, 2, 10);
    sched_policy_add_task(&g_ctx, 3, 7);

    uint32_t id;
    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 2); /* priority 10 */

    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 3); /* priority 7 */

    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 1); /* priority 3 */

    assert_false(sched_policy_has_ready_task(&g_ctx));
}

static void test_peek_shows_highest_after_dequeue(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 10, 20);
    sched_policy_add_task(&g_ctx, 20, 15);

    uint32_t id;
    sched_policy_peek_next(&g_ctx, &id);
    assert_int_equal(id, 10);

    sched_policy_select_next(&g_ctx, &id);
    assert_int_equal(id, 10);

    sched_policy_peek_next(&g_ctx, &id);
    assert_int_equal(id, 20);
}

/* -------------------------------------------------------------------------
 * Equal priority (FIFO) tests
 * ---------------------------------------------------------------------- */

static void test_equal_priority_fifo(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 1, 5);
    sched_policy_add_task(&g_ctx, 2, 5);
    sched_policy_add_task(&g_ctx, 3, 5);

    uint32_t id;
    sched_policy_select_next(&g_ctx, &id);
    assert_int_equal(id, 1);
    sched_policy_select_next(&g_ctx, &id);
    assert_int_equal(id, 2);
    sched_policy_select_next(&g_ctx, &id);
    assert_int_equal(id, 3);
}

/* -------------------------------------------------------------------------
 * Reset tests
 * ---------------------------------------------------------------------- */

static void test_reset_clears_queue(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 1, 10);
    sched_policy_add_task(&g_ctx, 2, 20);
    assert_true(sched_policy_has_ready_task(&g_ctx));

    assert_int_equal(sched_policy_reset(&g_ctx), SCHED_OK);
    assert_false(sched_policy_has_ready_task(&g_ctx));
}

static void test_reset_null(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_reset(NULL), SCHED_ERR_PARAM);
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_policy_t ctx;
    ctx.state = SCHED_POLICY_UNINIT;
    assert_int_equal(sched_policy_reset(&ctx), SCHED_ERR_STATE);
}

static void test_reset_then_reuse(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 1, 5);
    sched_policy_reset(&g_ctx);

    sched_policy_add_task(&g_ctx, 2, 8);
    uint32_t id;
    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 2);
}

/* -------------------------------------------------------------------------
 * Repeated selection tests
 * ---------------------------------------------------------------------- */

static void test_repeated_select_drains(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 1, 1);
    sched_policy_add_task(&g_ctx, 2, 2);

    uint32_t id;
    sched_policy_select_next(&g_ctx, &id);
    sched_policy_select_next(&g_ctx, &id);

    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Invalid input tests
 * ---------------------------------------------------------------------- */

static void test_select_null_ctx(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_policy_select_next(NULL, &id), SCHED_ERR_PARAM);
}

static void test_select_null_out(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_select_next(&g_ctx, NULL), SCHED_ERR_PARAM);
}

static void test_peek_null_ctx(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_policy_peek_next(NULL, &id), SCHED_ERR_PARAM);
}

static void test_peek_null_out(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_peek_next(&g_ctx, NULL), SCHED_ERR_PARAM);
}

static void test_add_null_ctx(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_add_task(NULL, 1, 5), SCHED_ERR_PARAM);
}

static void test_remove_null_ctx(void **state)
{
    (void)state;
    assert_int_equal(sched_policy_remove_task(NULL, 1), SCHED_ERR_PARAM);
}

static void test_has_ready_null(void **state)
{
    (void)state;
    assert_false(sched_policy_has_ready_task(NULL));
}

static void test_dispatch_null_ctx(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_policy_dispatch_candidate(NULL, &id), SCHED_ERR_PARAM);
}

static void test_operations_on_uninit(void **state)
{
    (void)state;
    sched_policy_t ctx;
    ctx.state = SCHED_POLICY_UNINIT;

    uint32_t id;
    assert_int_equal(sched_policy_select_next(&ctx, &id), SCHED_ERR_STATE);
    assert_int_equal(sched_policy_add_task(&ctx, 1, 5), SCHED_ERR_STATE);
    assert_int_equal(sched_policy_remove_task(&ctx, 1), SCHED_ERR_STATE);
    assert_false(sched_policy_has_ready_task(&ctx));
}

/* -------------------------------------------------------------------------
 * Boundary / stress tests
 * ---------------------------------------------------------------------- */

static void test_fill_to_capacity(void **state)
{
    (void)state;

    for (uint32_t i = 0; i < SCHED_READY_QUEUE_CAPACITY; i++)
    {
        assert_int_equal(sched_policy_add_task(&g_ctx, i + 1, i), SCHED_OK);
    }

    /* Next add should overflow */
    assert_int_equal(sched_policy_add_task(&g_ctx, 99, 50), SCHED_ERR_OVERFLOW);
}

static void test_remove_middle_then_select(void **state)
{
    (void)state;

    sched_policy_add_task(&g_ctx, 1, 10);
    sched_policy_add_task(&g_ctx, 2, 20);
    sched_policy_add_task(&g_ctx, 3, 15);

    assert_int_equal(sched_policy_remove_task(&g_ctx, 2), SCHED_OK);

    uint32_t id;
    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 3); /* priority 15, not 20 (removed) */

    assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 1); /* priority 10 */
}

static void test_fill_drain_refill(void **state)
{
    (void)state;

    for (uint32_t i = 0; i < SCHED_READY_QUEUE_CAPACITY; i++)
    {
        sched_policy_add_task(&g_ctx, i + 1, i + 1);
    }

    uint32_t id;
    for (uint32_t i = 0; i < SCHED_READY_QUEUE_CAPACITY; i++)
    {
        assert_int_equal(sched_policy_select_next(&g_ctx, &id), SCHED_OK);
    }
    assert_false(sched_policy_has_ready_task(&g_ctx));

    /* Refill */
    assert_int_equal(sched_policy_add_task(&g_ctx, 42, 7), SCHED_OK);
    assert_true(sched_policy_has_ready_task(&g_ctx));
}

static void test_dispatch_matches_select(void **state)
{
    (void)state;
    sched_policy_add_task(&g_ctx, 1, 5);
    sched_policy_add_task(&g_ctx, 2, 10);

    uint32_t id;
    assert_int_equal(sched_policy_dispatch_candidate(&g_ctx, &id), SCHED_OK);
    assert_int_equal(id, 2);
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
        /* Empty */
        cmocka_unit_test_setup(test_empty_no_ready_task, setup),
        cmocka_unit_test_setup(test_empty_select_returns_not_found, setup),
        cmocka_unit_test_setup(test_empty_peek_returns_not_found, setup),
        cmocka_unit_test_setup(test_empty_dispatch_returns_not_found, setup),
        /* Single task */
        cmocka_unit_test_setup(test_one_task_select, setup),
        cmocka_unit_test_setup(test_one_task_peek_does_not_dequeue, setup),
        /* Multiple priorities */
        cmocka_unit_test_setup(test_multi_priority_selects_highest, setup),
        cmocka_unit_test_setup(test_peek_shows_highest_after_dequeue, setup),
        /* FIFO */
        cmocka_unit_test_setup(test_equal_priority_fifo, setup),
        /* Reset */
        cmocka_unit_test_setup(test_reset_clears_queue, setup),
        cmocka_unit_test(test_reset_null),
        cmocka_unit_test(test_reset_uninit),
        cmocka_unit_test_setup(test_reset_then_reuse, setup),
        /* Repeated selection */
        cmocka_unit_test_setup(test_repeated_select_drains, setup),
        /* Invalid inputs */
        cmocka_unit_test(test_select_null_ctx),
        cmocka_unit_test_setup(test_select_null_out, setup),
        cmocka_unit_test(test_peek_null_ctx),
        cmocka_unit_test_setup(test_peek_null_out, setup),
        cmocka_unit_test(test_add_null_ctx),
        cmocka_unit_test(test_remove_null_ctx),
        cmocka_unit_test(test_has_ready_null),
        cmocka_unit_test(test_dispatch_null_ctx),
        cmocka_unit_test(test_operations_on_uninit),
        /* Boundary / stress */
        cmocka_unit_test_setup(test_fill_to_capacity, setup),
        cmocka_unit_test_setup(test_remove_middle_then_select, setup),
        cmocka_unit_test_setup(test_fill_drain_refill, setup),
        cmocka_unit_test_setup(test_dispatch_matches_select, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
