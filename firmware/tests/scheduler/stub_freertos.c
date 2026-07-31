/**
 * @file    stub_freertos.c
 * @brief   A lightweight FreeRTOS stub without CMocka dependencies.
 *          Used for standalone executables like benchmark_runner.
 */

#include "task.h"
#include "FreeRTOS.h"

void vTaskStartScheduler(void)
{
    /* No-op */
}

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void *const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t *const puxStackBuffer,
                               StaticTask_t *const pxTaskBuffer)
{
    (void)pxTaskCode;
    (void)pcName;
    (void)ulStackDepth;
    (void)pvParameters;
    (void)uxPriority;
    (void)puxStackBuffer;
    
    /* Return the pxTaskBuffer casted to TaskHandle_t as a dummy handle */
    return (TaskHandle_t)pxTaskBuffer;
}

void vTaskDelete(TaskHandle_t xTaskToDelete)
{
    (void)xTaskToDelete;
    /* No-op */
}

void vTaskPrioritySet(TaskHandle_t xTask, UBaseType_t uxNewPriority)
{
    (void)xTask;
    (void)uxNewPriority;
    /* No-op */
}

UBaseType_t uxTaskPriorityGet(const TaskHandle_t xTask)
{
    (void)xTask;
    return 0; /* Default priority */
}
