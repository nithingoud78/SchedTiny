#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include "bench_measure.h"

extern uint32_t mock_dwt_counter;

static void test_dwt_monotonicity(void **state)
{
    (void) state; /* unused */
    
    bench_init();
    
    // Simulate initial value
    mock_dwt_counter = 100;
    uint64_t t1 = bench_dwt_cycles64();
    assert_int_equal(t1, 100);
    
    // Simulate standard increment
    mock_dwt_counter = 200;
    uint64_t t2 = bench_dwt_cycles64();
    assert_int_equal(t2, 200);
}

static void test_dwt_overflow(void **state)
{
    (void) state; /* unused */
    
    bench_init();
    
    // Simulate near overflow
    mock_dwt_counter = 0xFFFFFFF0;
    uint64_t t1 = bench_dwt_cycles64();
    assert_int_equal(t1, 0xFFFFFFF0);
    
    // Simulate overflow
    mock_dwt_counter = 10;
    uint64_t t2 = bench_dwt_cycles64();
    
    // The top 32 bits should now be 1
    assert_true(t2 > t1);
    assert_int_equal(t2, 0x10000000AULL);
    
    // Second overflow
    mock_dwt_counter = 0xFFFFFFFF;
    bench_dwt_cycles64(); // Update state
    mock_dwt_counter = 5;
    uint64_t t3 = bench_dwt_cycles64();
    assert_int_equal(t3, 0x200000005ULL);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_dwt_monotonicity),
        cmocka_unit_test(test_dwt_overflow),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
