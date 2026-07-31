/**
 * @file    sched_adaptive.c
 * @brief   SchedTiny AI-Assisted Adaptive Scheduling Framework.
 *
 * Implements runtime policy selection using three decision engines:
 *   1. Rule-based heuristics
 *   2. Decision tree inference (generated or hand-crafted)
 *   3. Static lookup table
 *
 * All inference is deterministic, integer-only, and uses static memory.
 *
 * @author  @nithingoud78
 * @date    2026-07-31
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#include "sched_adaptive.h"

#include "sched_adaptive_model.h"
#include "sched_benchmark.h"
#include "sched_trace.h"

#include <string.h>

/* =========================================================================
 * Initialisation & Reset
 * ========================================================================= */

SchedStatus_t sched_adaptive_init(sched_adaptive_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    memset(ctx, 0, sizeof(sched_adaptive_t));

    ctx->config.engine                = SCHED_ADAPTIVE_ENGINE_RULE_BASED;
    ctx->config.eval_interval         = SCHED_ADAPTIVE_DEFAULT_INTERVAL;
    ctx->config.hysteresis_bp         = SCHED_ADAPTIVE_DEFAULT_HYSTERESIS;
    ctx->config.max_switches_per_1000 = 10;
    ctx->active_policy                = SCHED_BENCHMARK_POLICY_HPF;
    ctx->initialized                  = true;

    return SCHED_OK;
}

