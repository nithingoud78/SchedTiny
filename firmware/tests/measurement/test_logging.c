// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on
#include "bench_config.h"
#include "bench_log.h"
#include "bench_measure.h"

#include <stdio.h>
#include <string.h>

extern char mock_uart_buffer[4096];
extern uint16_t mock_uart_len;
extern uint8_t mock_uart_busy;
extern uint32_t mock_dwt_counter;

static void test_logging_basic(void **state)
{
    (void)state;

    bench_init();
    bench_log_init();

    mock_uart_len    = 0;
    mock_uart_busy   = 0;
    mock_dwt_counter = 12345;

    bench_log_event(BENCH_EV_TASK_START, 2, 42);

    SchedStatus_t status = bench_log_flush();
    assert_int_equal(status, SCHED_OK);

    // Check if the mock UART received the correct JSON
    assert_true(mock_uart_len > 0);

    // We expect: {"ts":12345,"ev":0,"tid":2,"val":42}\n
    char expected_json[128];
    snprintf(expected_json, sizeof(expected_json),
             "{\"ts\":12345,\"ev\":0,\"tid\":2,\"val\":42}\n");

    assert_string_equal(mock_uart_buffer, expected_json);
}

static void test_logging_busy_state(void **state)
{
    (void)state;

    bench_init();
    bench_log_init();

    mock_uart_busy = 1;  // Simulate DMA in progress

    bench_log_event(BENCH_EV_TASK_END, 1, 0);

    SchedStatus_t status = bench_log_flush();
    assert_int_equal(status, SCHED_ERR_STATE);

    // Now simulate DMA finish
    mock_uart_busy = 0;
    status         = bench_log_flush();
    assert_int_equal(status, SCHED_OK);
    assert_true(mock_uart_len > 0);
}

int main(void)
{
    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_logging_basic),
        cmocka_unit_test(test_logging_busy_state),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}
