#include "sched_core.h"
#include "task.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

// Note: Test execution order matters because we are testing static state
// transitions.

static void test_initial_state(void **state)
{
    (void)state;
    assert_int_equal(sched_core_get_state(), SCHED_STATE_UNINIT);
}

static void test_init_success(void **state)
{
    (void)state;
    SchedStatus_t status = sched_core_init();
    assert_int_equal(status, SCHED_OK);
    assert_int_equal(sched_core_get_state(), SCHED_STATE_READY);
}

static void test_init_double_fail(void **state)
{
    (void)state;
    SchedStatus_t status = sched_core_init();
    assert_int_equal(status, SCHED_ERR_STATE);
    assert_int_equal(sched_core_get_state(), SCHED_STATE_READY);
}

static void test_start_transitions_to_running_then_halts(void **state)
{
    (void)state;
    // We expect sched_core_start to call vTaskStartScheduler
    expect_function_call(vTaskStartScheduler);

    // In our mock environment, vTaskStartScheduler returns immediately.
    // The scheduler core should then transition to HALTED and trigger an
    // assertion. Since we defined HOST_TEST to bypass the infinite loop in
    // vAssertCalled, the function will just return.
    sched_core_start();

    assert_int_equal(sched_core_get_state(), SCHED_STATE_HALTED);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_initial_state),
        cmocka_unit_test(test_init_success),
        cmocka_unit_test(test_init_double_fail),
        cmocka_unit_test(test_start_transitions_to_running_then_halts),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