SchedStatus_t sched_adaptive_reset(sched_adaptive_t *ctx)
{
    if (ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    sched_adaptive_config_t saved_config = ctx->config;
    memset(ctx, 0, sizeof(sched_adaptive_t));
    ctx->config        = saved_config;
    ctx->active_policy = SCHED_BENCHMARK_POLICY_HPF;
    ctx->initialized   = true;

    return SCHED_OK;
}

SchedStatus_t sched_adaptive_set_config(sched_adaptive_t *ctx,
                                        const sched_adaptive_config_t *config)
{
    if (ctx == NULL || config == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    ctx->config = *config;

    /* Enforce minimum eval interval */
    if (ctx->config.eval_interval == 0)
    {
        ctx->config.eval_interval = 1;
    }

    return SCHED_OK;
}

/* =========================================================================
 * Feature Extraction
 * ========================================================================= */

SchedStatus_t sched_adaptive_extract_features(const sched_benchmark_t *bench_ctx,
                                              sched_benchmark_policy_t policy,
                                              sched_adaptive_features_t *out)
{
    if (bench_ctx == NULL || out == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!bench_ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    memset(out, 0, sizeof(sched_adaptive_features_t));

    /* If results exist for this policy, use them */
    if (policy <= SCHED_BENCHMARK_POLICY_MC && bench_ctx->has_results[policy])
    {
        const sched_benchmark_results_t *r = &bench_ctx->results[policy];

        out->cpu_utilization_bp    = r->cpu_utilization_bp;
        out->task_count            = r->total_tasks;
        out->avg_response_time     = r->avg_response_time;
        out->avg_waiting_time      = r->avg_waiting_time;
        out->idle_time             = r->idle_time;
        out->busy_time             = r->busy_time;
        out->energy_consumption    = r->estimated_energy_uj;
        out->avg_power             = r->estimated_power_uw;
        out->mode_switch_frequency = r->mode_switches;

        /* Compute deadline miss rate (per 10000 ticks, effectively bp) */
        if (r->task_completion_count + r->deadline_misses > 0)
        {
            out->deadline_miss_rate_bp =
                (r->deadline_misses * 10000) / (r->task_completion_count + r->deadline_misses);
        }

        /* Compute context switch rate (switches per 10000 busy ticks) */
        if (r->busy_time > 0)
        {
            out->context_switch_rate_bp = (r->context_switches * 10000) / r->busy_time;
        }

        /* Fault metrics */
        if (r->faults_injected > 0)
        {
            out->fault_injection_rate_bp = (r->faults_triggered * 10000) / r->faults_injected;
        }

        if (r->faults_triggered > 0)
        {
            out->recovery_success_rate_bp = (r->recovery_success * 10000) / r->faults_triggered;
        }
    }

    /* Compute criticality distribution from task definitions */
    uint32_t hi_count = 0;
    for (uint32_t i = 0; i < bench_ctx->task_count; i++)
    {
        if (bench_ctx->tasks[i].criticality == 1)
        {
            hi_count++;
        }
    }
    out->criticality_hi_count = hi_count;

    if (bench_ctx->task_count > 0)
    {
        out->hi_criticality_ratio_bp = (hi_count * 10000) / bench_ctx->task_count;
    }

    out->ready_queue_length = bench_ctx->task_count;

    return SCHED_OK;
}

/* =========================================================================
 * Rule-Based Decision Engine
 * ========================================================================= */

/**
 * @brief   Evaluate policy suitability using deterministic rules.
 *
 * Returns per-policy scores (0–10000) based on workload characteristics.
 */
static void rule_based_evaluate(const sched_adaptive_features_t *f, sched_adaptive_decision_t *out)
{
    memset(out->scores, 0, sizeof(out->scores));

    /* --- EDF scoring --- */
    /* EDF excels under high utilization with deadline pressure */
    if (f->cpu_utilization_bp > 8500)
    {
        out->scores[SCHED_BENCHMARK_POLICY_EDF] += 3000;
    }
    else if (f->cpu_utilization_bp > 6000)
    {
        out->scores[SCHED_BENCHMARK_POLICY_EDF] += 2000;
    }

    if (f->deadline_miss_rate_bp > 500)
    {
        out->scores[SCHED_BENCHMARK_POLICY_EDF] += 3000;
    }
    else if (f->deadline_miss_rate_bp > 100)
    {
        out->scores[SCHED_BENCHMARK_POLICY_EDF] += 1500;
    }

    /* --- MC scoring --- */
    /* MC excels when many HI-criticality tasks are present */
    if (f->hi_criticality_ratio_bp > 4000)
    {
        out->scores[SCHED_BENCHMARK_POLICY_MC] += 4000;
    }
    else if (f->hi_criticality_ratio_bp > 2000)
    {
        out->scores[SCHED_BENCHMARK_POLICY_MC] += 2000;
    }

    if (f->mode_switch_frequency > 0)
    {
        out->scores[SCHED_BENCHMARK_POLICY_MC] += 1000;
    }

    /* --- RMS scoring --- */
    /* RMS excels under low utilization with periodic workloads */
    if (f->cpu_utilization_bp < 5000)
    {
        out->scores[SCHED_BENCHMARK_POLICY_RMS] += 3000;
    }
    else if (f->cpu_utilization_bp < 7000)
    {
        out->scores[SCHED_BENCHMARK_POLICY_RMS] += 1500;
    }

    if (f->deadline_miss_rate_bp < 100)
    {
        out->scores[SCHED_BENCHMARK_POLICY_RMS] += 2000;
    }

    /* --- HPF scoring --- */
    /* HPF is the safe default; works well as a general-purpose policy */
    out->scores[SCHED_BENCHMARK_POLICY_HPF] += 2000; /* Base score */

    if (f->cpu_utilization_bp > 5000 && f->cpu_utilization_bp < 8500)
    {
        out->scores[SCHED_BENCHMARK_POLICY_HPF] += 1500;
    }

    if (f->hi_criticality_ratio_bp < 2000)
    {
        out->scores[SCHED_BENCHMARK_POLICY_HPF] += 1000;
    }

    /* --- Penalty adjustments --- */
    /* Penalise MC if very few HI-criticality tasks */
    if (f->hi_criticality_ratio_bp < 1000)
    {
        if (out->scores[SCHED_BENCHMARK_POLICY_MC] > 2000)
        {
            out->scores[SCHED_BENCHMARK_POLICY_MC] -= 2000;
        }
        else
        {
            out->scores[SCHED_BENCHMARK_POLICY_MC] = 0;
        }
    }

    /* Penalise RMS under very high utilization */
    if (f->cpu_utilization_bp > 8000)
    {
        if (out->scores[SCHED_BENCHMARK_POLICY_RMS] > 1500)
        {
            out->scores[SCHED_BENCHMARK_POLICY_RMS] -= 1500;
        }
        else
        {
            out->scores[SCHED_BENCHMARK_POLICY_RMS] = 0;
        }
    }

    /* Find the best policy */
    uint32_t best_score = 0;
    out->recommended    = SCHED_BENCHMARK_POLICY_HPF;

    for (int i = 0; i < SCHED_ADAPTIVE_NUM_POLICIES; i++)
    {
        if (out->scores[i] > best_score)
        {
            best_score       = out->scores[i];
            out->recommended = (sched_benchmark_policy_t)i;
        }
    }

    /* Confidence = best score as a fraction of maximum possible */
    out->confidence_bp = best_score;
    if (out->confidence_bp > 10000)
    {
        out->confidence_bp = 10000;
    }
}

/* =========================================================================
 * Decision Tree Engine
 * ========================================================================= */

static void decision_tree_evaluate(const sched_adaptive_features_t *f,
                                   sched_adaptive_decision_t *out)
{
    memset(out->scores, 0, sizeof(out->scores));

    sched_benchmark_policy_t prediction = sched_adaptive_tree_predict(f);

    /* Assign full confidence to the prediction */
    out->scores[prediction] = 8000;
    out->recommended        = prediction;
    out->confidence_bp      = 8000;

    /* Give partial scores to other policies for hysteresis comparison */
    for (int i = 0; i < SCHED_ADAPTIVE_NUM_POLICIES; i++)
    {
        if ((sched_benchmark_policy_t)i != prediction)
        {
            out->scores[i] = 2000;
        }
    }
}

/* =========================================================================
 * Lookup Table Engine
 * ========================================================================= */

static void lookup_table_evaluate(const sched_adaptive_features_t *f,
                                  sched_adaptive_decision_t *out)
{
    memset(out->scores, 0, sizeof(out->scores));

    /* Bucket utilization: 0=<25%, 1=25-50%, 2=50-75%, 3=>75% */
    uint32_t util_bucket = f->cpu_utilization_bp / 2500;
    if (util_bucket > 3)
    {
        util_bucket = 3;
    }

    /* Bucket criticality: 0=<25%, 1=25-50%, 2=50-75%, 3=>75% */
    uint32_t crit_bucket = f->hi_criticality_ratio_bp / 2500;
    if (crit_bucket > 3)
    {
        crit_bucket = 3;
    }

    sched_benchmark_policy_t recommendation = sched_adaptive_lookup_table[util_bucket][crit_bucket];

    out->scores[recommendation] = 7500;
    out->recommended            = recommendation;
    out->confidence_bp          = 7500;

    for (int i = 0; i < SCHED_ADAPTIVE_NUM_POLICIES; i++)
    {
        if ((sched_benchmark_policy_t)i != recommendation)
        {
            out->scores[i] = 2500;
        }
    }
}

/* =========================================================================
 * Public API: Evaluate
 * ========================================================================= */

SchedStatus_t sched_adaptive_evaluate(sched_adaptive_t *ctx,
                                      const sched_adaptive_features_t *features,
                                      sched_adaptive_decision_t *out)
{
    if (ctx == NULL || features == NULL || out == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    switch (ctx->config.engine)
    {
        case SCHED_ADAPTIVE_ENGINE_RULE_BASED:
            rule_based_evaluate(features, out);
            break;
        case SCHED_ADAPTIVE_ENGINE_DECISION_TREE:
            decision_tree_evaluate(features, out);
            break;
        case SCHED_ADAPTIVE_ENGINE_LOOKUP_TABLE:
            lookup_table_evaluate(features, out);
            break;
        default:
            rule_based_evaluate(features, out);
            break;
    }

    /* Record the decision in history */
    ctx->history[ctx->history_index] = *features;
    ctx->history_index               = (ctx->history_index + 1) % SCHED_ADAPTIVE_HISTORY_SIZE;
    if (ctx->history_count < SCHED_ADAPTIVE_HISTORY_SIZE)
    {
        ctx->history_count++;
    }

    ctx->stats.total_decisions++;
    ctx->stats.selection_counts[out->recommended]++;

    return SCHED_OK;
}

/* =========================================================================
 * Public API: Select Policy (Full Pipeline)
 * ========================================================================= */

SchedStatus_t sched_adaptive_select_policy(sched_adaptive_t *ctx,
                                           const sched_benchmark_t *bench_ctx,
                                           sched_benchmark_policy_t policy)
{
    if (ctx == NULL || bench_ctx == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    /* Extract features */
    sched_adaptive_features_t features;
    SchedStatus_t st = sched_adaptive_extract_features(bench_ctx, policy, &features);
    if (st != SCHED_OK)
    {
        return st;
    }

    /* Evaluate */
    sched_adaptive_decision_t decision;
    st = sched_adaptive_evaluate(ctx, &features, &decision);
    if (st != SCHED_OK)
    {
        return st;
    }

    /* Apply hysteresis */
    uint32_t current_score = decision.scores[ctx->active_policy];
    uint32_t new_score     = decision.scores[decision.recommended];

    if (decision.recommended != ctx->active_policy)
    {
        /* Only switch if the new policy scores sufficiently higher */
        if (new_score > current_score + ctx->config.hysteresis_bp)
        {
            ctx->active_policy = decision.recommended;
            ctx->stats.total_switches++;
            ctx->ticks_since_last_switch = 0;
#if SCHED_CONFIG_ENABLE_TRACE
            sched_trace_record(0, SCHED_TRACE_EVT_ADAPTIVE_POLICY_CHANGE, 0, ctx->active_policy, 0,
                               0, 0, 0, 0, 0);
#endif
        }
    }

    /* Track overhead (1 tick per decision as approximation) */
    ctx->stats.overhead_ticks++;
    ctx->stats.decision_latency++;

    return SCHED_OK;
}

/* =========================================================================
 * Public API: Tick & Accessors
 * ========================================================================= */

bool sched_adaptive_tick(sched_adaptive_t *ctx)
{
    if (ctx == NULL || !ctx->initialized)
    {
        return false;
    }

    ctx->ticks_since_last_eval++;
    ctx->ticks_since_last_switch++;

    if (ctx->ticks_since_last_eval >= ctx->config.eval_interval)
    {
        ctx->ticks_since_last_eval = 0;
        return true;
    }

    return false;
}

sched_benchmark_policy_t sched_adaptive_get_active_policy(const sched_adaptive_t *ctx)
{
    if (ctx == NULL || !ctx->initialized)
    {
        return SCHED_BENCHMARK_POLICY_HPF;
    }

    return ctx->active_policy;
}

SchedStatus_t sched_adaptive_get_stats(const sched_adaptive_t *ctx, sched_adaptive_stats_t *out)
{
    if (ctx == NULL || out == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    if (!ctx->initialized)
    {
        return SCHED_ERR_STATE;
    }

    *out = ctx->stats;
    return SCHED_OK;
}
