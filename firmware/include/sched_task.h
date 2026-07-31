/**
 * @file    sched_task.h
 * @brief   SchedTiny Task Registration and Lookup API.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 */

#ifndef SCHEDTINY_SCHED_TASK_H
#define SCHEDTINY_SCHED_TASK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"
#include "task.h"

#include "FreeRTOS.h"

/**
 * @brief   Maximum number of tasks the SchedTiny framework supports.
 */
#ifndef SCHED_MAX_TASKS
#define SCHED_MAX_TASKS 8
#endif

    /**
     * @brief   Task lifecycle states within the SchedTiny registry.
     */
    typedef enum
    {
        SCHED_TASK_STATE_UNUSED = 0,
        SCHED_TASK_STATE_READY,
        SCHED_TASK_STATE_RUNNING,
        SCHED_TASK_STATE_SUSPENDED,
        SCHED_TASK_STATE_DELETED
    } sched_task_state_t;

    /**
     * @brief   Task configuration and control block mapping.
     */
    typedef struct
    {
        uint32_t task_id;            /**< Unique ID assigned to the task */
        const char *name;            /**< Task name */
        TaskFunction_t pxTaskCode;   /**< Task callback function */
        void *pvParameters;          /**< Parameters passed to the task */
        uint32_t ulStackDepth;       /**< Stack depth (words) */
        UBaseType_t uxPriority;      /**< Task priority */
        StackType_t *puxStackBuffer; /**< Pointer to statically allocated stack */
        StaticTask_t *pxTaskBuffer;  /**< Pointer to statically allocated TCB */
        TaskHandle_t handle;         /**< Assigned FreeRTOS task handle */
        sched_task_state_t state;    /**< Internal lifecycle state */
    } sched_task_t;

    /**
     * @brief   Register a task in the framework and spawn it via FreeRTOS.
     * @param   config Pointer to the populated sched_task_t structure.
     * @return  SCHED_OK on success, SCHED_ERR_NO_MEM if limit reached, or SCHED_ERR_PARAM.
     */
    SchedStatus_t sched_task_register(const sched_task_t *config);

    /**
     * @brief   Remove a task from the framework registry.
     * @param   task_id The unique ID of the task.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if invalid ID.
     */
    SchedStatus_t sched_task_remove(uint32_t task_id);

    /**
     * @brief   Lookup a task by its ID.
     * @param   task_id The unique ID of the task.
     * @param   out_task Pointer to structure where task data will be copied.
     * @return  SCHED_OK on success, SCHED_ERR_NOT_FOUND if invalid ID.
     */
    SchedStatus_t sched_task_get(uint32_t task_id, sched_task_t *out_task);

    /**
     * @brief   Get all currently registered tasks.
     * @param   out_tasks Array buffer to hold tasks (must be sized to SCHED_MAX_TASKS).
     * @param   count Pointer to store the number of active tasks found.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_task_get_all(sched_task_t *out_tasks, uint32_t *count);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_TASK_H */
