# SchedTiny Open Research Questions

This document tracks unanswered research questions that may block progress on
experiments or the paper. When a question is answered, move it to
`references/research.md` Section I or the relevant experiment README.

See `docs/RESEARCH_GUIDE.md` for the process to resolve questions.

---

## Q-001: DWT Overflow Handling

**Question:** At 480 MHz, DWT->CYCCNT overflows every ~8.9 seconds. What is the
minimum measurement window that requires overflow handling?

**Impact:** Affects `firmware/src/bench/bench_measure.c` implementation.

**Status:** 🔄 Open

**Proposed resolution:** Use 64-bit accumulator in `bench_dwt_cycles64()` with
ISR-based overflow detection. Verify against logic analyzer capture.

---

## Q-002: TFLM Preemptibility

**Question:** Can TensorFlow Lite Micro inference be safely preempted mid-operation
without corrupting the model state? Specifically: can a FreeRTOS higher-priority task
preempt the inference task during a matrix multiplication kernel in CMSIS-NN?

**Impact:** Determines whether `sched_infer_with_budget()` can yield to PID task
without invoking TFLM cleanup.

**Status:** 🔄 Open

**Literature reference:** None found. This is a gap in the literature.

---

## Q-003: FreeRTOS EDF Port Availability

**Question:** Is there a maintained, production-quality EDF extension for FreeRTOS
that is compatible with Cortex-M7 and vendorable under a permissive license?

**Impact:** Required for EDF experiments (EXP-003 and beyond).

**Status:** 🔄 Open

**Candidates investigated so far:**
- Litmus^RT: research OS, not FreeRTOS-compatible
- FreeRTOS EDF extension (academic): not actively maintained
- Custom implementation: possible but adds scope

---

## Q-004: INA219 Sampling Rate vs. Inference Duration

**Question:** The INA219 maximum sampling rate is ~860 Hz (1.16 ms per sample).
If KWS inference takes ~20–30 ms, we get ~20 energy samples per inference.
Is this sufficient resolution for energy-per-inference measurement?

**Impact:** Affects power measurement methodology in `firmware/src/hal/hal_power.c`.

**Status:** 🔄 Open

**Proposed resolution:** Average INA219 readings over the inference window;
compare with GPIO-triggered logic analyzer to verify window alignment.

---

## Q-005: TFLM Tensor Arena Size for DS-CNN

**Question:** What is the exact tensor arena size required for the quantized DS-CNN
KWS model on Cortex-M7 with CMSIS-NN INT8 kernels?

**Impact:** Determines `BOARD_TFLM_ARENA_SIZE_BYTES` in `board_config.h`.
Current estimate: 256 KB. May be wrong.

**Status:** 🔄 Open

**Proposed resolution:** Run `model_runner_init()` with binary search on arena size;
record minimum that produces MODEL_OK.

---

## Q-006: Worst-Case ISR Duration for TIM2

**Question:** What is the worst-case execution time of the TIM2 ISR at maximum
load generator rate (10 kHz)?

**Impact:** Required for schedulability analysis (REQ-SCHED-004).

**Status:** 🔄 Open

**Proposed resolution:** Measure with DWT in EXP-001 (bare-metal) before any
task scheduling is running. This gives a clean ISR WCET.

---

## Q-007: Schedulability Bound with TFLM Task

**Question:** What is the maximum CPU utilization achievable by the task set
(PID + sensor + TinyML) that is still schedulable under FPP with interrupt load?
Is this above or below the RM utilization bound (69.3% for 3 tasks)?

**Impact:** Core theoretical contribution of the paper.

**Status:** 🔄 Open (this is a primary research question, not a blocker)
