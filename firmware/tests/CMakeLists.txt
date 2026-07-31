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

# Test executable: Fault Injection
add_executable(test_fault
    scheduler/test_fault.c
    scheduler/mock_freertos.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_benchmark.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_stats.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_adaptive.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_trace.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_dispatcher.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_policy_hpf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_edf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_rms.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_mc.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_ready_queue.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_task.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_power.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_fault.c
)
target_compile_definitions(test_fault PRIVATE 
    HOST_TEST 
    SCHED_BENCHMARK_MAX_TASKS=128
    SCHED_READY_QUEUE_CAPACITY=128
    SCHED_EDF_CAPACITY=128
    SCHED_RMS_CAPACITY=128
    SCHED_MC_CAPACITY=128
)
target_include_directories(test_fault PRIVATE
    ${CMAKE_SOURCE_DIR}/../core/scheduler
    ${CMAKE_SOURCE_DIR}/../include
    scheduler/mock_headers
)
target_link_libraries(test_fault cmocka)
add_test(NAME FaultTest COMMAND test_fault)
add_test(NAME SchedulerTest COMMAND test_scheduler)

# Test executable: Adaptive Scheduler
add_executable(test_adaptive
    scheduler/test_adaptive.c
    scheduler/mock_freertos.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_adaptive.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_trace.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_benchmark.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_stats.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_dispatcher.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_policy_hpf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_edf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_rms.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_mc.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_ready_queue.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_task.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_power.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_fault.c
)
target_compile_definitions(test_adaptive PRIVATE 
    HOST_TEST 
    SCHED_BENCHMARK_MAX_TASKS=128
    SCHED_READY_QUEUE_CAPACITY=128
    SCHED_EDF_CAPACITY=128
    SCHED_RMS_CAPACITY=128
    SCHED_MC_CAPACITY=128
)
target_include_directories(test_adaptive PRIVATE
    ${CMAKE_SOURCE_DIR}/../core/scheduler
    ${CMAKE_SOURCE_DIR}/../include
    scheduler/mock_headers
)
target_link_libraries(test_adaptive cmocka)
add_test(NAME AdaptiveTest COMMAND test_adaptive)

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

# Test executable: Trace Framework
add_executable(test_trace
    scheduler/test_trace.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_trace.c
)
target_compile_definitions(test_trace PRIVATE HOST_TEST)
target_include_directories(test_trace PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_trace cmocka)
add_test(NAME TraceTest COMMAND test_trace)

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

# Test executable: EDF Scheduler
add_executable(test_edf
    scheduler/test_edf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_edf.c
)
target_compile_definitions(test_edf PRIVATE HOST_TEST)
target_include_directories(test_edf PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_edf cmocka)
add_test(NAME EdfSchedulerTest COMMAND test_edf)

# Test executable: RMS Scheduler
add_executable(test_rms
    scheduler/test_rms.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_rms.c
)
target_compile_definitions(test_rms PRIVATE HOST_TEST)
target_include_directories(test_rms PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_rms cmocka)
add_test(NAME RmsSchedulerTest COMMAND test_rms)

# Test executable: MC Scheduler
add_executable(test_mc
    scheduler/test_mc.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_mc.c
)
target_compile_definitions(test_mc PRIVATE HOST_TEST)
target_include_directories(test_mc PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_mc cmocka)
add_test(NAME McSchedulerTest COMMAND test_mc)

# Test executable: Runtime Statistics
add_executable(test_stats
    scheduler/test_stats.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_stats.c
)
target_compile_definitions(test_stats PRIVATE HOST_TEST)
target_include_directories(test_stats PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_stats cmocka)
add_test(NAME StatsTest COMMAND test_stats)

# Test executable: Dispatcher
add_executable(test_dispatcher
    scheduler/test_dispatcher.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_dispatcher.c
)
target_compile_definitions(test_dispatcher PRIVATE HOST_TEST)
target_include_directories(test_dispatcher PRIVATE
    ${CMAKE_SOURCE_DIR}/../include
)
target_link_libraries(test_dispatcher cmocka)
add_test(NAME DispatcherTest COMMAND test_dispatcher)

# Test executable: Benchmark Harness
add_executable(test_benchmark
    scheduler/test_benchmark.c
    scheduler/mock_freertos.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_benchmark.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_stats.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_adaptive.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_trace.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_dispatcher.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_policy_hpf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_edf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_rms.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_mc.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_ready_queue.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_task.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_power.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_fault.c
)
target_compile_definitions(test_benchmark PRIVATE 
    HOST_TEST 
    SCHED_BENCHMARK_MAX_TASKS=128
    SCHED_READY_QUEUE_CAPACITY=128
    SCHED_EDF_CAPACITY=128
    SCHED_RMS_CAPACITY=128
    SCHED_MC_CAPACITY=128
)
target_include_directories(test_benchmark PRIVATE
    ${CMAKE_SOURCE_DIR}/../core/scheduler
    ${CMAKE_SOURCE_DIR}/../include
    scheduler/mock_headers
)
target_link_libraries(test_benchmark cmocka)
add_test(NAME BenchmarkTest COMMAND test_benchmark)

# Standalone executable: Benchmark Runner
add_executable(benchmark_runner
    ${CMAKE_SOURCE_DIR}/../core/benchmark/benchmark_runner.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_benchmark.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_stats.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_adaptive.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_trace.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_dispatcher.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_policy_hpf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_edf.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_rms.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_mc.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_ready_queue.c
    ${CMAKE_SOURCE_DIR}/../core/scheduler/sched_task.c
    scheduler/stub_freertos.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_power.c
    ${CMAKE_SOURCE_DIR}/../core/benchmark/sched_fault.c
)
target_compile_definitions(benchmark_runner PRIVATE 
    HOST_TEST 
    SCHED_BENCHMARK_MAX_TASKS=128
    SCHED_READY_QUEUE_CAPACITY=128
    SCHED_EDF_CAPACITY=128
    SCHED_RMS_CAPACITY=128
    SCHED_MC_CAPACITY=128
)
target_include_directories(benchmark_runner PRIVATE
    ${CMAKE_SOURCE_DIR}/../core/scheduler
    ${CMAKE_SOURCE_DIR}/../include
    scheduler/mock_headers
)

enable_testing()
