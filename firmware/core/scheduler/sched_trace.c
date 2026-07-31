/**
 * @file    sched_trace.c
 * @brief   Real-Time Trace & Visualization Framework implementation.
 */

#include "sched_trace.h"

#include "schedtiny_status.h"

#include <stdio.h>
#include <string.h>

#if SCHED_CONFIG_ENABLE_TRACE

static sched_trace_context_t g_trace_ctx;

static const char *get_event_name(sched_trace_event_type_t type)
{
    switch (type)
    {
        case SCHED_TRACE_EVT_TASK_CREATE:
            return "TASK_CREATE";
        case SCHED_TRACE_EVT_TASK_DELETE:
            return "TASK_DELETE";
        case SCHED_TRACE_EVT_TASK_READY:
            return "TASK_READY";
        case SCHED_TRACE_EVT_TASK_START:
            return "TASK_START";
        case SCHED_TRACE_EVT_TASK_STOP:
            return "TASK_STOP";
        case SCHED_TRACE_EVT_TASK_COMPLETE:
            return "TASK_COMPLETE";
        case SCHED_TRACE_EVT_TASK_PREEMPT:
            return "TASK_PREEMPT";
        case SCHED_TRACE_EVT_TASK_RESUME:
            return "TASK_RESUME";
        case SCHED_TRACE_EVT_TASK_BLOCK:
            return "TASK_BLOCK";
        case SCHED_TRACE_EVT_TASK_UNBLOCK:
            return "TASK_UNBLOCK";
        case SCHED_TRACE_EVT_CONTEXT_SWITCH:
            return "CONTEXT_SWITCH";
        case SCHED_TRACE_EVT_DEADLINE_MISS:
            return "DEADLINE_MISS";
        case SCHED_TRACE_EVT_FAULT_INJECTED:
            return "FAULT_INJECTED";
        case SCHED_TRACE_EVT_FAULT_TRIGGERED:
            return "FAULT_TRIGGERED";
        case SCHED_TRACE_EVT_FAULT_RECOVERED:
            return "FAULT_RECOVERED";
        case SCHED_TRACE_EVT_MC_MODE_SWITCH:
            return "MC_MODE_SWITCH";
        case SCHED_TRACE_EVT_ADAPTIVE_POLICY_CHANGE:
            return "ADAPTIVE_POLICY_CHANGE";
        case SCHED_TRACE_EVT_IDLE_ENTER:
            return "IDLE_ENTER";
        case SCHED_TRACE_EVT_IDLE_EXIT:
            return "IDLE_EXIT";
        default:
            return "UNKNOWN";
    }
}

SchedStatus_t sched_trace_init(void)
{
    if (g_trace_ctx.initialized)
    {
        return SCHED_ERR_STATE;
    }

    memset(&g_trace_ctx, 0, sizeof(g_trace_ctx));
    g_trace_ctx.initialized = true;

    return SCHED_OK;
}

SchedStatus_t sched_trace_reset(void)
{
    if (!g_trace_ctx.initialized)
    {
        return SCHED_ERR_STATE;
    }

    g_trace_ctx.head           = 0;
    g_trace_ctx.count          = 0;
    g_trace_ctx.overflow_count = 0;

    return SCHED_OK;
}

SchedStatus_t sched_trace_record(uint32_t timestamp,
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
    if (!g_trace_ctx.initialized)
    {
        return SCHED_ERR_STATE;
    }

    uint32_t index           = g_trace_ctx.head;
    sched_trace_event_t *evt = &g_trace_ctx.buffer[index];

    evt->timestamp        = timestamp;
    evt->event_type       = type;
    evt->task_id          = task_id;
    evt->scheduler_policy = policy;
    evt->core_id          = core_id;
    evt->criticality      = criticality;
    evt->priority         = priority;
    evt->deadline         = deadline;
    evt->execution_time   = execution_time;
    evt->metadata         = metadata;

    g_trace_ctx.head = (g_trace_ctx.head + 1) % SCHED_TRACE_BUFFER_SIZE;
    if (g_trace_ctx.count < SCHED_TRACE_BUFFER_SIZE)
    {
        g_trace_ctx.count++;
    }
    else
    {
        g_trace_ctx.overflow_count++;
    }

    return SCHED_OK;
}

