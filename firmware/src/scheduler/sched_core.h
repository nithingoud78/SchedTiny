/**
 * @file    sched_core.h
 * @brief   SchedTiny core scheduler interface.
 *
 * This module provides the top-level API for the SchedTiny interrupt-aware
 * scheduler. It integrates with FreeRTOS to register tasks, set scheduling
 * policies, and enable the measurement infrastructure.
 *
 * The scheduler is the primary research artifact of SchedTiny. It extends
 * FreeRTOS's fixed-priority preemptive scheduler with:
 *   - Explicit ISR registration and hook management (see sched_isr.h)
 *   - Policy abstraction (FPP, RM, EDF via sched_policy.h)
 *   - Per-task WCET tracking fed to the bench measurement layer
 *
 * @see     docs/SPEC.md REQ-SCHED-001, REQ-SCHED-002, REQ-SCHED-003
 * @see     docs/ARCHITECTURE.md Section 2.1
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_CORE_H
#define SCHEDTINY_SCHED_CORE_H

#include <stdint.h>
#include <stdbool.h>

#include "FreeRTOS.h"
#include "task.h"

#include "schedtiny_config.h"   /* Generated from configs/*.yaml */

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Public Constants
 * ========================================================================= */

/** Maximum number of tasks that can be registered with the scheduler. */
#define SCHED_MAX_TASKS         (SCHEDTINY_MAX_TASKS)

/** Task ID for the TinyML inference task. */
#define SCHED_TASK_ID_INFER     (0U)

/** Task ID for the PID control task. */
#define SCHED_TASK_ID_PID       (1U)

/** Task ID for the sensor acquisition task. */
#define SCHED_TASK_ID_SENSOR    (2U)

/* =========================================================================
 * Public Types
 * ========================================================================= */

/**
 * @brief   SchedTiny scheduler status codes.
 */
typedef enum
{
    SCHED_OK             = 0,   /**< Operation succeeded */
    SCHED_ERR_INVALID    = 1,   /**< Invalid argument */
    SCHED_ERR_FULL       = 2,   /**< Task registry full */
    SCHED_ERR_NOT_INIT   = 3,   /**< Scheduler not initialized */
    SCHED_ERR_BUSY       = 4,   /**< Scheduler busy */
} SchedStatus_t;

/**
 * @brief   Scheduling policy identifier.
 *
 * The active policy is selected at compile time via SCHEDTINY_POLICY
 * in schedtiny_config.h. Runtime policy switching is not supported
 * (it would invalidate schedulability analysis).
 */
typedef enum
{
    SCHED_POLICY_FPP = 0,   /**< Fixed-Priority Preemptive (FreeRTOS default) */
    SCHED_POLICY_RM  = 1,   /**< Rate-Monotonic (priorities assigned by period) */
    SCHED_POLICY_EDF = 2,   /**< Earliest Deadline First (requires EDF extension) */
} SchedPolicy_t;

/**
 * @brief   Per-task descriptor registered with the scheduler.
 */
typedef struct
{
    uint8_t          task_id;        /**< Unique task identifier (SCHED_TASK_ID_*) */
    TaskHandle_t     handle;         /**< FreeRTOS task handle */
    uint32_t         period_ms;      /**< Nominal period in milliseconds */
    uint32_t         wcet_budget_us; /**< Configured WCET budget in microseconds */
    uint32_t         deadline_ms;    /**< Relative deadline in milliseconds */
    bool             is_hard;        /**< true = hard deadline, false = soft */
} SchedTaskDesc_t;

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief   Initialize the SchedTiny scheduler.
 *
 * Must be called once from main() before creating any tasks.
 * Thread-safe: No (call before FreeRTOS scheduler starts).
 *
 * @return  SCHED_OK on success.
 */
SchedStatus_t sched_init(void);

/**
 * @brief   Register a task with the scheduler.
 *
 * Must be called for each task after xTaskCreate() but before
 * vTaskStartScheduler().
 *
 * @param[in] desc  Pointer to a populated task descriptor. Copied internally.
 * @return          SCHED_OK on success, SCHED_ERR_FULL if registry is full.
 */
SchedStatus_t sched_register_task(const SchedTaskDesc_t *desc);

/**
 * @brief   Return the active scheduling policy.
 *
 * The policy is compiled in; this function is for logging and diagnostics.
 *
 * @return  Active SchedPolicy_t value.
 */
SchedPolicy_t sched_get_policy(void);

/**
 * @brief   Run TinyML inference within a configurable CPU budget.
 *
 * This is the jitter-bounded inference API (REQ-SCHED-002).
 * The function runs TFLM inference and yields to higher-priority tasks
 * if the budget_us budget is exceeded during execution.
 *
 * @param[in] budget_us  CPU time budget in microseconds. Pass 0 for unlimited.
 * @return               true if inference completed within budget, false if
 *                       budget was exceeded (result may be partial).
 */
bool sched_infer_with_budget(uint32_t budget_us);

/**
 * @brief   Return the number of registered tasks.
 * @return  Number of tasks currently registered.
 */
uint8_t sched_get_task_count(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_CORE_H */
