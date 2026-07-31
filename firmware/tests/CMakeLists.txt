cmake_minimum_required(VERSION 3.25)
project(SchedTinyTests C)

set(CMAKE_C_STANDARD 11)

find_package(cmocka REQUIRED)

# Include directories from firmware
include_directories(
    ${CMAKE_SOURCE_DIR}/../include
    ${CMAKE_SOURCE_DIR}/../core/bench
    ${CMAKE_SOURCE_DIR}/../core/drivers
)

# Build the measurement library for testing
add_library(measure_lib STATIC
    ${CMAKE_SOURCE_DIR}/../core/bench/bench_measure.c
    ${CMAKE_SOURCE_DIR}/../core/bench/bench_log.c
)

# Test executable: Timer/DWT
add_executable(test_timer measurement/test_timer.c measurement/mock_hal.c)
target_link_libraries(test_timer measure_lib cmocka)
add_test(NAME TimerTest COMMAND test_timer)

# Test executable: Ring Buffer
add_executable(test_ring_buffer measurement/test_ring_buffer.c measurement/mock_hal.c)
target_link_libraries(test_ring_buffer measure_lib cmocka)
add_test(NAME RingBufferTest COMMAND test_ring_buffer)

# Test executable: Logging
add_executable(test_logging measurement/test_logging.c measurement/mock_hal.c)
target_link_libraries(test_logging measure_lib cmocka)
add_test(NAME LoggingTest COMMAND test_logging)

enable_testing()
