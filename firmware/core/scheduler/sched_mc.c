/**
 * @file    sched_mc.c
 * @brief   SchedTiny Mixed-Criticality (MC) Scheduling Policy.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_mc.h"

#include <string.h>

SchedStatus_t sched_mc_init(sched_mc_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_mc_t));
    ctx->mode        = SCHED_MC_MODE_LO;
    ctx->initialized = true;

    return SCHED_OK;
}

SchedStatus_t sched_mc_reset(sched_mc_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    memset(ctx->registry, 0, sizeof(ctx->registry));
    ctx->mode = SCHED_MC_MODE_LO;

    ctx->mode_switch_count     = 0;
    ctx->last_mode_switch_time = 0;
    ctx->hi_mode_entries       = 0;
    ctx->total_hi_duration     = 0;
    ctx->max_hi_duration       = 0;
    ctx->dropped_lo_tasks      = 0;
    ctx->recovered_lo_tasks    = 0;

    return SCHED_OK;
}

SchedStatus_t sched_mc_register(sched_mc_t *ctx,
                                uint32_t task_id,
                                sched_mc_criticality_t criticality,
                                uint32_t lo_wcet,
                                uint32_t hi_wcet,
                                uint32_t priority,
                                uint32_t rel_deadline)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }
    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Check for duplicate or empty slot */
    int empty_slot = -1;
    for (int i = 0; i < SCHED_MC_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            /* Duplicate */
            return SCHED_ERR_PARAM;
        }
        if (!ctx->registry[i].active && empty_slot == -1)
        {
            empty_slot = i;
        }
    }

    if (empty_slot == -1)
    {
        return SCHED_ERR_OVERFLOW;
    }

    sched_mc_task_t *t = &ctx->registry[empty_slot];
    t->task_id         = task_id;
    t->criticality     = criticality;
    t->lo_wcet         = lo_wcet;
    t->hi_wcet         = hi_wcet;
    t->priority        = priority;
    t->rel_deadline    = rel_deadline;
    t->current_budget  = lo_wcet; /* Initially assumes LO WCET */
    t->active          = true;
    t->dropped         = false;

    return SCHED_OK;
}

SchedStatus_t sched_mc_remove(sched_mc_t *ctx, uint32_t task_id)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }
    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    for (int i = 0; i < SCHED_MC_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            ctx->registry[i].active = false;
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_mc_set_mode(sched_mc_t *ctx, sched_mc_mode_t mode, uint32_t current_time)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }
    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (ctx->mode == mode)
    {
        return SCHED_OK;
    }

    if (mode == SCHED_MC_MODE_HI)
    {
        /* Switch LO -> HI */
        ctx->mode = SCHED_MC_MODE_HI;
        ctx->mode_switch_count++;
        ctx->hi_mode_entries++;
        ctx->last_mode_switch_time = current_time;

        /* Drop all LO tasks */
        for (int i = 0; i < SCHED_MC_CAPACITY; i++)
        {
            if (ctx->registry[i].active && ctx->registry[i].criticality == SCHED_MC_CRIT_LO)
            {
                if (!ctx->registry[i].dropped)
                {
                    ctx->registry[i].dropped = true;
                    ctx->dropped_lo_tasks++;
                }
            }
        }
    }
    else
    {
        /* Switch HI -> LO */
        ctx->mode = SCHED_MC_MODE_LO;
        ctx->mode_switch_count++;
        uint32_t duration = current_time - ctx->last_mode_switch_time;
        ctx->total_hi_duration += duration;
        if (duration > ctx->max_hi_duration)
        {
            ctx->max_hi_duration = duration;
        }

        /* Recover all LO tasks */
        for (int i = 0; i < SCHED_MC_CAPACITY; i++)
        {
            if (ctx->registry[i].active && ctx->registry[i].criticality == SCHED_MC_CRIT_LO)
            {
                if (ctx->registry[i].dropped)
                {
                    ctx->registry[i].dropped = false;
                    ctx->recovered_lo_tasks++;
                }
            }
        }
    }

    return SCHED_OK;
}

