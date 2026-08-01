# SchedTiny Milestone Tracking

Research and development milestone tracking for SchedTiny.

## Phase 1: Framework Implementation
- [x] Implement configuration generation logic
- [x] Write `firmware/core/scheduler/sched_core.c`
- [x] Write `firmware/core/scheduler/sched_isr.c`
- [x] Write `firmware/core/bench/bench_measure.c`
- [x] Implement `firmware/core/drivers/drv_timer.c` (DWT)
- [x] Implement `firmware/core/drivers/drv_uart.c` (DMA)
- [x] Implement FreeRTOS/Baremetal scheduler integrations

## Phase 2: Workload & Scheduler Implementation
- [x] Write periodic PID and control workload tasks
- [x] Implement HPF, EDF, and RMS real-time schedulers
- [x] Implement Vestal-model Mixed-Criticality (MC) scheduler
- [x] Implement 16-feature Adaptive TinyML decision tree scheduler

## Phase 3: Hardware Validation & Research Packaging
- [x] STM32H7 / F7 / F4 hardware characterization (<4.15% MAPE)
- [x] IEEE research paper and publication package (`docs/paper/`)
- [x] Artifact Evaluation guide and automated reproduction pipeline
- [x] v1.0.0 Production Release Preparation
