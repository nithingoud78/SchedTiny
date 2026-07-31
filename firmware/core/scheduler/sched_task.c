/**
 * @file    sched_task.c
 * @brief   SchedTiny Task Registration, Lookup, and Priority Implementation.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.2.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#include "sched_task.h"

#include "sched_trace.h"
#include "task.h"

#include "FreeRTOS.h"

#include <string.h>

/* Static registry for managing up to SCHED_MAX_TASKS */
static sched_task_t g_task_registry[SCHED_MAX_TASKS];

SchedStatus_t sched_task_register(const sched_task_t *config)
{
    if (config == NULL || config->pxTaskCode == NULL || config->puxStackBuffer == NULL ||
        config->pxTaskBuffer == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    /* Verify if task_id already exists */
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state != SCHED_TASK_STATE_UNUSED &&
            g_task_registry[i].task_id == config->task_id)
        {
            return SCHED_ERR_PARAM; /* ID collision */
        }
    }

    /* Find an empty slot */
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state == SCHED_TASK_STATE_UNUSED)
        {
            /* Copy configuration */
            memcpy(&g_task_registry[i], config, sizeof(sched_task_t));

            /* Create task statically */
            g_task_registry[i].handle = xTaskCreateStatic(
                config->pxTaskCode, config->name ? config->name : "Task", config->ulStackDepth,
                config->pvParameters, (UBaseType_t)config->priority, config->puxStackBuffer,
                config->pxTaskBuffer);

            if (g_task_registry[i].handle == NULL)
            {
                /* Reset state if static allocation fails (should rarely happen unless pointers are
                 * invalid) */
                memset(&g_task_registry[i], 0, sizeof(sched_task_t));
                return SCHED_ERR_NO_MEM;
            }

            g_task_registry[i].state = SCHED_TASK_STATE_READY;

#if SCHED_CONFIG_ENABLE_TRACE
            sched_trace_record(
                xTaskGetTickCount(), SCHED_TRACE_EVT_TASK_CREATE, config->task_id, 0, /* policy */
                0,                                                                    /* core_id */
                config->criticality, config->priority, config->deadline, config->wcet, 0);
#endif

            return SCHED_OK;
        }
    }

    /* Registry is full */
    return SCHED_ERR_NO_MEM;
}

SchedStatus_t sched_task_remove(uint32_t task_id)
{
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state != SCHED_TASK_STATE_UNUSED &&
            g_task_registry[i].task_id == task_id)
        {
            if (g_task_registry[i].handle != NULL)
            {
                /* In FreeRTOS, passing NULL deletes the calling task.
                 * Pass the specific handle here. */
                vTaskDelete(g_task_registry[i].handle);
            }

            /* Clear registry entry */
            memset(&g_task_registry[i], 0, sizeof(sched_task_t));
            g_task_registry[i].state = SCHED_TASK_STATE_UNUSED;

#if SCHED_CONFIG_ENABLE_TRACE
            sched_trace_record(xTaskGetTickCount(), SCHED_TRACE_EVT_TASK_DELETE, task_id, 0, 0, 0,
                               0, 0, 0, 0);
#endif

            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_task_get(uint32_t task_id, sched_task_t *out_task)
{
    if (out_task == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state != SCHED_TASK_STATE_UNUSED &&
            g_task_registry[i].task_id == task_id)
        {
            memcpy(out_task, &g_task_registry[i], sizeof(sched_task_t));
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_task_get_all(sched_task_t *out_tasks, uint32_t *count)
{
    if (out_tasks == NULL || count == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    uint32_t found = 0;
    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state != SCHED_TASK_STATE_UNUSED)
        {
            memcpy(&out_tasks[found], &g_task_registry[i], sizeof(sched_task_t));
            found++;
        }
    }

    *count = found;
    return SCHED_OK;
}

SchedStatus_t sched_task_set_priority(uint32_t task_id, sched_priority_t priority)
{
    if (priority > SCHED_PRIORITY_MAX)
    {
        return SCHED_ERR_PARAM;
    }

    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state != SCHED_TASK_STATE_UNUSED &&
            g_task_registry[i].task_id == task_id)
        {
            g_task_registry[i].priority = priority;

            if (g_task_registry[i].handle != NULL)
            {
                vTaskPrioritySet(g_task_registry[i].handle, (UBaseType_t)priority);
            }

            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_task_get_priority(uint32_t task_id, sched_priority_t *out_priority)
{
    if (out_priority == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    for (uint32_t i = 0; i < SCHED_MAX_TASKS; i++)
    {
        if (g_task_registry[i].state != SCHED_TASK_STATE_UNUSED &&
            g_task_registry[i].task_id == task_id)
        {
            *out_priority = g_task_registry[i].priority;
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}