sched_mc_mode_t sched_mc_get_mode(const sched_mc_t *ctx)
{
    if (ctx == NULL || !ctx->initialized)
    {
        return SCHED_MC_MODE_LO;
    }
    return ctx->mode;
}

SchedStatus_t sched_mc_tick(sched_mc_t *ctx, uint32_t active_task_id, uint32_t current_time)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }
    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    if (active_task_id == SCHED_MC_NO_TASK)
    {
        /* Idle tick. If we are in HI mode and no HI tasks are active (or all
         * finished), we could recover back to LO mode. For this static
         * implementation, we will use "Idle-time recovery": if the system is idle,
         * we can safely revert to LO mode.
         */
        if (ctx->mode == SCHED_MC_MODE_HI)
        {
            sched_mc_set_mode(ctx, SCHED_MC_MODE_LO, current_time);
        }
        return SCHED_OK;
    }

    /* Find active task and consume budget */
    for (int i = 0; i < SCHED_MC_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == active_task_id)
        {
            sched_mc_task_t *t = &ctx->registry[i];

            /* Budget logic */
            if (t->current_budget > 0)
            {
                /* Decrease available budget implicitly, or track elapsed time.
                 * Usually, current_budget is how much time is left.
                 * Or we track execution time from 0 upwards.
                 * Let's say current_budget tracks time REMAINING of the LO budget.
                 */
                t->current_budget--;
            }

            if (t->current_budget == 0)
            {
                if (t->criticality == SCHED_MC_CRIT_HI && ctx->mode == SCHED_MC_MODE_LO)
                {
                    /* Time exceeded LO WCET! Switch to HI mode */
                    sched_mc_set_mode(ctx, SCHED_MC_MODE_HI, current_time);
                }
                else if (t->criticality == SCHED_MC_CRIT_LO)
                {
                    /* LO task exceeded budget, it should be dropped (aborted) */
                    t->dropped = true;
                    ctx->dropped_lo_tasks++;
                }
            }
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_mc_dispatch(sched_mc_t *ctx, uint32_t *out_task_id)
{
    if (ctx == NULL || out_task_id == NULL)
    {
        return SCHED_ERR_PARAM;
    }
    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    uint32_t best_id   = SCHED_MC_NO_TASK;
    uint32_t best_prio = 0xFFFFFFFF;  // Lowest number = highest priority in this framework?
    // Wait, FreeRTOS priority logic: HIGHER number = HIGHER priority.
    uint32_t highest_prio = 0;
    bool found            = false;

    for (int i = 0; i < SCHED_MC_CAPACITY; i++)
    {
        sched_mc_task_t *t = &ctx->registry[i];
        if (t->active && !t->dropped)
        {
            /* In HI mode, only HI criticality tasks are considered */
            if (ctx->mode == SCHED_MC_MODE_HI && t->criticality == SCHED_MC_CRIT_LO)
            {
                continue;
            }

            /* Fixed Priority: higher value = higher priority */
            if (!found || t->priority > highest_prio)
            {
                highest_prio = t->priority;
                best_id      = t->task_id;
                found        = true;
            }
        }
    }

    if (found)
    {
        *out_task_id = best_id;
        return SCHED_OK;
    }

    *out_task_id = SCHED_MC_NO_TASK;
    return SCHED_ERR_NOT_FOUND;
}

SchedStatus_t sched_mc_reset_budget(sched_mc_t *ctx, uint32_t task_id)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }
    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    for (int i = 0; i < SCHED_MC_CAPACITY; i++)
    {
        if (ctx->registry[i].active && ctx->registry[i].task_id == task_id)
        {
            sched_mc_task_t *t = &ctx->registry[i];
            /* At release, reset state */
            t->dropped = false;

            /* In HI mode, HI tasks get HI WCET. In LO mode, they get LO WCET */
            if (t->criticality == SCHED_MC_CRIT_HI && ctx->mode == SCHED_MC_MODE_HI)
            {
                t->current_budget = t->hi_wcet;
            }
            else
            {
                t->current_budget = t->lo_wcet;
            }
            return SCHED_OK;
        }
    }

    return SCHED_ERR_NOT_FOUND;
}
