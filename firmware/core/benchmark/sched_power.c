#include "sched_power.h"

#include <stddef.h>

/* Default Simulated Hardware Power Constants for Cortex-M7 (microwatts) */
#define POWER_UW_ACTIVE_DEFAULT 100000 /* 100 mW */
#define POWER_UW_IDLE_DEFAULT   20000  /* 20 mW */

/* Frequency Scaled Multipliers */
#define POWER_FREQ_SCALE_FACTOR 1.5f /* Mock scaling for higher performance states */

SchedStatus_t sched_power_compute(sched_power_model_t model,
                                  const sched_stats_data_t *stats,
                                  uint32_t task_completion_count,
                                  sched_power_metrics_t *out_metrics)
{
    if (stats == NULL || out_metrics == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    /* Convert ticks to seconds (assuming 1 tick = 1 ms for standard simulator) */
    uint32_t total_time_ms = stats->busy_time + stats->idle_time;
    if (total_time_ms == 0)
    {
        /* Avoid division by zero */
        out_metrics->estimated_energy_uj = 0;
        out_metrics->estimated_power_uw  = 0;
        out_metrics->energy_per_task_uj  = 0;
        out_metrics->energy_per_cs_uj    = 0;
        return SCHED_OK;
    }

    uint32_t energy_uj = 0;

    switch (model)
    {
        case SCHED_POWER_MODEL_CONSTANT:
            /* Uniform power draw regardless of idle/active state */
            energy_uj = (POWER_UW_ACTIVE_DEFAULT / 1000) * total_time_ms;
            break;

        case SCHED_POWER_MODEL_IDLE_ACTIVE:
            /* Differentiates busy vs idle time */
            energy_uj = ((POWER_UW_ACTIVE_DEFAULT / 1000) * stats->busy_time) +
                        ((POWER_UW_IDLE_DEFAULT / 1000) * stats->idle_time);
            break;

        case SCHED_POWER_MODEL_FREQ_SCALED:
            /* Approximates a higher frequency state during active computation */
            energy_uj = ((uint32_t)(POWER_UW_ACTIVE_DEFAULT * POWER_FREQ_SCALE_FACTOR) / 1000 *
                         stats->busy_time) +
                        ((POWER_UW_IDLE_DEFAULT / 1000) * stats->idle_time);
            break;

        default:
            return SCHED_ERR_PARAM;
    }

    /* Context Switch Overhead Approximation: Assume each context switch takes an extra active chunk
       We estimate 1 context switch = 2 uJ overhead (simulated physical cache flush + register push)
     */
    uint32_t context_switch_overhead_uj = stats->context_switch_count * 2;
    energy_uj += context_switch_overhead_uj;

    out_metrics->estimated_energy_uj = energy_uj;
    out_metrics->estimated_power_uw  = (energy_uj * 1000) / total_time_ms;

    if (task_completion_count > 0)
    {
        out_metrics->energy_per_task_uj = energy_uj / task_completion_count;
    }
    else
    {
        out_metrics->energy_per_task_uj = 0;
    }

    if (stats->context_switch_count > 0)
    {
        out_metrics->energy_per_cs_uj = context_switch_overhead_uj / stats->context_switch_count;
    }
    else
    {
        out_metrics->energy_per_cs_uj = 0;
    }

    return SCHED_OK;
}
