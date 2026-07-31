# Real-Time Trace & Visualization Framework

This document describes the Phase 17 Trace Visualization framework.

## Architecture

The framework records core scheduling events dynamically:
- `TASK_CREATE` / `TASK_DELETE`
- `TASK_READY`
- `TASK_START` / `TASK_PREEMPT` / `TASK_RESUME` / `TASK_COMPLETE`
- `TASK_BLOCK` / `TASK_UNBLOCK`
- `CONTEXT_SWITCH`
- `IDLE_ENTER` / `IDLE_EXIT`
- `DEADLINE_MISS`
- `MC_MODE_SWITCH`
- `ADAPTIVE_POLICY_CHANGE`
- `FAULT_INJECTED` / `FAULT_TRIGGERED` / `FAULT_RECOVERED`

Traces are stored in a static lock-free ring buffer designed for O(1) overhead (`sched_trace.c`).
Events are emitted at runtime via `sched_trace_record()`.

## Exporting

The core benchmark outputs traces over UART in JSON array format using `sched_trace_export_json()`.
The Python analysis pipeline ingests this array and converts it to a standard DataFrame.

## Python Pipeline

1. **`parse_uart_log.py`**: Reads the trace array.
2. **`gantt_chart.py`**: Plots an execution timeline matching standard publication graphics.
3. **`trace_statistics.py`**: Aggregates fault rates, misses, switches, and context switch counts.
4. **`timeline_visualizer.py`**: Orchestrates parsing, statistical analysis, and Gantt chart generation into a single command.

```bash
python scripts/analysis/timeline_visualizer.py --input trace.json --output-dir results/
```
