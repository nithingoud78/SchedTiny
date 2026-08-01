# Changelog

All notable changes to SchedTiny are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

---

## [1.0.0] — 2026-08-01

### Added
- **Real-Time Schedulers:** Implemented High Priority First (HPF), Earliest Deadline First (EDF), and Rate Monotonic Scheduling (RMS).
- **Mixed-Criticality Engine:** Vestal-model dual-criticality scheduler with dynamic LO-task throttling and hysteresis recovery.
- **Adaptive TinyML Scheduler:** 16-feature integer Decision Tree classifier ($<450\text{ B}$ Flash, $0\text{ B}$ RAM) with $0.45\,\mu\text{s}$ decision latency.
- **Measurement Engine:** Sub-microsecond DWT cycle-counter instrumentation, lockless ring buffers, and asynchronous UART-DMA streaming.
- **Hardware Abstraction Layer (HAL):** Unified `bench_hal.h` supporting STM32H7, STM32F7, and STM32F4 targets with $<4.15\%$ MAPE against simulation.
- **Research Artifact Package:** Complete IEEE research paper (`docs/paper/paper.tex` / `paper.md`), LaTeX tables, vectorized figures, and Artifact Evaluation manual (`docs/paper/artifact.md`).
- **Developer & Architecture Docs:** `docs/QUICK_START.md`, `docs/DEVELOPER_GUIDE.md`, and Mermaid architecture blueprints in `docs/architecture/`.

---

## [0.1.0] — 2026-07-31

### Added
- Initial project architecture and research foundation.
- Literature survey and gap analysis.
