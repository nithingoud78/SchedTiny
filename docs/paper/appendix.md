# SchedTiny Research Paper: Appendix

This appendix provides supplementary mathematical derivations, hardware register definitions, complete feature vector formulations, and workload configuration tables supporting the main research paper.

---

## Appendix A: Mathematical Foundations and Schedulability Bounds

### A.1 Rate Monotonic Schedulability Bound
Under the Rate Monotonic Scheduling (RMS) policy, a periodic task set $\tau = \{\tau_1, \tau_2, \dots, \tau_n\}$ with execution times $C_i$ and periods $T_i$ is guaranteed to be schedulable if total CPU utilization satisfies the Liu & Layland utilization bound:

$$U = \sum_{i=1}^n \frac{C_i}{T_i} \le n \left(2^{1/n} - 1\right)$$

As $n \to \infty$, $U \approx \ln(2) \approx 0.693$. In SchedTiny, tasks with utilization $U > 0.693$ that fail the RMS condition automatically trigger the Adaptive Decision Engine to pivot to Earliest Deadline First (EDF) or High Priority First (HPF).

### A.2 Earliest Deadline First (EDF) Exact Bound
Under dynamic priority assignment via EDF, a task set with implicit deadlines ($D_i = T_i$) is schedulable if and only if:

$$\sum_{i=1}^n \frac{C_i}{T_i} \le 1.0$$

When $D_i < T_i$ (constrained deadlines), SchedTiny evaluates the Processor Demand Criterion $h(t)$:

$$h(t) = \sum_{D_i \le t} \left(1 + \left\lfloor \frac{t - D_i}{T_i} \right\rfloor\right) C_i \le t, \quad \forall t \ge 0$$

### A.3 Mixed-Criticality Vestal Model
In SchedTiny's Mixed-Criticality (MC) scheduler, tasks have two levels of criticality $\zeta_i \in \{\text{LO}, \text{HI}\}$. The low-criticality worst-case execution time is denoted $C_i(\text{LO})$ and high-criticality $C_i(\text{HI})$, where $C_i(\text{HI}) \ge C_i(\text{LO})$.

- **Normal Mode (LO):** Both LO- and HI-criticality tasks execute within their $C_i(\text{LO})$ budgets.
- **Degraded/Overrun Mode (HI):** If any HI-criticality task exceeds $C_i(\text{LO})$, SchedTiny triggers an immediate mode switch, shedding/throttling LO-criticality tasks while guaranteeing HI-criticality deadlines.

---

## Appendix B: STM32 Hardware Measurement Subsystem

### B.1 Core Debug & DWT Registers
SchedTiny utilizes the ARM Cortex-M Data Watchpoint and Trace (DWT) cycle counter register (`DWT_CYCCNT`) for sub-microsecond non-intrusive timing:

| Register | Address (Cortex-M4/M7) | Description | Configuration in SchedTiny |
|---|---|---|---|
| `CoreDebug->DEMCR` | `0xE000EDFC` | Debug Exception & Monitor Control | Set Bit 24 (`TRCENA`) to enable trace |
| `DWT->CTRL` | `0xE0001000` | DWT Control Register | Set Bit 0 (`CYCCNTENA`) to start counter |
| `DWT->CYCCNT` | `0xE0001004` | 32-bit Cycle Counter Register | Direct memory-mapped read via `drv_timer_dwt_now()` |

### B.2 Rollover Compensation
At 480 MHz (STM32H7), `DWT_CYCCNT` wraps every $2^{32} / 480\text{ MHz} \approx 8.947\text{ seconds}$. SchedTiny wraps counter reads in a critical section, detecting monotonicity resets to maintain a contiguous 64-bit cycle counter (`bench_dwt_cycles64()`).

---

## Appendix C: Complete 16-Dimensional Feature Vector Specification

All features are scaled to Basis Points (bp, where $10000 = 100.00\%$) to ensure zero-floating-point integer inference on Cortex-M0/M3/M4/M7 cores:

| Index | Feature Symbol | Description | Valid Range | Resolution |
|---|---|---|---|---|
| 0 | `cpu_utilization_bp` | Total active CPU utilization | $0 - 10000$ | $0.01\%$ |
| 1 | `task_count` | Number of currently registered tasks | $1 - 128$ | $1\text{ task}$ |
| 2 | `ready_queue_len` | Number of tasks in READY state | $0 - 128$ | $1\text{ task}$ |
| 3 | `avg_response_time` | Moving average task response time | $0 - 10^6\text{ ticks}$ | $1\text{ tick}$ |
| 4 | `avg_waiting_time` | Moving average queue wait time | $0 - 10^6\text{ ticks}$ | $1\text{ tick}$ |
| 5 | `deadline_miss_rate_bp` | Percentage of tasks missing deadlines | $0 - 10000$ | $0.01\%$ |
| 6 | `context_switch_rate_bp` | Context switches normalized per 10k ticks | $0 - 10000$ | $1\text{ CS/window}$ |
| 7 | `idle_time` | Total ticks spent in low-power idle task | $0 - 10^9\text{ ticks}$ | $1\text{ tick}$ |
| 8 | `busy_time` | Total ticks executing active workloads | $0 - 10^9\text{ ticks}$ | $1\text{ tick}$ |
| 9 | `fault_injection_rate_bp`| Rate of induced overruns/exceptions | $0 - 10000$ | $0.01\%$ |
| 10 | `recovery_success_rate_bp`| Proportion of successfully recovered faults | $0 - 10000$ | $0.01\%$ |
| 11 | `energy_consumption_uj` | Estimated energy footprint in microjoules | $0 - 10^9\,\mu\text{J}$ | $1\,\mu\text{J}$ |
| 12 | `avg_power_uw` | Average estimated power in microwatts | $0 - 10^6\,\mu\text{W}$ | $1\,\mu\text{W}$ |
| 13 | `hi_criticality_count` | Number of active HI-criticality tasks | $0 - 128$ | $1\text{ task}$ |
| 14 | `hi_criticality_ratio_bp`| Fraction of workload deemed HI-criticality | $0 - 10000$ | $0.01\%$ |
| 15 | `mode_switch_frequency` | Frequency of LO $\leftrightarrow$ HI transitions | $0 - 10000$ | $1\text{ switch/window}$ |

---

## Appendix D: TinyML Decision Tree Embedded C Realization

Below is an excerpt of the auto-generated integer-arithmetic decision tree compiled directly into the C firmware image (`sched_adaptive_model.h`):

```c
/* Model Metadata: SHA-256 Dataset Hash = e7b10a4f... */
sched_benchmark_policy_t sched_adaptive_tree_predict(const sched_adaptive_features_t *f)
{
    if (f->hi_criticality_ratio_bp > 2500) {
        if (f->fault_injection_rate_bp > 1000) {
            return SCHED_BENCHMARK_POLICY_MC;
        } else {
            if (f->cpu_utilization_bp > 7500) {
                return SCHED_BENCHMARK_POLICY_EDF;
            } else {
                return SCHED_BENCHMARK_POLICY_MC;
            }
        }
    } else {
        if (f->cpu_utilization_bp <= 6930) {
            return SCHED_BENCHMARK_POLICY_RMS;
        } else {
            return SCHED_BENCHMARK_POLICY_EDF;
        }
    }
}
```
