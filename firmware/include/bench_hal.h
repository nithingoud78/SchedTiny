/**
 * @file    bench_hal.h
 * @brief   SchedTiny benchmark hardware abstraction layer.
 *
 * Provides compile-time configurations for STM32 variants to ensure
 * accurate DWT cycle conversion and latency measurements.
 *
 * @author  @nithingoud78
 * @date    2026-08-01
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_BENCH_HAL_H
#define SCHEDTINY_BENCH_HAL_H

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * Target MCU Selection.
 * If none is provided by the build system, default to generic simulation at 480 MHz.
 */
#if defined(STM32F4)
#define BENCH_CORE_CLOCK_MHZ 168
#define BENCH_MCU_VARIANT    "STM32F4"
#elif defined(STM32F7)
#define BENCH_CORE_CLOCK_MHZ 216
#define BENCH_MCU_VARIANT    "STM32F7"
#elif defined(STM32H7)
#define BENCH_CORE_CLOCK_MHZ 480
#define BENCH_MCU_VARIANT    "STM32H7"
#else
#define BENCH_CORE_CLOCK_MHZ 480
#define BENCH_MCU_VARIANT    "SIMULATION_OR_UNKNOWN"
#endif

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_BENCH_HAL_H */
