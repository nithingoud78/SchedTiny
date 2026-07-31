/**
 * @file    sched_adaptive.h
 * @brief   SchedTiny AI-Assisted Adaptive Scheduling Framework.
 *
 * Provides runtime policy selection using lightweight decision engines
 * (rule-based heuristics, decision tree inference, lookup tables).
 * All inference is deterministic, integer-only, and uses static memory.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_SCHED_ADAPTIVE_H
#define SCHEDTINY_SCHED_ADAPTIVE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "sched_benchmark.h"
#include "schedtiny_status.h"

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief   Number of workload features extracted for decision making.
 */
#define SCHED_ADAPTIVE_NUM_FEATURES 16

/**
 * @brief   Maximum entries in the feature history window.
 */
#define SCHED_ADAPTIVE_HISTORY_SIZE 8

/**
 * @brief   Default evaluation interval in simulation ticks.
 */
#define SCHED_ADAPTIVE_DEFAULT_INTERVAL 100

/**
 * @brief   Default hysteresis threshold in basis points.
 *          A new policy must score at least this many bp higher to trigger a switch.
 */
#define SCHED_ADAPTIVE_DEFAULT_HYSTERESIS 500

/**
 * @brief   Number of supported scheduling policies the adaptive engine selects from.
 */
#define SCHED_ADAPTIVE_NUM_POLICIES 4

    /**
     * @brief   Decision engine types.
     */
    typedef enum
    {
        SCHED_ADAPTIVE_ENGINE_RULE_BASED = 0,
        SCHED_ADAPTIVE_ENGINE_DECISION_TREE,
        SCHED_ADAPTIVE_ENGINE_LOOKUP_TABLE
    } sched_adaptive_engine_t;

    /**
     * @brief   Workload feature vector (all values in basis points 0–10000).
     *
     * Features are normalised to basis points so that all inference
     * can be performed in integer arithmetic without floating point.
     */
    typedef struct
    {
        uint32_t cpu_utilization_bp;       /**< CPU utilization (0–10000) */
        uint32_t task_count;               /**< Number of active tasks */
        uint32_t ready_queue_length;       /**< Ready queue occupancy */
        uint32_t avg_response_time;        /**< Average response time (ticks) */
        uint32_t avg_waiting_time;         /**< Average waiting time (ticks) */
        uint32_t deadline_miss_rate_bp;    /**< Deadline miss rate (0–10000) */
        uint32_t context_switch_rate_bp;   /**< Context switch rate (0–10000) */
        uint32_t idle_time;                /**< Accumulated idle ticks */
        uint32_t busy_time;                /**< Accumulated busy ticks */
        uint32_t fault_injection_rate_bp;  /**< Fault rate (0–10000) */
        uint32_t recovery_success_rate_bp; /**< Recovery success (0–10000) */
        uint32_t energy_consumption;       /**< Estimated energy (uJ) */
        uint32_t avg_power;                /**< Estimated power (uW) */
        uint32_t criticality_hi_count;     /**< Number of HI-criticality tasks */
        uint32_t hi_criticality_ratio_bp;  /**< HI-crit ratio (0–10000) */
        uint32_t mode_switch_frequency;    /**< MC mode switches */
    } sched_adaptive_features_t;

    /**
     * @brief   Per-policy score from a decision engine evaluation.
     */
    typedef struct
    {
        uint32_t scores[SCHED_ADAPTIVE_NUM_POLICIES]; /**< Score per policy (0–10000) */
        sched_benchmark_policy_t recommended;         /**< Highest-scoring policy */
        uint32_t confidence_bp;                       /**< Confidence of recommendation */
    } sched_adaptive_decision_t;

    /**
     * @brief   Adaptive scheduler runtime statistics.
     */
    typedef struct
    {
        uint32_t total_decisions;  /**< Total evaluate() calls */
        uint32_t total_switches;   /**< Policy switches performed */
        uint32_t decision_latency; /**< Cumulative decision ticks */
        uint32_t overhead_ticks;   /**< Total adaptive overhead */
        uint32_t accuracy_bp;      /**< Accuracy vs oracle (0–10000) */
        uint32_t selection_counts[SCHED_ADAPTIVE_NUM_POLICIES]; /**< Per-policy counts */
    } sched_adaptive_stats_t;

    /**
     * @brief   Adaptive scheduler configuration.
     */
    typedef struct
    {
        sched_adaptive_engine_t engine; /**< Which decision engine to use */
        uint32_t eval_interval;         /**< Ticks between evaluations */
        uint32_t hysteresis_bp;         /**< Min score delta to switch (bp) */
        uint32_t max_switches_per_1000; /**< Max switches per 1000 ticks */
    } sched_adaptive_config_t;

    /**
     * @brief   Adaptive scheduler context.
     */
    typedef struct
    {
        sched_adaptive_config_t config;
        sched_benchmark_policy_t active_policy;
        sched_adaptive_features_t history[SCHED_ADAPTIVE_HISTORY_SIZE];
        uint32_t history_count;
        uint32_t history_index;
        uint32_t ticks_since_last_eval;
        uint32_t ticks_since_last_switch;
        sched_adaptive_stats_t stats;
        bool initialized;
    } sched_adaptive_t;

    /**
     * @brief   Initialize the adaptive scheduler context.
     * @param   ctx Pointer to adaptive context.
     * @return  SCHED_OK on success, SCHED_ERR_PARAM if NULL.
     */
    SchedStatus_t sched_adaptive_init(sched_adaptive_t *ctx);

    /**
     * @brief   Reset the adaptive scheduler to initial state.
     * @param   ctx Pointer to adaptive context.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_adaptive_reset(sched_adaptive_t *ctx);

    /**
     * @brief   Configure the adaptive scheduler.
     * @param   ctx    Pointer to adaptive context.
     * @param   config Pointer to configuration.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_adaptive_set_config(sched_adaptive_t *ctx,
                                            const sched_adaptive_config_t *config);

    /**
     * @brief   Extract workload features from benchmark state.
     * @param   bench_ctx Pointer to benchmark context.
     * @param   policy    Current policy being benchmarked.
     * @param   out       Pointer to store extracted features.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_adaptive_extract_features(const sched_benchmark_t *bench_ctx,
                                                  sched_benchmark_policy_t policy,
                                                  sched_adaptive_features_t *out);

    /**
     * @brief   Run the decision engine and return per-policy scores.
     * @param   ctx      Pointer to adaptive context.
     * @param   features Pointer to current feature vector.
     * @param   out      Pointer to store the decision result.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_adaptive_evaluate(sched_adaptive_t *ctx,
                                          const sched_adaptive_features_t *features,
                                          sched_adaptive_decision_t *out);

    /**
     * @brief   Full pipeline: extract features, evaluate, apply hysteresis, switch if needed.
     * @param   ctx       Pointer to adaptive context.
     * @param   bench_ctx Pointer to current benchmark context.
     * @param   policy    Current active policy index (for feature extraction).
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_adaptive_select_policy(sched_adaptive_t *ctx,
                                               const sched_benchmark_t *bench_ctx,
                                               sched_benchmark_policy_t policy);

    /**
     * @brief   Get the currently active policy selected by the adaptive scheduler.
     * @param   ctx Pointer to adaptive context.
     * @return  The active policy, or SCHED_BENCHMARK_POLICY_HPF if uninitialised.
     */
    sched_benchmark_policy_t sched_adaptive_get_active_policy(const sched_adaptive_t *ctx);

    /**
     * @brief   Retrieve adaptive scheduler runtime statistics.
     * @param   ctx  Pointer to adaptive context.
     * @param   out  Pointer to store statistics.
     * @return  SCHED_OK on success.
     */
    SchedStatus_t sched_adaptive_get_stats(const sched_adaptive_t *ctx,
                                           sched_adaptive_stats_t *out);

    /**
     * @brief   Notify the adaptive scheduler that one simulation tick has elapsed.
     *          Used for interval tracking and rate limiting.
     * @param   ctx Pointer to adaptive context.
     * @return  true if an evaluation should be performed this tick.
     */
    bool sched_adaptive_tick(sched_adaptive_t *ctx);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_SCHED_ADAPTIVE_H */
