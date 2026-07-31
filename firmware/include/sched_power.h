/**
 * @file    sched_power.h
 * @brief   SchedTiny Energy & Power Profiling Framework.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_POWER_H
#define SCHEDTINY_SCHED_POWER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "sched_stats.h"
#include "schedtiny_status.h"

#include <stdint.h>

    /**
     * @brief Supported simulated power models.
     */
    typedef enum
    {
        SCHED_POWER_MODEL_CONSTANT = 0,
        SCHED_POWER_MODEL_IDLE_ACTIVE,
        SCHED_POWER_MODEL_FREQ_SCALED
    } sched_power_model_t;

    /**
     * @brief Energy and power metrics derived from runtime statistics.
     */
    typedef struct
    {
        uint32_t estimated_energy_uj; /**< Total energy consumed in microjoules */
        uint32_t estimated_power_uw;  /**< Average power in microwatts */
        uint32_t energy_per_task_uj;  /**< Average energy per task completion */
        uint32_t energy_per_cs_uj;    /**< Average energy overhead per context switch */
    } sched_power_metrics_t;

    /**
     * @brief   Compute estimated power and energy metrics based on runtime statistics.
     *
     * @param   model       The power model to simulate.
     * @param   stats       Pointer to the finalized scheduling statistics.
     * @param   out_metrics Pointer to store the computed energy metrics.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_power_compute(sched_power_model_t model,
                                      const sched_stats_data_t *stats,
                                      uint32_t task_completion_count,
                                      sched_power_metrics_t *out_metrics);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_POWER_H */
