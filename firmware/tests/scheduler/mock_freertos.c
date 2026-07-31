#include "task.h"

#include "FreeRTOS.h"

// clang-format off
#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <stdint.h>
#include <cmocka.h>
// clang-format on

void vTaskStartScheduler(void)
{
    function_called();
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
    (void)pxTaskBuffer;

    return (TaskHandle_t)mock();
}

void vTaskDelete(TaskHandle_t xTaskToDelete)
{
    check_expected(xTaskToDelete);
}

void vTaskPrioritySet(TaskHandle_t xTask, UBaseType_t uxNewPriority)
{
    check_expected(xTask);
    check_expected(uxNewPriority);
}

UBaseType_t uxTaskPriorityGet(const TaskHandle_t xTask)
{
    (void)xTask;
    return (UBaseType_t)mock();
}
