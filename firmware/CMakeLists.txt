# SchedTiny Firmware CMakeLists.txt
#
# Build system for the SchedTiny embedded firmware.
# Supports three STM32 Nucleo boards; board is selected at configure time.
#
# Usage:
#   cmake -S firmware -B build \
#         -DCMAKE_TOOLCHAIN_FILE=../tools/cmake/arm-none-eabi.cmake \
#         -DBOARD=nucleo_h743zi2 \
#         -DCMAKE_BUILD_TYPE=Release
#
# Copyright (c) 2026 SchedTiny Contributors. Apache License 2.0.

cmake_minimum_required(VERSION 3.25)

# ---------------------------------------------------------------------------
# Project
# ---------------------------------------------------------------------------
project(SchedTiny
    VERSION 0.1.0
    DESCRIPTION "Interrupt-Aware Scheduling Framework for Co-Running Real-Time Control and TinyML"
    LANGUAGES C ASM
)

# Enforce C11
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# ---------------------------------------------------------------------------
# Board Selection
# ---------------------------------------------------------------------------
set(BOARD "nucleo_h743zi2" CACHE STRING "Target board (nucleo_h743zi2)")
set_property(CACHE BOARD PROPERTY STRINGS nucleo_h743zi2)

set(SUPPORTED_BOARDS nucleo_h743zi2)
if(NOT BOARD IN_LIST SUPPORTED_BOARDS)
    message(FATAL_ERROR "Unknown BOARD '${BOARD}'. Supported: ${SUPPORTED_BOARDS}")
endif()

message(STATUS "SchedTiny: Board = ${BOARD}")

# ---------------------------------------------------------------------------
# Configuration Code Generation
# ---------------------------------------------------------------------------
# CONFIG_FILE selects the experiment-specific YAML override.
# The Python script generates firmware/include/schedtiny_config.h.
set(CONFIG_FILE "${CMAKE_SOURCE_DIR}/../configs/tasks/benchmarks.yaml"
    CACHE FILEPATH "Path to experiment YAML configuration file")

find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(GENERATED_CONFIG_H "${CMAKE_BINARY_DIR}/generated/schedtiny_config.h")
set(GENERATED_CONFIG_C "${CMAKE_BINARY_DIR}/generated/schedtiny_config.c")

add_custom_command(
    OUTPUT ${GENERATED_CONFIG_H} ${GENERATED_CONFIG_C}
    COMMAND ${Python3_EXECUTABLE} -m pip install pyyaml || true
    COMMAND ${Python3_EXECUTABLE}
            "${CMAKE_SOURCE_DIR}/../scripts/build/generate_config.py"
            "--config" "${CONFIG_FILE}"
            "--out-header" "${GENERATED_CONFIG_H}"
            "--out-source" "${GENERATED_CONFIG_C}"
    DEPENDS "${CONFIG_FILE}" "${CMAKE_SOURCE_DIR}/../scripts/build/generate_config.py"
    COMMENT "Generating schedtiny_config.h/.c from ${CONFIG_FILE}"
    VERBATIM
)

add_custom_target(schedtiny_config_gen DEPENDS ${GENERATED_CONFIG_H})

# ---------------------------------------------------------------------------
# Reproducible Build Timestamp
# ---------------------------------------------------------------------------
if(NOT DEFINED ENV{SOURCE_DATE_EPOCH})
    message(STATUS "SOURCE_DATE_EPOCH not set. Build may not be bit-reproducible.")
endif()

# ---------------------------------------------------------------------------
# Board-Specific Configuration
# ---------------------------------------------------------------------------
include(boards/${BOARD}/CMakeLists.txt)

# BOARD_MCU, BOARD_LINKER_SCRIPT, BOARD_STARTUP_FILE, BOARD_DEFS are
# set by the board CMakeLists.txt above.

