/**
 * @file    test_trace.c
 * @brief   Unit tests for SchedTiny trace framework.
 */

#include "sched_trace.h"

#include <cmocka.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/* Test basic trace recording */
static void test_trace_basic(void **state)
{
    (void)state;

    sched_trace_reset();

    uint32_t count;
    assert_int_equal(sched_trace_get_count(&count), SCHED_OK);
    assert_int_equal(count, 0);

    /* Record one event */
    assert_int_equal(
        sched_trace_record(100, SCHED_TRACE_EVT_TASK_START, 1, 0, 0, 0, 10, 1000, 50, 0), SCHED_OK);

    assert_int_equal(sched_trace_get_count(&count), SCHED_OK);
    assert_int_equal(count, 1);

    const sched_trace_event_t *buf;
    assert_int_equal(sched_trace_get_buffer(&buf, &count), SCHED_OK);
    assert_int_equal(count, 1);

    assert_int_equal(buf[0].timestamp, 100);
    assert_int_equal(buf[0].event_type, SCHED_TRACE_EVT_TASK_START);
    assert_int_equal(buf[0].task_id, 1);
}

/* Test buffer wrapping */
static void test_trace_wrap(void **state)
{
    (void)state;

    sched_trace_reset();

    /* Fill buffer exactly */
    for (int i = 0; i < SCHED_TRACE_MAX_EVENTS; i++)
    {
        sched_trace_record(i, SCHED_TRACE_EVT_TASK_READY, 1, 0, 0, 0, 0, 0, 0, 0);
    }

    uint32_t count;
    assert_int_equal(sched_trace_get_count(&count), SCHED_OK);
    assert_int_equal(count, SCHED_TRACE_MAX_EVENTS);

    /* Overwrite one */
    sched_trace_record(9999, SCHED_TRACE_EVT_TASK_PREEMPT, 2, 0, 0, 0, 0, 0, 0, 0);

    assert_int_equal(sched_trace_get_count(&count), SCHED_OK);
    assert_int_equal(count, SCHED_TRACE_MAX_EVENTS);

    const sched_trace_event_t *buf;
    assert_int_equal(sched_trace_get_buffer(&buf, &count), SCHED_OK);

    /* The first logical element should now be the second one we inserted (timestamp 1) */
    assert_int_equal(buf[0].timestamp, 1);

    /* The last logical element should be the newly inserted one */
    assert_int_equal(buf[SCHED_TRACE_MAX_EVENTS - 1].timestamp, 9999);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_trace_basic),
        cmocka_unit_test(test_trace_wrap),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
