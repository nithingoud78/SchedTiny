/**
 * @file    test_hal.c
 * @brief   Unit tests for SchedTiny Hardware Abstraction Layer.
 *
 * @author  @nithingoud78
 * @date    2026-08-01
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "bench_hal.h"
#include "bench_measure.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
#include <string.h>
// clang-format on

/* -------------------------------------------------------------------------
 * Tests
 * ---------------------------------------------------------------------- */

static void test_hal_clock_frequency(void **state)
{
    (void)state;
    // Core clock should be defined and greater than zero
    assert_true(BENCH_CORE_CLOCK_MHZ > 0);
}

static void test_hal_mcu_variant(void **state)
{
    (void)state;
    // Variant name should be defined and not empty
    assert_non_null(BENCH_MCU_VARIANT);
    assert_true(strlen(BENCH_MCU_VARIANT) > 0);
}

static void test_bench_cycles_to_us(void **state)
{
    (void)state;
    uint64_t cycles = BENCH_CORE_CLOCK_MHZ * 1000;

    // 1000 microseconds worth of cycles should equal 1000
    uint32_t microseconds = bench_cycles_to_us(cycles);
    assert_int_equal(microseconds, 1000);
}

/* -------------------------------------------------------------------------
 * Test Runner
 * ---------------------------------------------------------------------- */

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_hal_clock_frequency),
        cmocka_unit_test(test_hal_mcu_variant),
        cmocka_unit_test(test_bench_cycles_to_us),
    };

    return cmocka_run_group_tests(tests, NULL, NULL);
}
