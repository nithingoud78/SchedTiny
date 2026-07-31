/**
 * @file    sched_trace.h
 * @brief   Real-Time Trace & Visualization Framework definitions and API.
 */

#ifndef SCHED_TRACE_H
#define SCHED_TRACE_H

#include "sched_types.h"

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SCHED_CONFIG_ENABLE_TRACE
#define SCHED_CONFIG_ENABLE_TRACE 1
#endif

#ifndef SCHED_TRACE_BUFFER_SIZE
#define SCHED_TRACE_BUFFER_SIZE 1024
#endif

    /**
     * @brief   Trace Event Types
     */
    typedef enum
    {
        SCHED_TRACE_EVT_TASK_CREATE = 0,
        SCHED_TRACE_EVT_TASK_DELETE,
        SCHED_TRACE_EVT_TASK_READY,
        SCHED_TRACE_EVT_TASK_START,
        SCHED_TRACE_EVT_TASK_STOP,
        SCHED_TRACE_EVT_TASK_COMPLETE,
        SCHED_TRACE_EVT_TASK_PREEMPT,
        SCHED_TRACE_EVT_TASK_RESUME,
        SCHED_TRACE_EVT_TASK_BLOCK,
        SCHED_TRACE_EVT_TASK_UNBLOCK,
        SCHED_TRACE_EVT_CONTEXT_SWITCH,
        SCHED_TRACE_EVT_DEADLINE_MISS,
        SCHED_TRACE_EVT_FAULT_INJECTED,
        SCHED_TRACE_EVT_FAULT_TRIGGERED,
        SCHED_TRACE_EVT_FAULT_RECOVERED,
        SCHED_TRACE_EVT_MC_MODE_SWITCH,
        SCHED_TRACE_EVT_ADAPTIVE_POLICY_CHANGE,
        SCHED_TRACE_EVT_IDLE_ENTER,
        SCHED_TRACE_EVT_IDLE_EXIT,
        SCHED_TRACE_EVT_MAX
    } sched_trace_event_type_t;

    /**
     * @brief   Trace Event Structure
     */
    typedef struct
    {
        uint32_t timestamp;
        sched_trace_event_type_t event_type;
        uint32_t task_id;
        uint8_t scheduler_policy; /* mapped from sched_benchmark_policy_t or local policy */
        uint8_t core_id;
        uint8_t criticality;
        uint8_t priority;
        uint32_t deadline;
        uint32_t execution_time;
        uint32_t metadata;
    } sched_trace_event_t;

    /**
     * @brief   Trace Buffer Context
     */
    typedef struct
    {
        sched_trace_event_t buffer[SCHED_TRACE_BUFFER_SIZE];
        uint32_t head;
        uint32_t count;
        uint32_t overflow_count;
        bool initialized;
    } sched_trace_context_t;

#if SCHED_CONFIG_ENABLE_TRACE

    /**
     * @brief   Initializes the trace framework.
     */
    SchedStatus_t sched_trace_init(void);

    /**
     * @brief   Resets the trace buffer.
     */
    SchedStatus_t sched_trace_reset(void);

    /**
     * @brief   Records a trace event.
     */
    SchedStatus_t sched_trace_record(uint32_t timestamp,
                                     sched_trace_event_type_t type,
                                     uint32_t task_id,
                                     uint8_t policy,
                                     uint8_t core_id,
                                     uint8_t criticality,
                                     uint8_t priority,
                                     uint32_t deadline,
                                     uint32_t execution_time,
                                     uint32_t metadata);

    /**
     * @brief   Exports the trace buffer to a CSV string.
     */
    SchedStatus_t sched_trace_export_csv(char *buffer, size_t max_len);

    /**
     * @brief   Exports the trace buffer to a JSON string.
     */
    SchedStatus_t sched_trace_export_json(char *buffer, size_t max_len);

    /**
     * @brief   Exports the trace buffer to a binary format.
     */
    SchedStatus_t sched_trace_export_bin(uint8_t *buffer, size_t max_len, size_t *out_len);

    /**
     * @brief   Gets the global trace context (for testing/accessing stats).
     */
    sched_trace_context_t *sched_trace_get_context(void);

#else  /* SCHED_CONFIG_ENABLE_TRACE == 0 */

/* Stub definitions to compile out tracing */
static inline SchedStatus_t sched_trace_init(void)
{
    return SCHED_OK;
}

static inline SchedStatus_t sched_trace_reset(void)
{
    return SCHED_OK;
}

static inline SchedStatus_t sched_trace_record(uint32_t timestamp,
                                               sched_trace_event_type_t type,
                                               uint32_t task_id,
                                               uint8_t policy,
                                               uint8_t core_id,
                                               uint8_t criticality,
                                               uint8_t priority,
                                               uint32_t deadline,
                                               uint32_t execution_time,
                                               uint32_t metadata)
{
    (void)timestamp;
    (void)type;
    (void)task_id;
    (void)policy;
    (void)core_id;
    (void)criticality;
    (void)priority;
    (void)deadline;
    (void)execution_time;
    (void)metadata;
    return SCHED_OK;
}

static inline SchedStatus_t sched_trace_export_csv(char *buffer, size_t max_len)
{
    (void)buffer;
    (void)max_len;
    return SCHED_ERR_NOT_SUPPORTED;
}

static inline SchedStatus_t sched_trace_export_json(char *buffer, size_t max_len)
{
    (void)buffer;
    (void)max_len;
    return SCHED_ERR_NOT_SUPPORTED;
}

static inline SchedStatus_t sched_trace_export_bin(uint8_t *buffer, size_t max_len, size_t *out_len)
{
    (void)buffer;
    (void)max_len;
    (void)out_len;
    return SCHED_ERR_NOT_SUPPORTED;
}

static inline sched_trace_context_t *sched_trace_get_context(void)
{
    return NULL;
}

#endif /* SCHED_CONFIG_ENABLE_TRACE */

#ifdef __cplusplus
}
#endif

#endif /* SCHED_TRACE_H */
