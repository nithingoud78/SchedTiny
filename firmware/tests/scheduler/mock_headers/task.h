#ifndef TASK_H
#define TASK_H

#include "FreeRTOS.h"

void vTaskStartScheduler(void);

typedef void (*TaskFunction_t)(void *);

TaskHandle_t xTaskCreateStatic(TaskFunction_t pxTaskCode,
                               const char *const pcName,
                               const uint32_t ulStackDepth,
                               void *const pvParameters,
                               UBaseType_t uxPriority,
                               StackType_t *const puxStackBuffer,
                               StaticTask_t *const pxTaskBuffer);

void vTaskDelete(TaskHandle_t xTaskToDelete);

#endif