# ---------------------------------------------------------------------------
# Main Executable Target
# ---------------------------------------------------------------------------
add_executable(schedtiny
    main.c
    # Framework Core
    core/scheduler/sched_core.c
    core/scheduler/sched_task.c
    core/scheduler/sched_ready_queue.c
    core/scheduler/sched_policy_hpf.c
    core/scheduler/sched_edf.c
    core/scheduler/sched_rms.c
    core/scheduler/sched_dispatcher.c
    core/benchmark/sched_stats.c
    core/benchmark/sched_benchmark.c
    core/benchmark/sched_power.c
    core/bench/bench_measure.c
    core/bench/bench_log.c
    core/drivers/drv_timer.c
    core/drivers/drv_uart.c
    core/drivers/drv_gpio.c
    core/drivers/drv_power.c
    core/ml/model_runner.c
    core/ml/model_kws.c
    core/ml/model_anomaly.c
    # Benchmark Framework
    core/bench/bench_workloads.c
    core/bench/bench_runner.c
    ${GENERATED_CONFIG_C}
    # Board startup (set by board CMakeLists)
    ${BOARD_STARTUP_FILE}
)

add_dependencies(schedtiny schedtiny_config_gen)

# ---------------------------------------------------------------------------
# Include Directories
# ---------------------------------------------------------------------------
target_include_directories(schedtiny PRIVATE
    include/                           # SchedTiny public headers
    ${CMAKE_BINARY_DIR}/generated/     # Generated schedtiny_config.h
    third_party/tflite-micro/
    third_party/CMSIS-NN/Include/
    boards/${BOARD}/                   # Board-specific headers
)

# ---------------------------------------------------------------------------
# Preprocessor Definitions
# ---------------------------------------------------------------------------
target_compile_definitions(schedtiny PRIVATE
    ${BOARD_DEFS}                      # Board-specific defines (from board CMakeLists)
    USE_HAL_DRIVER
    $<$<CONFIG:Debug>:SCHEDTINY_DEBUG=1>
    $<$<CONFIG:Release>:NDEBUG>
)

# ---------------------------------------------------------------------------
# Compiler Flags
# ---------------------------------------------------------------------------
target_compile_options(schedtiny PRIVATE
    ${BOARD_CPU_FLAGS}                 # -mcpu=cortex-m7 -mfpu=fpv5-d16 etc.
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter              # FreeRTOS callbacks have unused params
    -ffunction-sections
    -fdata-sections
    -fno-common
    -ffreestanding
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-Og -g3 -gdwarf-4>
)

# ---------------------------------------------------------------------------
# Linker
# ---------------------------------------------------------------------------
target_link_options(schedtiny PRIVATE
    -T${BOARD_LINKER_SCRIPT}
    ${BOARD_CPU_FLAGS}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/schedtiny.map
    -Wl,--print-memory-usage
    --specs=nano.specs
    --specs=nosys.specs
    -lc
    -lm
)

# ---------------------------------------------------------------------------
# Post-build: Generate binary and hex
# ---------------------------------------------------------------------------
add_custom_command(TARGET schedtiny POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:schedtiny>
            ${CMAKE_BINARY_DIR}/schedtiny.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:schedtiny>
            ${CMAKE_BINARY_DIR}/schedtiny.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:schedtiny>
    COMMENT "Generating binary and HEX files"
    VERBATIM
)

# ---------------------------------------------------------------------------
# Third-party libraries (FetchContent)
# ---------------------------------------------------------------------------
include(FetchContent)

# Fetch FreeRTOS Kernel
FetchContent_Declare(
  freertos_kernel
  GIT_REPOSITORY https://github.com/FreeRTOS/FreeRTOS-Kernel.git
  GIT_TAG        V11.0.1
)
FetchContent_MakeAvailable(freertos_kernel)

# Create an interface library for FreeRTOS
add_library(freertos_lib INTERFACE)
target_include_directories(freertos_lib INTERFACE
    ${freertos_kernel_SOURCE_DIR}/include
    ${freertos_kernel_SOURCE_DIR}/portable/GCC/ARM_CM7/r0p1
)

# Core FreeRTOS sources
set(FREERTOS_SOURCES
    ${freertos_kernel_SOURCE_DIR}/list.c
    ${freertos_kernel_SOURCE_DIR}/queue.c
    ${freertos_kernel_SOURCE_DIR}/tasks.c
    ${freertos_kernel_SOURCE_DIR}/timers.c
    ${freertos_kernel_SOURCE_DIR}/portable/GCC/ARM_CM7/r0p1/port.c
    # No heap_x.c since we are strictly using static allocation
)

