# SchedTiny System Specification

**Version:** 0.1.0-draft  
**Status:** Draft — pending first experimental validation  
**Owner:** Lead Researcher  
**Last Updated:** 2026-07-31

---

## 1. Purpose

This document defines the **testable functional and non-functional requirements**
for SchedTiny. It is the contract between the research goals (from
`references/research.md`) and the implementation (in `firmware/`). Every
requirement here must be traceable to:

1. A research gap in `references/research.md` Section C, or
2. A research contribution in `references/research.md` Section D.

If a feature cannot be traced to either, it is out of scope.

---

## 2. Research Gaps Addressed (Traceability Root)

From `references/research.md` Section C:

| Gap ID | Gap Description | SchedTiny Requirement |
|---|---|---|
| GAP-1 | No open mixed-criticality benchmark | REQ-BENCH-001 to REQ-BENCH-005 |
| GAP-2 | Scheduling-aware latency models absent | REQ-SCHED-003, REQ-MEAS-002 |
| GAP-3 | Interrupt-induced jitter not quantified | REQ-MEAS-001, REQ-MEAS-003 |
| GAP-4 | No RTOS-integrated TinyML scheduler hooks | REQ-SCHED-001, REQ-SCHED-002 |
| GAP-5 | Energy-aware scheduling underexplored | REQ-MEAS-004 |
| GAP-6 | Formal schedulability analysis for ML tasks | REQ-SCHED-004 |
| GAP-7 | No reproducible task models | REQ-CONFIG-001 |
| GAP-8 | Cross-MCU generalization missing | REQ-PLATFORM-001 |

---

## 3. System Context

SchedTiny runs on a bare STM32 microcontroller. No Linux, no network, no OS other
than FreeRTOS. The system boundary is:

```
[Physical World] → [Sensors (MPU6050)] → [STM32 + FreeRTOS] → [UART/GPIO]
                                                ↑
                                     [Configuration (YAML → C header)]
```

**External interfaces:**
- UART: structured JSON log output (measurement data)
- GPIO: logic analyzer trigger markers (timing ground truth)
- INA219 via I²C: real-time power measurement
- JTAG/SWD: programming and debugging

---

## 4. Functional Requirements

### 4.1 Scheduler (REQ-SCHED)

**REQ-SCHED-001: ISR Hook API**  
The scheduler SHALL provide a hook mechanism that allows interrupt service routines
(ISRs) to notify the scheduler of their entry and exit, enabling jitter attribution
to specific interrupt sources.

*Trace:* GAP-4, Contribution D.1 in `references/research.md`

**REQ-SCHED-002: Jitter-Bounded Inference API**  
The scheduler SHALL expose an API function `sched_infer_with_budget(budget_us)` that
runs TinyML inference within a specified CPU budget, yielding to higher-priority tasks
when the budget is exceeded.

*Trace:* GAP-4, Contribution D.8

**REQ-SCHED-003: Policy Abstraction**  
The scheduler SHALL support the following scheduling policies, selectable at
compile time via a configuration header:
- Fixed-Priority Preemptive (FPP) — FreeRTOS default
- Rate-Monotonic (RM)
- Earliest Deadline First (EDF) — requires FreeRTOS extension

*Trace:* GAP-2, Baselines H in `references/research.md`

**REQ-SCHED-004: Schedulability Analysis Support**  
The system SHALL expose WCET estimates for all tasks (measured by the bench layer)
in a format compatible with response-time analysis (RTA) calculations in Python.

*Trace:* GAP-6

### 4.2 Measurement (REQ-MEAS)

**REQ-MEAS-001: Interrupt Latency Measurement**  
The bench layer SHALL measure the time delta between ISR entry and the resumption
of the interrupted task, with a resolution of ≤ 1 µs using the DWT cycle counter.

*Trace:* GAP-3

**REQ-MEAS-002: Inference Latency Measurement**  
The bench layer SHALL record inference latency (start-to-end) with DWT cycle counter
resolution, storing at minimum: minimum, maximum, mean, p95, p99 over a configurable
window size (default: 1000 samples).

*Trace:* GAP-2, Metric G.1 in `references/research.md`

**REQ-MEAS-003: Jitter Distribution**  
The bench layer SHALL record the distribution of interrupt-induced latency deltas
for each ISR source separately (DMA, TIM, USART, etc.).

