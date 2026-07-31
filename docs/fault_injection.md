# Fault Injection & Reliability Framework

## Overview
SchedTiny provides a comprehensive fault injection framework to evaluate the reliability and robustness of scheduling policies (HPF, EDF, RMS, Mixed Criticality). This framework is designed for research-grade evaluation and strictly uses static memory, pure C17, and deterministic LCG for reproducible experiments.

## Supported Fault Types

- **Execution Overrun (`SCHED_FAULT_EXECUTION_OVERRUN`):** Task WCET is exceeded dynamically.
- **Random Failure (`SCHED_FAULT_RANDOM_FAILURE`):** Task aborts or drops during execution.
- **Dispatcher Failure (`SCHED_FAULT_DISPATCHER_FAILURE`):** The dispatcher skips selecting a valid task and chooses Idle.
- **Priority Inversion (`SCHED_FAULT_PRIORITY_INVERSION`):** A high-priority task is blocked by a low-priority task (statistics simulation).

## Architecture

The framework relies on a global deterministic LCG (Linear Congruential Generator), accessible via `benchmark_lcg_rand()`.

It hooks into the main scheduling benchmark simulation loops via:
- `sched_fault_tick_hook`: Evaluates dynamic fault conditions every simulation tick.
- `sched_fault_dispatch_hook`: Evaluates dispatcher-level faults immediately before context switching.

## Metrics
The benchmark harnesses report detailed reliability metrics:
- **FaultsInjected:** Number of potential fault triggers injected into the simulation.
- **FaultsTriggered:** Actual faults that occurred (e.g., successful overruns).
- **RecoverySuccess:** Number of times the system recovered from a fault gracefully.
- **RecoveryTime:** Cumulative ticks spent recovering.
- **MissedDeadlinesAfterFault:** Deadlines missed directly resulting from a fault.
- **SystemAvailability:** Percentage of time the system remained in a nominal scheduling state.
- **FaultCoverage:** Ratio of handled faults vs total injected faults.

## APIs
```c
SchedStatus_t sched_fault_init(void);
SchedStatus_t sched_fault_reset(void);
SchedStatus_t sched_fault_enable(sched_fault_type_t type);
SchedStatus_t sched_fault_disable(sched_fault_type_t type);
SchedStatus_t sched_fault_inject(sched_fault_type_t type, uint32_t probability_bp);
SchedStatus_t sched_fault_clear(sched_fault_type_t type);
SchedStatus_t sched_fault_statistics(sched_fault_stats_t *stats);
```
