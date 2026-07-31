/**
 * @file    test_dispatcher.c
 * @brief   CMocka unit tests for SchedTiny Dispatcher.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_dispatcher.h"

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

static sched_dispatcher_t g_ctx;

static int setup(void **state)
{
    (void)state;
    sched_dispatcher_init(&g_ctx);
    return 0;
}

/* -------------------------------------------------------------------------
 * Initialization & Reset
 * ---------------------------------------------------------------------- */

static void test_init_success(void **state)
{
    (void)state;
    sched_dispatcher_t ctx;
    assert_int_equal(sched_dispatcher_init(&ctx), SCHED_OK);
    assert_true(ctx.initialized);
    assert_true(ctx.is_idle);
    assert_int_equal(ctx.current_task_id, SCHED_DISPATCHER_NO_TASK);
    assert_int_equal(ctx.context_switch_count, 0);
    assert_int_equal(ctx.tick_count, 0);
}

static void test_init_null(void **state)
{
    (void)state;
    assert_int_equal(sched_dispatcher_init(NULL), SCHED_ERR_PARAM);
}

static void test_reset_success(void **state)
{
    (void)state;
    sched_dispatcher_dispatch(&g_ctx, 1);
    sched_dispatcher_tick(&g_ctx);

    assert_int_equal(sched_dispatcher_reset(&g_ctx), SCHED_OK);

    assert_true(g_ctx.is_idle);
    assert_int_equal(g_ctx.current_task_id, SCHED_DISPATCHER_NO_TASK);
    assert_int_equal(g_ctx.context_switch_count, 0);
    assert_int_equal(g_ctx.tick_count, 0);
}

static void test_reset_uninit(void **state)
{
    (void)state;
    sched_dispatcher_t ctx;
    ctx.initialized = false;
    assert_int_equal(sched_dispatcher_reset(&ctx), SCHED_ERR_STATE);
}

/* -------------------------------------------------------------------------
 * Dispatching
 * ---------------------------------------------------------------------- */

static void test_first_dispatch(void **state)
{
    (void)state;
    assert_int_equal(sched_dispatcher_dispatch(&g_ctx, 1), SCHED_OK);

    assert_false(g_ctx.is_idle);
    assert_int_equal(g_ctx.current_task_id, 1);
    assert_int_equal(g_ctx.context_switch_count, 1);  // 1 switch from idle to task
}

static void test_repeated_dispatch_noop(void **state)
{
    (void)state;
    sched_dispatcher_dispatch(&g_ctx, 1);
    uint32_t switches = g_ctx.context_switch_count;

    assert_int_equal(sched_dispatcher_dispatch(&g_ctx, 1), SCHED_OK);

    /* The context switch count should not increase if task is unchanged */
    assert_int_equal(g_ctx.context_switch_count, switches);
}

static void test_context_switch_counting(void **state)
{
    (void)state;
    sched_dispatcher_dispatch(&g_ctx, 1);
    assert_int_equal(g_ctx.context_switch_count, 1);

    sched_dispatcher_dispatch(&g_ctx, 2);
    assert_int_equal(g_ctx.context_switch_count, 2);

    sched_dispatcher_dispatch(&g_ctx, 3);
    assert_int_equal(g_ctx.context_switch_count, 3);

    sched_dispatcher_dispatch(&g_ctx, SCHED_DISPATCHER_NO_TASK);  // to idle
    assert_int_equal(g_ctx.context_switch_count, 4);

    sched_dispatcher_dispatch(&g_ctx, SCHED_DISPATCHER_NO_TASK);  // still idle
    assert_int_equal(g_ctx.context_switch_count, 4);
}

/* -------------------------------------------------------------------------
 * Idle state
 * ---------------------------------------------------------------------- */

static void test_idle_state(void **state)
{
    (void)state;
    sched_dispatcher_dispatch(&g_ctx, 1);
    assert_false(g_ctx.is_idle);

    assert_int_equal(sched_dispatcher_idle(&g_ctx), SCHED_OK);
    assert_true(g_ctx.is_idle);
    assert_int_equal(g_ctx.current_task_id, SCHED_DISPATCHER_NO_TASK);
}

/* -------------------------------------------------------------------------
 * Tick
 * ---------------------------------------------------------------------- */

static void test_dispatcher_tick(void **state)
{
    (void)state;
    assert_int_equal(g_ctx.tick_count, 0);
    assert_int_equal(sched_dispatcher_tick(&g_ctx), SCHED_OK);
    assert_int_equal(g_ctx.tick_count, 1);
}

/* -------------------------------------------------------------------------
 * Current Task & Switch Count Getters
 * ---------------------------------------------------------------------- */

static void test_getters(void **state)
{
    (void)state;
    uint32_t task_id;
    uint32_t switches;

    /* Initially idle, should return error when requesting current task */
    assert_int_equal(sched_dispatcher_current_task(&g_ctx, &task_id), SCHED_ERR_STATE);

    sched_dispatcher_dispatch(&g_ctx, 5);

    assert_int_equal(sched_dispatcher_current_task(&g_ctx, &task_id), SCHED_OK);
    assert_int_equal(task_id, 5);

    assert_int_equal(sched_dispatcher_context_switch_count(&g_ctx, &switches), SCHED_OK);
    assert_int_equal(switches, 1);
}

/* -------------------------------------------------------------------------
 * Invalid Arguments
 * ---------------------------------------------------------------------- */

static void test_invalid_args(void **state)
{
    (void)state;
    uint32_t dummy;

    assert_int_equal(sched_dispatcher_dispatch(NULL, 1), SCHED_ERR_PARAM);
    assert_int_equal(sched_dispatcher_tick(NULL), SCHED_ERR_PARAM);
    assert_int_equal(sched_dispatcher_idle(NULL), SCHED_ERR_PARAM);

    assert_int_equal(sched_dispatcher_current_task(NULL, &dummy), SCHED_ERR_PARAM);
    assert_int_equal(sched_dispatcher_current_task(&g_ctx, NULL), SCHED_ERR_PARAM);

    assert_int_equal(sched_dispatcher_context_switch_count(NULL, &dummy), SCHED_ERR_PARAM);
    assert_int_equal(sched_dispatcher_context_switch_count(&g_ctx, NULL), SCHED_ERR_PARAM);
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
        cmocka_unit_test_setup(test_first_dispatch, setup),
        cmocka_unit_test_setup(test_repeated_dispatch_noop, setup),
        cmocka_unit_test_setup(test_context_switch_counting, setup),
        cmocka_unit_test_setup(test_idle_state, setup),
        cmocka_unit_test_setup(test_dispatcher_tick, setup),
        cmocka_unit_test_setup(test_getters, setup),
        cmocka_unit_test_setup(test_invalid_args, setup),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
