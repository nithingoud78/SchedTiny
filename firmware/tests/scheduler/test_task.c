/**
 * @file    test_task.c
 * @brief   Unit tests for SchedTiny Task Registration, Lookup, and Priority API.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.2.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_task.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

/* -------------------------------------------------------------------------
 * Helpers
 * ---------------------------------------------------------------------- */

static void dummy_task_callback(void *pvParameters)
{
    (void)pvParameters;
}

/**
 * @brief Build a minimal valid task config with a given ID and priority.
 */
static sched_task_t make_config(uint32_t id,
                                sched_priority_t prio,
                                StackType_t *stack,
                                StaticTask_t *tcb)
{
    sched_task_t cfg = {.task_id        = id,
                        .name           = "TestTask",
                        .pxTaskCode     = dummy_task_callback,
                        .pvParameters   = NULL,
                        .ulStackDepth   = 128,
                        .priority       = prio,
                        .puxStackBuffer = stack,
                        .pxTaskBuffer   = tcb,
                        .handle         = NULL,
                        .state          = SCHED_TASK_STATE_UNUSED};
    return cfg;
}

/* -------------------------------------------------------------------------
 * Registration tests
 * ---------------------------------------------------------------------- */

static void test_register_success(void **state)
{
    (void)state;

    StackType_t stack[128];
    StaticTask_t tcb;
    sched_task_t cfg = make_config(10, 5, stack, &tcb);

    will_return(xTaskCreateStatic, (intptr_t)0xABCD);

    SchedStatus_t rc = sched_task_register(&cfg);
    assert_int_equal(rc, SCHED_OK);

    sched_task_t out;
    rc = sched_task_get(10, &out);
    assert_int_equal(rc, SCHED_OK);
    assert_int_equal(out.task_id, 10);
    assert_int_equal(out.priority, 5);
    assert_int_equal(out.state, SCHED_TASK_STATE_READY);
    assert_ptr_equal(out.handle, (TaskHandle_t)0xABCD);
}

static void test_register_null_config(void **state)
{
    (void)state;
    assert_int_equal(sched_task_register(NULL), SCHED_ERR_PARAM);
}

static void test_register_null_callback(void **state)
{
    (void)state;

    StackType_t stack[128];
    StaticTask_t tcb;
    sched_task_t cfg = make_config(11, 1, stack, &tcb);
    cfg.pxTaskCode   = NULL;

    assert_int_equal(sched_task_register(&cfg), SCHED_ERR_PARAM);
}

static void test_register_null_stack(void **state)
{
    (void)state;

    StaticTask_t tcb;
    sched_task_t cfg   = make_config(12, 1, NULL, &tcb);
    cfg.puxStackBuffer = NULL;

    assert_int_equal(sched_task_register(&cfg), SCHED_ERR_PARAM);
}

/* -------------------------------------------------------------------------
 * Remove tests
 * ---------------------------------------------------------------------- */

static void test_remove_success(void **state)
{
    (void)state;

    /* Task 10 is still registered from test_register_success (shared registry). */
    expect_value(vTaskDelete, xTaskToDelete, (intptr_t)0xABCD);

    assert_int_equal(sched_task_remove(10), SCHED_OK);

    sched_task_t out;
    assert_int_equal(sched_task_get(10, &out), SCHED_ERR_NOT_FOUND);
}

static void test_remove_not_found(void **state)
{
    (void)state;
    assert_int_equal(sched_task_remove(999), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Priority setter tests
 * ---------------------------------------------------------------------- */

static void test_set_priority_success(void **state)
{
    (void)state;

    /* Register a fresh task */
    StackType_t stack[128];
    StaticTask_t tcb;
    sched_task_t cfg = make_config(20, 3, stack, &tcb);

    will_return(xTaskCreateStatic, (intptr_t)0x5678);
    assert_int_equal(sched_task_register(&cfg), SCHED_OK);

    /* Set priority — expect vTaskPrioritySet with the handle and new priority */
    expect_value(vTaskPrioritySet, xTask, (intptr_t)0x5678);
    expect_value(vTaskPrioritySet, uxNewPriority, 10);

    assert_int_equal(sched_task_set_priority(20, 10), SCHED_OK);

    /* Verify registry was updated */
    sched_priority_t prio;
    assert_int_equal(sched_task_get_priority(20, &prio), SCHED_OK);
    assert_int_equal(prio, 10);
}

static void test_set_priority_max_valid(void **state)
{
    (void)state;

    expect_value(vTaskPrioritySet, xTask, (intptr_t)0x5678);
    expect_value(vTaskPrioritySet, uxNewPriority, SCHED_PRIORITY_MAX);

    assert_int_equal(sched_task_set_priority(20, SCHED_PRIORITY_MAX), SCHED_OK);

    sched_priority_t prio;
    assert_int_equal(sched_task_get_priority(20, &prio), SCHED_OK);
    assert_int_equal(prio, SCHED_PRIORITY_MAX);
}

static void test_set_priority_min_valid(void **state)
{
    (void)state;

    expect_value(vTaskPrioritySet, xTask, (intptr_t)0x5678);
    expect_value(vTaskPrioritySet, uxNewPriority, SCHED_PRIORITY_MIN);

    assert_int_equal(sched_task_set_priority(20, SCHED_PRIORITY_MIN), SCHED_OK);

    sched_priority_t prio;
    assert_int_equal(sched_task_get_priority(20, &prio), SCHED_OK);
    assert_int_equal(prio, SCHED_PRIORITY_MIN);
}

static void test_set_priority_out_of_range(void **state)
{
    (void)state;
    /* Exceeds SCHED_PRIORITY_MAX (31) */
    assert_int_equal(sched_task_set_priority(20, SCHED_PRIORITY_MAX + 1), SCHED_ERR_PARAM);
}

static void test_set_priority_task_not_found(void **state)
{
    (void)state;
    assert_int_equal(sched_task_set_priority(999, 5), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Priority getter tests
 * ---------------------------------------------------------------------- */

static void test_get_priority_success(void **state)
{
    (void)state;

    sched_priority_t prio = 0xFF;
    assert_int_equal(sched_task_get_priority(20, &prio), SCHED_OK);
    assert_int_equal(prio, SCHED_PRIORITY_MIN); /* set to 0 in previous test */
}

static void test_get_priority_null_out(void **state)
{
    (void)state;
    assert_int_equal(sched_task_get_priority(20, NULL), SCHED_ERR_PARAM);
}

static void test_get_priority_not_found(void **state)
{
    (void)state;

    sched_priority_t prio;
    assert_int_equal(sched_task_get_priority(999, &prio), SCHED_ERR_NOT_FOUND);
}

/* -------------------------------------------------------------------------
 * Main
 * ---------------------------------------------------------------------- */

int main(void)
{
    const struct CMUnitTest tests[] = {
        /* Registration */
        cmocka_unit_test(test_register_success),
        cmocka_unit_test(test_register_null_config),
        cmocka_unit_test(test_register_null_callback),
        cmocka_unit_test(test_register_null_stack),
        /* Remove */
        cmocka_unit_test(test_remove_success),
        cmocka_unit_test(test_remove_not_found),
        /* Priority set */
        cmocka_unit_test(test_set_priority_success),
        cmocka_unit_test(test_set_priority_max_valid),
        cmocka_unit_test(test_set_priority_min_valid),
        cmocka_unit_test(test_set_priority_out_of_range),
        cmocka_unit_test(test_set_priority_task_not_found),
        /* Priority get */
        cmocka_unit_test(test_get_priority_success),
        cmocka_unit_test(test_get_priority_null_out),
        cmocka_unit_test(test_get_priority_not_found),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
