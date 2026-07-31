# EXP-001: Baseline — Bare-Metal KWS Inference (No RTOS)

## Experiment Header

- **Experiment ID:** EXP-001
- **Status:** Planned
- **Researcher:** @nithingoud78
- **Date Started:** —
- **Date Completed:** —
- **Hardware:** STM32 NUCLEO-H743ZI2
- **Firmware SHA:** *(fill after flash)*
- **Config file:** `experiments/EXP-001_baseline_baremetal/config.yaml`
- **Related Gap:** GAP-1 (No open mixed-criticality benchmark)
- **Related Issue:** *(open issue to claim)*

---

## Hypothesis

We hypothesize that bare-metal (no RTOS) KWS inference on the NUCLEO-H743ZI2 will
produce the minimum achievable latency and near-zero jitter (< 5 µs standard deviation),
serving as the lower-bound baseline for all subsequent RTOS and scheduling experiments.

This establishes the "zero scheduling overhead" reference point from which all
FreeRTOS-introduced overhead will be measured.

---

## Independent Variables

| Variable | Values | Units |
|---|---|---|
| Execution mode | bare-metal (no FreeRTOS) | — |
| ISR load | 0 (none) | Hz |

---

## Dependent Variables

| Metric | Measurement Method | Resolution |
|---|---|---|
| KWS inference latency: min, mean, p95, p99 | DWT cycle counter | 2 ns |
| Jitter (std dev of latency) | DWT cycle counter | 2 ns |
| Flash usage | ELF .map file | bytes |
| SRAM usage | ELF .map file | bytes |

---

## Controlled Variables

- Execution mode: bare-metal (SCHED_BARE_METAL=1, no FreeRTOS)
- ISR load: 0 Hz (none)
- Board: NUCLEO-H743ZI2 (Cortex-M7, 480 MHz)
- Compiler: arm-none-eabi-gcc, -O2
- I-cache: enabled
- D-cache: enabled
- Sample count: 1000 consecutive KWS inferences
- Warmup: 100 samples discarded
- Input data: fixed synthetic input (MFCC feature vector, all zeros)
- Ambient temperature: room temperature, documented in session note

---

## Expected Outcome

We expect:
- Mean latency: ~20–40 ms (DS-CNN on Cortex-M7 @ 480 MHz, INT8 CMSIS-NN)
  *(baseline estimate from ST MLPerf Tiny v1.3 results for KWS)*
- Jitter: < 5 µs (no interrupt sources)
- Flash: < 500 KB (model + runtime)
- SRAM: < 400 KB (tensor arena 256 KB + stack)

---

## Literature Context

ST reports 48 inferences/sec for KWS on STM32U385RG (Cortex-M33 @ 160 MHz)
[ST MLPerf Tiny v1.3]. The H743ZI2 at 480 MHz should achieve significantly
better throughput. This experiment provides the SchedTiny-specific baseline.

---

## Actual Outcome

*(Fill in after running)*

---

## Interpretation

*(Fill in after analysis)*

---

## References

- `[stmicro2025mlperf]` — ST MLPerf Tiny v1.3 results, KWS @ 48 inferences/sec
- `[banbury2020benchmarking]` — TinyML benchmarking methodology (arXiv 2003.04821)