*Trace:* GAP-3, Metric G.2

**REQ-MEAS-004: Energy Measurement**  
The system SHALL integrate with INA219 over I²C to record energy consumption per
inference task execution, with a resolution of ≤ 1 mW average power.

*Trace:* GAP-5, Metric G.2

**REQ-MEAS-005: Deadline Miss Rate**  
The bench layer SHALL record the number and rate of deadline misses for each
periodic task, reported over a configurable observation window.

*Trace:* Metric G.4 in `references/research.md`

**REQ-MEAS-006: Structured Log Output**  
All measurements SHALL be output as newline-delimited JSON records over UART at
115200 baud, with fields: `{timestamp_us, event_type, task_id, value_us}`.
This format is consumed by `scripts/analysis/parse_uart_log.py`.

### 4.3 Benchmark Tasks (REQ-BENCH)

**REQ-BENCH-001: PID Control Task**  
The system SHALL implement a periodic PID control task with:
- Configurable period T_pid (default: 1 ms)
- Configurable worst-case execution time budget C_pid (default: 200 µs)
- Hard deadline equal to its period

**REQ-BENCH-002: Sensor Acquisition Task**  
The system SHALL implement a periodic sensor acquisition task reading MPU6050
accelerometer/gyroscope data via I²C, with configurable period T_sensor.

**REQ-BENCH-003: KWS Inference Task**  
The system SHALL implement a periodic TinyML inference task running the quantized
DS-CNN Keyword Spotting model (TFLM, INT8) with:
- Configurable period T_infer (default: 40 ms for 25 Hz inference)
- Soft deadline (deadline misses are recorded but do not trigger faults)

**REQ-BENCH-004: ISR Load Generator**  
The system SHALL implement a configurable ISR load generator using hardware timers
(TIM2, TIM3), producing interrupts at a configurable rate (0–10000 Hz) to simulate
realistic interrupt environments.

**REQ-BENCH-005: Bare-Metal Baseline**  
The system SHALL support a bare-metal (no RTOS) configuration for baseline
comparison, runnable via a compile-time switch `SCHED_BARE_METAL=1`.

### 4.4 Configuration (REQ-CONFIG)

**REQ-CONFIG-001: YAML-Driven Parameters**  
All numerical experimental parameters (task periods, WCET budgets, ISR rates,
priority levels, window sizes) SHALL be defined in `configs/*.yaml` and translated
to C header files at CMake configure time via a Python code-generation script.
No magic numbers shall appear in firmware source.

*Trace:* GAP-7, Improvement #2 in `docs/ARCHITECTURE.md`

**REQ-CONFIG-002: Compile-Time Configuration**  
Configuration changes SHALL NOT require source code modification. All parameters
are settable via `-DCONFIG_FILE=path/to/config.yaml` at CMake configure time.

### 4.5 Platform (REQ-PLATFORM)

**REQ-PLATFORM-001: Single-Board Support (V1)**  
The firmware SHALL compile and run on the primary STM32 Nucleo board:
- NUCLEO-H743ZI2 (Cortex-M7, 480 MHz)

Board selection is done via `-DBOARD=<name>` CMake parameter.

---

## 5. Non-Functional Requirements

| ID | Requirement | Acceptance Criterion |
|---|---|---|
| NFR-001 | Measurement overhead < 1% CPU | Bench layer uses DWT (no interrupt) |
| NFR-002 | Log output does not block tasks | UART output uses DMA transfer |
| NFR-003 | Reproducible builds | Same SHA produces bit-identical ELF |
| NFR-004 | Memory footprint documented | Flash/SRAM breakdown in ELF map file |
| NFR-005 | All results reproducible | `run.sh` reproduces results ±5% on same HW |
| NFR-006 | Third-party SHAs pinned | `firmware/third_party/README.md` lists exact SHAs |

---

## 6. Out of Scope

The following are explicitly **not** requirements for SchedTiny v1:

- GPU, NPU, or FPGA acceleration
- Multi-core scheduling (STM32H7 dual-core)
- Over-the-air model updates
- Linux-based embedded platforms
- Custom silicon

---

## 7. Revision History

| Version | Date | Author | Change |
|---|---|---|---|
| 0.1.0-draft | 2026-07-31 | @nithingoud78 | Initial draft from research.md |
