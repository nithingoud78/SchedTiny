# ASSUMPTIONS.md

This document lists the foundational engineering and research assumptions made by the SchedTiny framework.
If any of these assumptions are proven false, the architecture or methodology must be re-evaluated.

## Hardware Assumptions
1. **DWT Reliability:** The Cortex-M7 DWT cycle counter provides accurate, zero-overhead execution timing without being affected by pipeline stalls in a way that breaks deterministic measurement.
2. **UART DMA Non-Interference:** UART TX via DMA does not introduce significant bus contention that artificially inflates task WCET.

## Software Assumptions
1. **Preemptible Inference:** TFLM/CMSIS-NN matrix multiplication kernels can be preempted by higher-priority FreeRTOS tasks (like PID control) without corrupting internal model state.
2. **Overhead Bounding:** The overhead introduced by `sched_isr_on_entry()` and `bench_log_event()` is negligible relative to the task execution times.
