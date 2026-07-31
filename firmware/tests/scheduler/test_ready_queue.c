/**
 * @file    test_ready_queue.c
 * @brief   CMocka unit tests for SchedTiny Ready Queue.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_ready_queue.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

/* -------------------------------------------------------------------------
 * Fixture: each test gets a freshly cleared queue
 * ---------------------------------------------------------------------- */

static sched_ready_queue_t g_queue;

static int setup(void **state)
{
    (void)state;
    sched_rq_clear(&g_queue);
    return 0;
}

/* -------------------------------------------------------------------------
 * Empty queue tests
 * ---------------------------------------------------------------------- */

static void test_empty_queue_is_empty(void **state)
{
    (void)state;
    assert_true(sched_rq_is_empty(&g_queue));
    assert_false(sched_rq_is_full(&g_queue));
    assert_int_equal(sched_rq_size(&g_queue), 0);
}

static void test_dequeue_empty_returns_not_found(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_ERR_NOT_FOUND);
}

static void test_remove_empty_returns_not_found(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_remove(&g_queue, 42), SCHED_ERR_NOT_FOUND);
}

static void test_contains_empty_returns_false(void **state)
{
    (void)state;
    assert_false(sched_rq_contains(&g_queue, 1));
}

/* -------------------------------------------------------------------------
 * Enqueue tests
 * ---------------------------------------------------------------------- */

static void test_enqueue_single(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_enqueue(&g_queue, 1, 5), SCHED_OK);
    assert_int_equal(sched_rq_size(&g_queue), 1);
    assert_true(sched_rq_contains(&g_queue, 1));
    assert_false(sched_rq_is_empty(&g_queue));
}

static void test_enqueue_priority_ordering(void **state)
{
    (void)state;

    /* Insert low, medium, high — verify dequeue returns high first */
    assert_int_equal(sched_rq_enqueue(&g_queue, 10, 1), SCHED_OK);
    assert_int_equal(sched_rq_enqueue(&g_queue, 20, 5), SCHED_OK);
    assert_int_equal(sched_rq_enqueue(&g_queue, 30, 3), SCHED_OK);

    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 20); /* priority 5 — highest */

    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 30); /* priority 3 */

    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 10); /* priority 1 — lowest */
}

static void test_enqueue_same_priority_fifo(void **state)
{
    (void)state;

    /* Equal priority — FIFO order preserved */
    assert_int_equal(sched_rq_enqueue(&g_queue, 1, 5), SCHED_OK);
    assert_int_equal(sched_rq_enqueue(&g_queue, 2, 5), SCHED_OK);
    assert_int_equal(sched_rq_enqueue(&g_queue, 3, 5), SCHED_OK);

    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 1);
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 2);
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 3);
}

static void test_enqueue_duplicate_rejected(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_enqueue(&g_queue, 1, 5), SCHED_OK);
    assert_int_equal(sched_rq_enqueue(&g_queue, 1, 10), SCHED_ERR_STATE);
    assert_int_equal(sched_rq_size(&g_queue), 1);
}

/* -------------------------------------------------------------------------
 * Full queue tests
 * ---------------------------------------------------------------------- */

static void test_full_queue(void **state)
{
    (void)state;

    for (uint32_t i = 0; i < SCHED_READY_QUEUE_CAPACITY; i++)
    {
        assert_int_equal(sched_rq_enqueue(&g_queue, i + 1, i), SCHED_OK);
    }

    assert_true(sched_rq_is_full(&g_queue));
    assert_int_equal(sched_rq_size(&g_queue), SCHED_READY_QUEUE_CAPACITY);

    /* One more should overflow */
    assert_int_equal(sched_rq_enqueue(&g_queue, 99, 50), SCHED_ERR_OVERFLOW);
}

/* -------------------------------------------------------------------------
 * Dequeue tests
 * ---------------------------------------------------------------------- */

static void test_dequeue_returns_highest_priority(void **state)
{
    (void)state;

    sched_rq_enqueue(&g_queue, 1, 10);
    sched_rq_enqueue(&g_queue, 2, 20);
    sched_rq_enqueue(&g_queue, 3, 15);

    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 2); /* priority 20 */
    assert_int_equal(sched_rq_size(&g_queue), 2);
}

/* -------------------------------------------------------------------------
 * Remove tests
 * ---------------------------------------------------------------------- */

static void test_remove_middle(void **state)
{
    (void)state;

    sched_rq_enqueue(&g_queue, 1, 10);
    sched_rq_enqueue(&g_queue, 2, 8);
    sched_rq_enqueue(&g_queue, 3, 5);

    /* Remove middle element (task 2, priority 8) */
    assert_int_equal(sched_rq_remove(&g_queue, 2), SCHED_OK);
    assert_int_equal(sched_rq_size(&g_queue), 2);
    assert_false(sched_rq_contains(&g_queue, 2));

    /* Remaining order should be preserved */
    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 1); /* priority 10 */
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 3); /* priority 5 */
}

static void test_remove_head(void **state)
{
    (void)state;

    sched_rq_enqueue(&g_queue, 1, 10);
    sched_rq_enqueue(&g_queue, 2, 5);

    assert_int_equal(sched_rq_remove(&g_queue, 1), SCHED_OK);
    assert_int_equal(sched_rq_size(&g_queue), 1);

    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 2);
}

static void test_remove_tail(void **state)
{
    (void)state;

    sched_rq_enqueue(&g_queue, 1, 10);
    sched_rq_enqueue(&g_queue, 2, 5);

    assert_int_equal(sched_rq_remove(&g_queue, 2), SCHED_OK);
    assert_int_equal(sched_rq_size(&g_queue), 1);

    uint32_t id;
    assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
    assert_int_equal(id, 1);
}