# Placeholder for TinyML
add_library(tflm_lib INTERFACE)

# Add FreeRTOS sources to schedtiny
target_sources(schedtiny PRIVATE ${FREERTOS_SOURCES})
target_link_libraries(schedtiny PRIVATE freertos_lib tflm_lib)

# ---------------------------------------------------------------------------
# Print configuration summary
# ---------------------------------------------------------------------------
message(STATUS "")
message(STATUS "=== SchedTiny Build Configuration ===")
message(STATUS "  Board:       ${BOARD}")
message(STATUS "  Build type:  ${CMAKE_BUILD_TYPE}")
message(STATUS "  Config file: ${CONFIG_FILE}")
message(STATUS "  Toolchain:   ${CMAKE_C_COMPILER}")
message(STATUS "======================================")
message(STATUS "")

# ---------------------------------------------------------------------------
# STM32 Hardware Demonstration Target
# ---------------------------------------------------------------------------
add_executable(stm32_scheduler_demo
    ${CMAKE_SOURCE_DIR}/../examples/stm32_scheduler_demo/main.c
    platform/stm32/platform_init.c
    platform/stm32/platform_timer.c
    platform/stm32/platform_gpio.c
    platform/stm32/platform_clock.c
    # Framework Core
    core/scheduler/sched_core.c
    core/scheduler/sched_task.c
    core/scheduler/sched_ready_queue.c
    core/scheduler/sched_policy_hpf.c
    core/scheduler/sched_edf.c
    core/scheduler/sched_rms.c
    core/scheduler/sched_dispatcher.c
    core/benchmark/sched_stats.c
    core/benchmark/sched_benchmark.c
    core/benchmark/sched_power.c
    ${GENERATED_CONFIG_C}
    ${BOARD_STARTUP_FILE}
)

add_dependencies(stm32_scheduler_demo schedtiny_config_gen)

target_include_directories(stm32_scheduler_demo PRIVATE
    include/
    platform/stm32/
    ${CMAKE_BINARY_DIR}/generated/
    third_party/tflite-micro/
    third_party/CMSIS-NN/Include/
    boards/${BOARD}/
)

target_compile_definitions(stm32_scheduler_demo PRIVATE
    ${BOARD_DEFS}
    USE_HAL_DRIVER
    STM32
    $<$<CONFIG:Debug>:SCHEDTINY_DEBUG=1>
    $<$<CONFIG:Release>:NDEBUG>
)

target_compile_options(stm32_scheduler_demo PRIVATE
    ${BOARD_CPU_FLAGS}
    -Wall
    -Wextra
    -Werror
    -Wno-unused-parameter
    -ffunction-sections
    -fdata-sections
    -fno-common
    -ffreestanding
    $<$<CONFIG:Release>:-O2>
    $<$<CONFIG:Debug>:-Og -g3 -gdwarf-4>
)

target_link_options(stm32_scheduler_demo PRIVATE
    -T${BOARD_LINKER_SCRIPT}
    ${BOARD_CPU_FLAGS}
    -Wl,--gc-sections
    -Wl,-Map=${CMAKE_BINARY_DIR}/stm32_scheduler_demo.map
    -Wl,--print-memory-usage
    --specs=nano.specs
    --specs=nosys.specs
    -lc
    -lm
)

target_sources(stm32_scheduler_demo PRIVATE ${FREERTOS_SOURCES})
target_link_libraries(stm32_scheduler_demo PRIVATE freertos_lib)

add_custom_command(TARGET stm32_scheduler_demo POST_BUILD
    COMMAND ${CMAKE_OBJCOPY} -O binary $<TARGET_FILE:stm32_scheduler_demo>
            ${CMAKE_BINARY_DIR}/stm32_scheduler_demo.bin
    COMMAND ${CMAKE_OBJCOPY} -O ihex $<TARGET_FILE:stm32_scheduler_demo>
            ${CMAKE_BINARY_DIR}/stm32_scheduler_demo.hex
    COMMAND ${CMAKE_SIZE} $<TARGET_FILE:stm32_scheduler_demo>
    COMMENT "Generating binary and HEX files for stm32_scheduler_demo"
    VERBATIM
)

