# SchedTiny Roadmap

> **Status:** Pre-publication research framework  
> **License:** Apache 2.0  
> **Target:** IEEE journal submission + Germany MS portfolio + long-term open-source

This roadmap tracks planned milestones at the research, implementation, and
publication level. It is updated after each research session (see `meeting_notes/`).

---

## Milestone 0 — Foundation (Current)

**Goal:** Repository skeleton, documentation, and build system.  
**Status:** 🔄 In Progress

- [x] Literature survey (`references/research.md`, 25 papers)
- [x] Research gap analysis (8 gaps confirmed, see `docs/SPEC.md`)
- [x] Repository architecture design
- [x] License (Apache 2.0), code of conduct, security policy
- [x] `docs/` documentation suite
- [x] `firmware/` skeleton: CMakeLists, HAL, scheduler, bench, ML, board configs
- [x] `experiments/` template
- [x] `scripts/` analysis pipeline
- [x] GitHub Actions CI/CD, issue templates, CODEOWNERS
- [ ] `hardware/` wiring diagrams and measurement rig setup
- [ ] `datasets/` download scripts tested end-to-end
- [ ] Literature matrix completed (`references/literature_matrix.md`)

---

## Milestone 1 — Firmware Core

**Goal:** Functional FreeRTOS + TFLM + measurement layer on H743ZI2.  
**Target:** TBD

- [ ] HAL layer: DWT timer, UART logger, GPIO triggers, INA219 power
- [ ] Benchmark measurement API (`bench_measure.h`)
- [ ] Structured UART log output (JSON format)
- [ ] FreeRTOS task set: PID control + TFLM inference + sensor acquisition
- [ ] Baseline experiment: bare-metal inference (EXP-001)
- [ ] Baseline experiment: FreeRTOS fixed-priority, no interrupt load (EXP-002)
- [ ] Reproducible build verified on H743ZI2

---

## Milestone 2 — Interrupt-Aware Scheduler

**Goal:** Core research contribution implemented and benchmarked.  
**Target:** TBD

- [ ] Interrupt-aware scheduler: ISR hook API (`sched_isr.h`)
- [ ] Jitter measurement: interrupt-induced latency delta quantification
- [ ] Experiment: FreeRTOS FP + interrupt load sweep (EXP-003)
- [ ] Experiment: EDF vs RM vs FP under interrupt load (EXP-004)
- [ ] Experiment: co-running PID + KWS inference under load (EXP-005)
- [ ] Schedulability analysis: response-time analysis for ML tasks (EXP-006)
- [ ] Energy measurement: INA219 integration validated (EXP-007)

---

## Milestone 3 — Benchmark Suite

**Goal:** SchedTiny released as an open, reproducible benchmark.  
**Target:** TBD

- [ ] KWS benchmark: Speech Commands dataset, quantized DS-CNN model
- [ ] Anomaly detection benchmark: synthetic vibration dataset
- [ ] Cross-board benchmark: H743ZI2, F767ZI, L496ZG results
- [ ] MLPerf Tiny compatibility check (single-task baseline)
- [ ] `scripts/analysis/` pipeline produces paper-quality figures
- [ ] All experiments reproducible from `run.sh` on a fresh hardware setup

---

## Milestone 4 — Journal Paper

**Goal:** First journal submission.  
**Target:** TBD

- [ ] Paper draft: `papers/journal_v1/main.tex`
- [ ] All figures generated from `results/` (no hand-drawn)
- [ ] Artifact checklist: IEEE artifact evaluation ready
- [ ] Submission to target venue (IEEE TECS or IEEE ESL)
- [ ] Response to reviewer comments (R1, R2)

---

## Milestone 5 — Community Release

**Goal:** Long-term open-source framework.  
**Target:** Post-publication

- [ ] v1.0.0 tagged release
- [ ] Docker container for reproducible experiments
- [ ] GitHub Pages documentation site (Doxygen + mkdocs)
- [ ] Contributing guide refinement based on early contributor feedback
- [ ] Extension: on-device learning scheduler (future paper)
- [ ] Extension: Zephyr RTOS port
- [ ] Extension: RISC-V board support

---

## Research Questions Tracked

See `references/open_questions.md` for the full list. Active questions:

1. What is the worst-case interrupt-induced jitter on KWS inference on H743ZI2?
2. Can EDF outperform RM in mixed-criticality TinyML + control workloads?
3. What is the schedulability utilization bound for the TFLM task model?
4. How does ISR rate affect the p99 inference latency tail?
5. Is energy-aware scheduling viable without hardware power gating on Cortex-M7?

---

## Not In Scope

The following are explicitly out of scope for SchedTiny v1:

- GPU or NPU acceleration
- Cloud-assisted inference
- Over-the-air model updates
- Multi-core MCU scheduling (STM32H7 dual-core is future work)
- Linux-based embedded systems (Raspberry Pi, etc.)
- Custom silicon or FPGA implementations
