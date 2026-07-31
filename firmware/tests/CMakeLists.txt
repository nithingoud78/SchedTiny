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

# Test executable: Scheduler
add_executable(test_scheduler 
    scheduler/test_scheduler.c 
    scheduler/mock_freertos.c 
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_core.c
)
# Define HOST_TEST to disable infinite loops in assertions
target_compile_definitions(test_scheduler PRIVATE HOST_TEST)
# Need include paths for FreeRTOS and Scheduler
target_include_directories(test_scheduler PRIVATE 
    ${CMAKE_SOURCE_DIR}/../core/scheduler 
    ${CMAKE_SOURCE_DIR}/../include 
    scheduler/mock_headers
)
target_link_libraries(test_scheduler cmocka)
add_test(NAME SchedulerTest COMMAND test_scheduler)

# Test executable: Task Registration
add_executable(test_task 
    scheduler/test_task.c 
    scheduler/mock_freertos.c 
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_task.c
)
target_compile_definitions(test_task PRIVATE HOST_TEST)
target_include_directories(test_task PRIVATE 
    ${CMAKE_SOURCE_DIR}/../core/scheduler 
    ${CMAKE_SOURCE_DIR}/../include 
    scheduler/mock_headers
)
target_link_libraries(test_task cmocka)
add_test(NAME TaskRegistrationTest COMMAND test_task)

# Test executable: Ready Queue
add_executable(test_ready_queue
    scheduler/test_ready_queue.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_ready_queue.c
)
target_compile_definitions(test_ready_queue PRIVATE HOST_TEST)
target_include_directories(test_ready_queue PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_ready_queue cmocka)
add_test(NAME ReadyQueueTest COMMAND test_ready_queue)

# Test executable: Scheduler Policy
add_executable(test_scheduler_policy
    scheduler/test_scheduler_policy.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_policy_hpf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_ready_queue.c
)
target_compile_definitions(test_scheduler_policy PRIVATE HOST_TEST)
target_include_directories(test_scheduler_policy PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_scheduler_policy cmocka)
add_test(NAME SchedulerPolicyTest COMMAND test_scheduler_policy)

enable_testing()