SchedStatus_t sched_trace_export_csv(char *buffer, size_t max_len)
{
    if (!g_trace_ctx.initialized || buffer == NULL || max_len == 0)
    {
        return SCHED_ERR_PARAM;
    }

    const char *header =
        "Timestamp,Event,TaskID,Policy,Core,Criticality,Priority,Deadline,ExecutionTime,Metadata\n";
    int offset = snprintf(buffer, max_len, "%s", header);
    if (offset < 0 || (size_t)offset >= max_len)
        return SCHED_ERR_OVERFLOW;

    uint32_t start_idx = (g_trace_ctx.count < SCHED_TRACE_BUFFER_SIZE) ? 0 : g_trace_ctx.head;

    for (uint32_t i = 0; i < g_trace_ctx.count; i++)
    {
        uint32_t idx                   = (start_idx + i) % SCHED_TRACE_BUFFER_SIZE;
        const sched_trace_event_t *evt = &g_trace_ctx.buffer[idx];

        int written =
            snprintf(buffer + offset, max_len - offset, "%lu,%s,%lu,%u,%u,%u,%u,%lu,%lu,%lu\n",
                     (unsigned long)evt->timestamp, get_event_name(evt->event_type),
                     (unsigned long)evt->task_id, evt->scheduler_policy, evt->core_id,
                     evt->criticality, evt->priority, (unsigned long)evt->deadline,
                     (unsigned long)evt->execution_time, (unsigned long)evt->metadata);

        if (written < 0 || (size_t)written >= max_len - offset)
            return SCHED_ERR_OVERFLOW;
        offset += written;
    }

    return SCHED_OK;
}

SchedStatus_t sched_trace_export_json(char *buffer, size_t max_len)
{
    if (!g_trace_ctx.initialized || buffer == NULL || max_len == 0)
    {
        return SCHED_ERR_PARAM;
    }

    int offset = snprintf(buffer, max_len, "[\n");
    if (offset < 0 || (size_t)offset >= max_len)
        return SCHED_ERR_OVERFLOW;

    uint32_t start_idx = (g_trace_ctx.count < SCHED_TRACE_BUFFER_SIZE) ? 0 : g_trace_ctx.head;
    bool first         = true;

    for (uint32_t i = 0; i < g_trace_ctx.count; i++)
    {
        uint32_t idx                   = (start_idx + i) % SCHED_TRACE_BUFFER_SIZE;
        const sched_trace_event_t *evt = &g_trace_ctx.buffer[idx];

        if (!first)
        {
            int written = snprintf(buffer + offset, max_len - offset, ",\n");
            if (written < 0 || (size_t)written >= max_len - offset)
                return SCHED_ERR_OVERFLOW;
            offset += written;
        }
        first = false;

        int written = snprintf(buffer + offset, max_len - offset,
                               "  {\n"
                               "    \"Timestamp\": %lu,\n"
                               "    \"Event\": \"%s\",\n"
                               "    \"TaskID\": %lu,\n"
                               "    \"Policy\": %u,\n"
                               "    \"Core\": %u,\n"
                               "    \"Criticality\": %u,\n"
                               "    \"Priority\": %u,\n"
                               "    \"Deadline\": %lu,\n"
                               "    \"ExecutionTime\": %lu,\n"
                               "    \"Metadata\": %lu\n"
                               "  }",
                               (unsigned long)evt->timestamp, get_event_name(evt->event_type),
                               (unsigned long)evt->task_id, evt->scheduler_policy, evt->core_id,
                               evt->criticality, evt->priority, (unsigned long)evt->deadline,
                               (unsigned long)evt->execution_time, (unsigned long)evt->metadata);

        if (written < 0 || (size_t)written >= max_len - offset)
            return SCHED_ERR_OVERFLOW;
        offset += written;
    }

    int written = snprintf(buffer + offset, max_len - offset, "\n]\n");
    if (written < 0 || (size_t)written >= max_len - offset)
        return SCHED_ERR_OVERFLOW;

    return SCHED_OK;
}

SchedStatus_t sched_trace_export_bin(uint8_t *buffer, size_t max_len, size_t *out_len)
{
    if (!g_trace_ctx.initialized || buffer == NULL || max_len == 0 || out_len == NULL)
    {
        return SCHED_ERR_PARAM;
    }

    size_t required_len = g_trace_ctx.count * sizeof(sched_trace_event_t);
    if (max_len < required_len)
    {
        return SCHED_ERR_OVERFLOW;
    }

    uint32_t start_idx = (g_trace_ctx.count < SCHED_TRACE_BUFFER_SIZE) ? 0 : g_trace_ctx.head;

    for (uint32_t i = 0; i < g_trace_ctx.count; i++)
    {
        uint32_t idx = (start_idx + i) % SCHED_TRACE_BUFFER_SIZE;
        memcpy(&buffer[i * sizeof(sched_trace_event_t)], &g_trace_ctx.buffer[idx],
               sizeof(sched_trace_event_t));
    }

    *out_len = required_len;

    return SCHED_OK;
}

sched_trace_context_t *sched_trace_get_context(void)
{
    return &g_trace_ctx;
}

#endif /* SCHED_CONFIG_ENABLE_TRACE */
