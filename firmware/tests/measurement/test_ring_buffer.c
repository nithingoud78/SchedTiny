// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
#include "bench_config.h"
#include "bench_measure.h"

static void test_ring_buffer_basic(void **state)
{
    (void)state;
    bench_init();

    bench_log_event(BENCH_EV_TASK_START, 1, 100);
    bench_log_event(BENCH_EV_TASK_END, 1, 200);

    BenchEventRecord_t records[10];
    uint32_t count = bench_get_log_records(records, 10);

    assert_int_equal(count, 2);
    assert_int_equal(records[0].event, BENCH_EV_TASK_START);
    assert_int_equal(records[0].task_id, 1);
    assert_int_equal(records[0].value, 100);

    assert_int_equal(records[1].event, BENCH_EV_TASK_END);
}

static void test_ring_buffer_overflow(void **state)
{
    (void)state;
    bench_init();

    // Fill the buffer to capacity
    for (int i = 0; i < BENCH_RING_BUFFER_SIZE; i++)
    {
        bench_log_event(BENCH_EV_TASK_START, 0, i);
    }

    // We expect 1 event to be dropped because the ring buffer needs 1 empty slot to differentiate
    // full vs empty
    assert_int_equal(bench_get_dropped_events(), 1);

    // Attempt to log another one
    bench_log_event(BENCH_EV_TASK_END, 0, 999);
    assert_int_equal(bench_get_dropped_events(), 2);

    // Read out the buffer
    BenchEventRecord_t records[BENCH_RING_BUFFER_SIZE + 10];
    uint32_t count = bench_get_log_records(records, BENCH_RING_BUFFER_SIZE + 10);

    // Capacity is BENCH_RING_BUFFER_SIZE - 1
    assert_int_equal(count, BENCH_RING_BUFFER_SIZE - 1);

    // The very first record read should have value 0
    assert_int_equal(records[0].value, 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_ring_buffer_basic),
        cmocka_unit_test(test_ring_buffer_overflow),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