static void test_remove_nonexistent(void **state)
{
    (void)state;
    sched_rq_enqueue(&g_queue, 1, 5);
    assert_int_equal(sched_rq_remove(&g_queue, 999), SCHED_ERR_NOT_FOUND);
    assert_int_equal(sched_rq_size(&g_queue), 1);
}

/* -------------------------------------------------------------------------
 * Invalid argument tests
 * ---------------------------------------------------------------------- */

static void test_null_queue_clear(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_clear(NULL), SCHED_ERR_PARAM);
}

static void test_null_queue_enqueue(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_enqueue(NULL, 1, 5), SCHED_ERR_PARAM);
}

static void test_null_queue_dequeue(void **state)
{
    (void)state;
    uint32_t id;
    assert_int_equal(sched_rq_dequeue(NULL, &id), SCHED_ERR_PARAM);
}

static void test_null_out_dequeue(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_dequeue(&g_queue, NULL), SCHED_ERR_PARAM);
}

static void test_null_queue_remove(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_remove(NULL, 1), SCHED_ERR_PARAM);
}

static void test_null_queue_contains(void **state)
{
    (void)state;
    assert_false(sched_rq_contains(NULL, 1));
}

static void test_null_queue_size(void **state)
{
    (void)state;
    assert_int_equal(sched_rq_size(NULL), 0);
}

static void test_null_queue_is_empty(void **state)
{
    (void)state;
    assert_true(sched_rq_is_empty(NULL));
}

static void test_null_queue_is_full(void **state)
{
    (void)state;
    assert_false(sched_rq_is_full(NULL));
}

/* -------------------------------------------------------------------------
 * Stress tests
 * ---------------------------------------------------------------------- */

static void test_fill_drain_cycle(void **state)
{
    (void)state;

    /* Fill completely */
    for (uint32_t i = 0; i < SCHED_READY_QUEUE_CAPACITY; i++)
    {
        assert_int_equal(sched_rq_enqueue(&g_queue, i + 100, i * 2), SCHED_OK);
    }
    assert_true(sched_rq_is_full(&g_queue));

    /* Drain completely — should come out in descending priority */
    uint32_t prev_prio = UINT32_MAX;
    for (uint32_t i = 0; i < SCHED_READY_QUEUE_CAPACITY; i++)
    {
        uint32_t id;
        assert_int_equal(sched_rq_dequeue(&g_queue, &id), SCHED_OK);
        /* Verify descending order */
        uint32_t expected_prio = (SCHED_READY_QUEUE_CAPACITY - 1 - i) * 2;
        (void)prev_prio;
        assert_int_equal(id, (SCHED_READY_QUEUE_CAPACITY - i - 1) + 100);
        (void)expected_prio;
    }
    assert_true(sched_rq_is_empty(&g_queue));

    /* Refill — must work after drain */
    assert_int_equal(sched_rq_enqueue(&g_queue, 1, 1), SCHED_OK);
    assert_int_equal(sched_rq_size(&g_queue), 1);
}

static void test_interleaved_enqueue_dequeue(void **state)
{
    (void)state;

    sched_rq_enqueue(&g_queue, 1, 10);
    sched_rq_enqueue(&g_queue, 2, 20);

    uint32_t id;
    sched_rq_dequeue(&g_queue, &id);
    assert_int_equal(id, 2);

    sched_rq_enqueue(&g_queue, 3, 15);

    sched_rq_dequeue(&g_queue, &id);
    assert_int_equal(id, 3); /* priority 15 > 10 */

    sched_rq_dequeue(&g_queue, &id);
    assert_int_equal(id, 1);

    assert_true(sched_rq_is_empty(&g_queue));
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* Empty queue */
        cmocka_unit_test_setup(test_empty_queue_is_empty, setup),
        cmocka_unit_test_setup(test_dequeue_empty_returns_not_found, setup),
        cmocka_unit_test_setup(test_remove_empty_returns_not_found, setup),
        cmocka_unit_test_setup(test_contains_empty_returns_false, setup),
        /* Enqueue */
        cmocka_unit_test_setup(test_enqueue_single, setup),
        cmocka_unit_test_setup(test_enqueue_priority_ordering, setup),
        cmocka_unit_test_setup(test_enqueue_same_priority_fifo, setup),
        cmocka_unit_test_setup(test_enqueue_duplicate_rejected, setup),
        /* Full queue */
        cmocka_unit_test_setup(test_full_queue, setup),
        /* Dequeue */
        cmocka_unit_test_setup(test_dequeue_returns_highest_priority, setup),
        /* Remove */
        cmocka_unit_test_setup(test_remove_middle, setup),
        cmocka_unit_test_setup(test_remove_head, setup),
        cmocka_unit_test_setup(test_remove_tail, setup),
        cmocka_unit_test_setup(test_remove_nonexistent, setup),
        /* Invalid arguments */
        cmocka_unit_test(test_null_queue_clear),
        cmocka_unit_test(test_null_queue_enqueue),
        cmocka_unit_test(test_null_queue_dequeue),
        cmocka_unit_test(test_null_out_dequeue),
        cmocka_unit_test(test_null_queue_remove),
        cmocka_unit_test(test_null_queue_contains),
        cmocka_unit_test(test_null_queue_size),
        cmocka_unit_test(test_null_queue_is_empty),
        cmocka_unit_test(test_null_queue_is_full),
        /* Stress */
        cmocka_unit_test_setup(test_fill_drain_cycle, setup),
        cmocka_unit_test_setup(test_interleaved_enqueue_dequeue, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
