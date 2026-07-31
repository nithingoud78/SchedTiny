# TODO.md

Research-driven checklist for SchedTiny implementation.
This document tracks active work items required before experiments can begin.

## Phase 1: Framework Implementation
- [ ] Implement `scripts/build/generate_config.py` full logic
- [ ] Write `firmware/core/scheduler/sched_core.c`
- [ ] Write `firmware/core/scheduler/sched_isr.c`
- [ ] Write `firmware/core/bench/bench_measure.c`
- [ ] Implement `firmware/core/drivers/drv_timer.c` (DWT)
- [ ] Implement `firmware/core/drivers/drv_uart.c` (DMA)
- [ ] Integrate FreeRTOS source in CMake

## Phase 2: Workload Implementation
- [ ] Write `firmware/research/tasks/task_pid.c`
- [ ] Write `firmware/research/tasks/task_sensor.c`
- [ ] Write `firmware/research/tasks/task_infer.c`
- [ ] Setup TFLM and CMSIS-NN integration

## Phase 3: Validation
- [ ] Verify DWT overflow handling (Q-001)
- [ ] Verify TFLM preemptibility (Q-002)
- [ ] Verify TIM2 ISR worst-case execution time (Q-006)
