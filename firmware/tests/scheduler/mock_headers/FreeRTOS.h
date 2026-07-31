#ifndef FREERTOS_H
#define FREERTOS_H

#include <stddef.h>
#include <stdint.h>

#define configMINIMAL_STACK_SIZE     128
#define configTIMER_TASK_STACK_DEPTH 128
#define configUSE_TIMERS             1

typedef uint32_t StackType_t;
typedef uint32_t UBaseType_t;

typedef struct tskTaskControlBlock *TaskHandle_t;

typedef struct xSTATIC_TCB
{
    void *dummy1;
} StaticTask_t;

#endif
