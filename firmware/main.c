/**
 * @file    main.c
 * @brief   SchedTiny Firmware Entry Point (Milestone 2 - RTOS Integration).
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "FreeRTOS.h"
#include "task.h"
#include "sched_core.h"
#include "bench_measure.h"
#include "bench_log.h"
#include "drv_gpio.h"
#include "drv_timer.h"
#include "bench_runner.h"
#include "schedtiny_config.h"

// ----------------------------------------------------------------------------
// STACK & TCB DEFINITIONS (Strict Static Allocation)
// ----------------------------------------------------------------------------

#define SYSTEM_TASK_STACK_SIZE      512
#define LOGGER_TASK_STACK_SIZE      512

static StaticTask_t xSystemTaskTCB;
static StackType_t uxSystemTaskStack[SYSTEM_TASK_STACK_SIZE];

static StaticTask_t xLoggerTaskTCB;
static StackType_t uxLoggerTaskStack[LOGGER_TASK_STACK_SIZE];

TaskHandle_t xSystemTaskHandle = NULL;
TaskHandle_t xLoggerTaskHandle = NULL;

// ----------------------------------------------------------------------------
// TASK DECLARATIONS
// ----------------------------------------------------------------------------

/**
 * @brief   Logger Task
 *
 * Runs at low priority just above Idle. Flushes the high-speed measurement
 * ring buffer to UART JSON asynchronously.
 */
void vLoggerTask(void *pvParameters)
{
    (void)pvParameters;
    const TickType_t xPollRate = pdMS_TO_TICKS(50); // Poll every 50ms
    
    for(;;)
    {
        bench_log_flush();
        vTaskDelay(xPollRate);
    }
}

/**
 * @brief   System Task
 *
 * Executes immediately upon scheduler start. Initializes hardware dependencies,
 * spawns background tasks, and then suspends itself.
 */
void vSystemTask(void *pvParameters)
{
    (void)pvParameters;

    // 1. Initialize hardware subsystems
    drv_gpio_init();
    bench_init();
    bench_log_init();

    // 2. Spawn Static Application Tasks (Logger & Benchmarks)
    xLoggerTaskHandle = xTaskCreateStatic(
        vLoggerTask,
        "Logger",
        LOGGER_TASK_STACK_SIZE,
        NULL,
        1, // Low priority (above idle)
        uxLoggerTaskStack,
        &xLoggerTaskTCB
    );

    // Spawn dynamically generated benchmark workloads
    bench_runner_spawn_all(g_BenchmarkTasks, SCHEDTINY_MAX_TASKS);

    // 3. System initialization complete. Suspend self indefinitely.
    vTaskSuspend(NULL);
}

// ----------------------------------------------------------------------------
// ENTRY POINT
// ----------------------------------------------------------------------------

int main(void)
{
    // Note: STM32 HAL initialization (HAL_Init) and SystemClock_Config() 
    // are typically handled here or in a board-specific startup wrapper before main.

    // Create the System Task statically
    xSystemTaskHandle = xTaskCreateStatic(
        vSystemTask,
        "System",
        SYSTEM_TASK_STACK_SIZE,
        NULL,
        (configMAX_PRIORITIES - 1), // Highest priority initially
        uxSystemTaskStack,
        &xSystemTaskTCB
    );

    // Start the FreeRTOS Scheduler
    vTaskStartScheduler();

    // Should never reach here if configSUPPORT_STATIC_ALLOCATION is used 
    // and Idle/Timer tasks are spawned successfully.
    for(;;);
    
    return 0;
}
