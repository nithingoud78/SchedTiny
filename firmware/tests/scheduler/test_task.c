/**
 * @file    test_task.c
 * @brief   Unit tests for SchedTiny Task Registration and Lookup API.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
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

/* Dummy callback */
static void dummy_task_callback(void *pvParameters)
{
    (void)pvParameters;
}

static void test_sched_task_register_success(void **state)
{
    (void)state;

    StackType_t stack[128];
    StaticTask_t tcb;

    sched_task_t config = {.task_id        = 1,
                           .name           = "TestTask",
                           .pxTaskCode     = dummy_task_callback,
                           .pvParameters   = NULL,
                           .ulStackDepth   = 128,
                           .uxPriority     = 1,
                           .puxStackBuffer = stack,
                           .pxTaskBuffer   = &tcb,
                           .handle         = NULL,
                           .state          = SCHED_TASK_STATE_UNUSED};

    /* Will expect xTaskCreateStatic and we return a non-null handle (0x1234) */
    will_return(xTaskCreateStatic, (intptr_t)0x1234);

    SchedStatus_t status = sched_task_register(&config);
    assert_int_equal(status, SCHED_OK);

    /* Verify it is in the registry */
    sched_task_t out_task;
    status = sched_task_get(1, &out_task);
    assert_int_equal(status, SCHED_OK);
    assert_int_equal(out_task.task_id, 1);
    assert_string_equal(out_task.name, "TestTask");
    assert_int_equal(out_task.state, SCHED_TASK_STATE_READY);
    assert_ptr_equal(out_task.handle, (TaskHandle_t)0x1234);
}

static void test_sched_task_register_fail_invalid_param(void **state)
{
    (void)state;
    SchedStatus_t status = sched_task_register(NULL);
    assert_int_equal(status, SCHED_ERR_PARAM);
}

static void test_sched_task_remove_success(void **state)
{
    (void)state;

    /* Task 1 is already registered from the previous test (static registry). */

    /* We expect vTaskDelete to be called with handle 0x1234 */
    expect_value(vTaskDelete, xTaskToDelete, (intptr_t)0x1234);

    SchedStatus_t status = sched_task_remove(1);
    assert_int_equal(status, SCHED_OK);

    /* Verify it is removed */
    sched_task_t out_task;
    status = sched_task_get(1, &out_task);
    assert_int_equal(status, SCHED_ERR_NOT_FOUND);
}

static void test_sched_task_remove_not_found(void **state)
{
    (void)state;
    SchedStatus_t status = sched_task_remove(99);
    assert_int_equal(status, SCHED_ERR_NOT_FOUND);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_sched_task_register_success),
        cmocka_unit_test(test_sched_task_register_fail_invalid_param),
        cmocka_unit_test(test_sched_task_remove_success),
        cmocka_unit_test(test_sched_task_remove_not_found),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
