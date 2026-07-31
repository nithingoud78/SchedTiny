/**
 * @file    sched_mc.h
 * @brief   SchedTiny Mixed-Criticality (MC) Scheduling Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_MC_H
#define SCHEDTINY_SCHED_MC_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

#ifndef SCHED_MC_CAPACITY
#define SCHED_MC_CAPACITY 128
#endif

#define SCHED_MC_NO_TASK UINT32_MAX

    /**
     * @brief   System criticality modes.
     */
    typedef enum
    {
        SCHED_MC_MODE_LO = 0,
        SCHED_MC_MODE_HI = 1
    } sched_mc_mode_t;

    /**
     * @brief   Task criticality levels.
     */
    typedef enum
    {
        SCHED_MC_CRIT_LO = 0,
        SCHED_MC_CRIT_HI = 1
    } sched_mc_criticality_t;

    /**
     * @brief   Mixed-criticality task structure.
     */
    typedef struct
    {
        uint32_t task_id;
        sched_mc_criticality_t criticality;
        uint32_t lo_wcet;
        uint32_t hi_wcet;
        uint32_t current_budget;
        uint32_t priority;
        uint32_t rel_deadline;
        bool active;
        bool dropped;
    } sched_mc_task_t;

    /**
     * @brief   Mixed-Criticality scheduler context.
     */
    typedef struct
    {
        sched_mc_mode_t mode;
        sched_mc_task_t registry[SCHED_MC_CAPACITY];
        bool initialized;

        /* Metrics */
        uint32_t mode_switch_count;
        uint32_t last_mode_switch_time;
        uint32_t hi_mode_entries;
        uint32_t total_hi_duration;
        uint32_t max_hi_duration;
        uint32_t dropped_lo_tasks;
        uint32_t recovered_lo_tasks;
    } sched_mc_t;

    /**
     * @brief   Initialize the MC scheduler context.
     */
    SchedStatus_t sched_mc_init(sched_mc_t *ctx);

    /**
     * @brief   Reset the MC scheduler context.
     */
    SchedStatus_t sched_mc_reset(sched_mc_t *ctx);

    /**
     * @brief   Register a task with MC parameters.
     */
    SchedStatus_t sched_mc_register(sched_mc_t *ctx,
                                    uint32_t task_id,
                                    sched_mc_criticality_t criticality,
                                    uint32_t lo_wcet,
                                    uint32_t hi_wcet,
                                    uint32_t priority,
                                    uint32_t rel_deadline);

    /**
     * @brief   Remove a task from the MC registry.
     */
    SchedStatus_t sched_mc_remove(sched_mc_t *ctx, uint32_t task_id);

    /**
     * @brief   Force set the system mode.
     */
    SchedStatus_t sched_mc_set_mode(sched_mc_t *ctx, sched_mc_mode_t mode, uint32_t current_time);

    /**
     * @brief   Get current system mode.
     */
    sched_mc_mode_t sched_mc_get_mode(const sched_mc_t *ctx);

    /**
     * @brief   Tick function. Consumes budget for the given task and handles
     * automatic mode switching.
     */
    SchedStatus_t sched_mc_tick(sched_mc_t *ctx, uint32_t active_task_id, uint32_t current_time);

    /**
     * @brief   Select the next task to dispatch.
     *          In LO mode: Highest priority active task (not dropped).
     *          In HI mode: Highest priority active HI task.
     */
    SchedStatus_t sched_mc_dispatch(sched_mc_t *ctx, uint32_t *out_task_id);

    /**
     * @brief   Reset the budget of a task, usually at release.
     */
    SchedStatus_t sched_mc_reset_budget(sched_mc_t *ctx, uint32_t task_id);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_MC_H */
