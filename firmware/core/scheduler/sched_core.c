/**
 * @file    sched_core.c
 * @brief   SchedTiny Core RTOS Hooks and Static Allocation Management.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "bench_measure.h"
#include "task.h"

#include "FreeRTOS.h"

#include <stdio.h>

/*-----------------------------------------------------------
 * STATIC ALLOCATION BUFFERS FOR RTOS DAEMONS
 *----------------------------------------------------------*/

// Idle Task Memory
static StaticTask_t xIdleTaskTCB;
static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize)
{
    *ppxIdleTaskTCBBuffer   = &xIdleTaskTCB;
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;
    *pulIdleTaskStackSize   = configMINIMAL_STACK_SIZE;
}

// Timer Task Memory (if configUSE_TIMERS == 1)
#if (configUSE_TIMERS == 1)
static StaticTask_t xTimerTaskTCB;
static StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize)
{
    *ppxTimerTaskTCBBuffer   = &xTimerTaskTCB;
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;
    *pulTimerTaskStackSize   = configTIMER_TASK_STACK_DEPTH;
}
#endif

/*-----------------------------------------------------------
 * SYSTEM HOOKS
 *----------------------------------------------------------*/

void vApplicationIdleHook(void)
{
    // The Idle hook allows us to periodically flush logs when CPU is unused.
    // However, the dedicated Logger task handles this right now.
}

void vApplicationTickHook(void)
{
    // Executes inside the SysTick ISR.
    // Reserved for Milestone 4 (interrupt-aware scheduling logic).
}

/*-----------------------------------------------------------
 * FATAL ERROR HOOKS
 *----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;

    // Log fatal error bypassing ring buffer directly if possible, or halt.
    // In production, we trigger a breakpoint or system reset.
    __asm volatile("bkpt #0");
    for (;;)
        ;
}

void vApplicationMallocFailedHook(void)
{
    // Should never execute because configSUPPORT_DYNAMIC_ALLOCATION is 0.
    __asm volatile("bkpt #0");
    for (;;)
        ;
}

void vAssertCalled(const char *pcFile, uint32_t ulLine)
{
    (void)pcFile;
    (void)ulLine;

    // Catch-all assertion trap.
    __asm volatile("bkpt #0");
    for (;;)
        ;
}
