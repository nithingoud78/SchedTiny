/*
 * FreeRTOS V11.0.1 Configuration
 *
 * SchedTiny Milestone 2: RTOS Platform Integration.
 * Configured conservatively for zero heap fragmentation and maximum observability.
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "bench_measure.h"

#include <stdint.h>

/*-----------------------------------------------------------
 * Core Settings
 *----------------------------------------------------------*/
#define configUSE_PREEMPTION                    1
#define configUSE_TICKLESS_IDLE                 0
#define configCPU_CLOCK_HZ                      (480000000)  // STM32H743ZI2 Core Clock
#define configTICK_RATE_HZ                      ((TickType_t)1000)
#define configMAX_PRIORITIES                    (5)
#define configMINIMAL_STACK_SIZE                ((uint16_t)128)
#define configMAX_TASK_NAME_LEN                 (16)
#define configUSE_16_BIT_TICKS                  0
#define configIDLE_SHOULD_YIELD                 1
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               8
#define configUSE_QUEUE_SETS                    0
#define configUSE_TIME_SLICING                  0  // Disable round-robin for precise control tests
#define configUSE_NEWLIB_REENTRANT              0
#define configENABLE_BACKWARD_COMPATIBILITY     0
#define configNUM_THREAD_LOCAL_STORAGE_POINTERS 0

/*-----------------------------------------------------------
 * Memory allocation - Strictly Static Only
 *----------------------------------------------------------*/
#define configSUPPORT_STATIC_ALLOCATION  1
#define configSUPPORT_DYNAMIC_ALLOCATION 0

/*-----------------------------------------------------------
 * Hook function related definitions.
 *----------------------------------------------------------*/
#define configUSE_IDLE_HOOK            1
#define configUSE_TICK_HOOK            1
#define configCHECK_FOR_STACK_OVERFLOW 2
#define configUSE_MALLOC_FAILED_HOOK \
    1  // Technically unneeded without dynamic alloc, but kept for safety
#define configUSE_DAEMON_TASK_STARTUP_HOOK 0

/*-----------------------------------------------------------
 * Run time and task stats gathering related definitions.
 *----------------------------------------------------------*/
#define configGENERATE_RUN_TIME_STATS        1
#define configUSE_TRACE_FACILITY             1
#define configUSE_STATS_FORMATTING_FUNCTIONS 0  // We will format our own JSON outputs via bench_log

// Map the Run Time Stats directly to our highly accurate DWT cycle counter
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()
#define portGET_RUN_TIME_COUNTER_VALUE() ((uint32_t)bench_cycles_to_us(bench_dwt_cycles64()))

/*-----------------------------------------------------------
 * Co-routine related definitions.
 *----------------------------------------------------------*/
#define configUSE_CO_ROUTINES           0
#define configMAX_CO_ROUTINE_PRIORITIES 1

/*-----------------------------------------------------------
 * Software timer related definitions.
 *----------------------------------------------------------*/
#define configUSE_TIMERS             1
#define configTIMER_TASK_PRIORITY    (configMAX_PRIORITIES - 1)
#define configTIMER_QUEUE_LENGTH     10
#define configTIMER_TASK_STACK_DEPTH configMINIMAL_STACK_SIZE

/*-----------------------------------------------------------
 * Interrupt nesting behaviour configuration.
 *----------------------------------------------------------*/
// STM32H7 specific NVIC properties
#define configPRIO_BITS 4

#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY      15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY 5

#define configKERNEL_INTERRUPT_PRIORITY \
    (configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))
#define configMAX_SYSCALL_INTERRUPT_PRIORITY \
    (configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS))

/*-----------------------------------------------------------
 * Assertions
 *----------------------------------------------------------*/
extern void vAssertCalled(const char *pcFile, uint32_t ulLine);
#define configASSERT(x) \
    if ((x) == 0)       \
    vAssertCalled(__FILE__, __LINE__)

/*-----------------------------------------------------------
 * FreeRTOS MPU specific definitions.
 *----------------------------------------------------------*/
#define configINCLUDE_APPLICATION_DEFINED_PRIVILEGED_FUNCTIONS 0
#define configTOTAL_MPU_REGIONS                                8
#define configTEX_S_C_B_FLASH                                  0x07UL
#define configTEX_S_C_B_SRAM                                   0x07UL
#define configENFORCE_SYSTEM_CALLS_FROM_KERNEL_ONLY            1

/*-----------------------------------------------------------
 * Optional functions
 *----------------------------------------------------------*/
#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_xResumeFromISR              1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetSchedulerState      1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_uxTaskGetStackHighWaterMark 1
#define INCLUDE_xTaskGetIdleTaskHandle      0
#define INCLUDE_eTaskGetState               0
#define INCLUDE_xEventGroupSetBitFromISR    1
#define INCLUDE_xTimerPendFunctionCall      0
#define INCLUDE_xTaskAbortDelay             0
#define INCLUDE_xTaskGetHandle              0
#define INCLUDE_xTaskResumeFromISR          1

/*-----------------------------------------------------------
 * Map FreeRTOS interrupts to CMSIS standard names
 *----------------------------------------------------------*/
#define vPortSVCHandler     SVC_Handler
#define xPortPendSVHandler  PendSV_Handler
#define xPortSysTickHandler SysTick_Handler

#endif /* FREERTOS_CONFIG_H */
