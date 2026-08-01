# SchedTiny

**An Open Benchmark and Interrupt-Aware Scheduling Framework for Co-Running Real-Time Control Tasks and TinyML Inference on STM32 Microcontrollers**

[![License](https://img.shields.io/badge/License-Apache%202.0-blue.svg)](LICENSE)
[![Build](https://github.com/nithingoud78/SchedTiny/actions/workflows/ci.yml/badge.svg)](https://github.com/nithingoud78/SchedTiny/actions/workflows/ci.yml)
[![Docs](https://github.com/nithingoud78/SchedTiny/actions/workflows/docs.yml/badge.svg)](https://github.com/nithingoud78/SchedTiny/actions/workflows/docs.yml)

---

## What Is SchedTiny?

SchedTiny is a research-grade, open-source framework that addresses a specific and
well-documented gap in the embedded ML literature: **no publicly available benchmark
exists that co-runs real-time control tasks and TinyML inference under interrupt-aware
scheduling on STM32 microcontrollers.**

Most TinyML benchmarks (including MLPerf Tiny) measure single-task inference in
isolation. Real embedded systems run PID control loops, sensor acquisition tasks,
communication handlers, and ML inference simultaneously — all competing for the same
CPU under interrupt pressure. The scheduling interactions that result are poorly
understood and largely unmeasured.

SchedTiny provides:

- **An open benchmark suite** — reproducible workloads (Keyword Spotting + PID
  control) on STM32 Nucleo boards (H743ZI2, F767ZI, L496ZG) with FreeRTOS
- **An interrupt-aware scheduler** — a FreeRTOS-compatible scheduler that explicitly
  models ISR interactions and provides jitter-bounded inference
- **A measurement framework** — first-class instrumentation for latency, jitter,
  energy, CPU utilization, and deadline miss rate
- **A research pipeline** — from YAML-configured experiments to journal-quality figures

---

## Research Context

SchedTiny directly addresses the following confirmed research gaps
(see `references/research.md`, Section C):

| Gap | SchedTiny Contribution |
|---|---|
| No open mixed-criticality benchmark | Open benchmark suite (EXP-001 to EXP-007) |
| Scheduling-aware latency models absent | Interrupt-aware WCET characterization |
| Interrupt-induced jitter not quantified | Systematic ISR jitter measurement API |
| No RTOS-integrated TinyML scheduler hooks | `sched_isr.h` ISR hook API |
| Energy-aware scheduling underexplored | INA219-based energy measurement integration |
| No reproducible task models | YAML-configured task parameter system |

**Baselines compared:** Bare-metal loop, FreeRTOS fixed-priority, Rate-Monotonic, EDF.  
**Models:** Quantized DS-CNN for KWS; anomaly detection MLP.  
**Primary hardware:** STM32 NUCLEO-H743ZI2 (Cortex-M7 @ 480 MHz, 1 MB SRAM).

---

## Repository Structure

```
SchedTiny/
├── docs/               # All documentation (ARCHITECTURE, SPEC, guides)
├── firmware/           # Embedded C source (scheduler, HAL, bench, ML, tasks)
├── models/             # TFLite models + C arrays + training scripts
├── datasets/           # Dataset acquisition scripts (no large binaries in git)
├── experiments/        # Self-contained, numbered experiment definitions
├── scripts/            # Python analysis and build automation
├── results/            # Processed metrics and journal figures (raw logs gitignored)
├── configs/            # YAML configuration — single source of truth for parameters
├── papers/             # LaTeX journal paper source
├── references/         # Literature survey, gap analysis, bibliography
├── hardware/           # Schematics, BOM, measurement rig documentation
├── tools/              # Doxygen, clang-format, CMake toolchain files
├── tests/              # Host-side integration tests
├── ci/                 # Docker reproducible build environment
├── examples/           # Minimal working examples for new contributors
├── assets/             # Diagrams and project media
└── meeting_notes/      # Research session diary
```

For the full design rationale, see [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md).

---

## Quick Start

### Prerequisites

- ARM GNU Toolchain 12.3+
- CMake 3.25+
- STM32CubeProgrammer 2.14+
- Python 3.12+
- STM32 NUCLEO-H743ZI2 board (or F767ZI / L496ZG)

See [docs/BUILD_GUIDE.md](docs/BUILD_GUIDE.md) for full installation instructions.

### Build (CMake)

```bash
# Configure for the primary target board
cmake -S firmware -B build \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DBOARD=nucleo_h743zi2 \
      -DEXPERIMENT=EXP-001

# Build
cmake --build build --parallel

# Flash
python scripts/build/flash.py --board nucleo_h743zi2 --elf build/schedtiny.elf
```

### Run an Experiment

```bash
# Navigate to experiment
cd experiments/EXP-001_baseline_freertos

# Read the hypothesis first (required)
cat README.md

# Run (flashes board, captures UART, stores raw results)
bash run.sh

# Analyze
jupyter lab analysis.ipynb
```

---

## Hardware

| Board | MCU | Core | Clock | SRAM | Flash | Status |
|---|---|---|---|---|---|---|
| NUCLEO-H743ZI2 | STM32H743ZI | Cortex-M7 | 480 MHz | 1 MB | 2 MB | ✅ Primary |
| NUCLEO-F767ZI | STM32F767ZI | Cortex-M7 | 216 MHz | 512 KB | 2 MB | 🔄 Planned |
| NUCLEO-L496ZG | STM32L496ZG | Cortex-M4 | 80 MHz | 320 KB | 1 MB | 🔄 Planned |

**Sensors:** MPU6050 (IMU), INA219 (power measurement)  
**Measurement:** Logic Analyzer (24 MHz) for GPIO timing markers

See [docs/HARDWARE_GUIDE.md](docs/HARDWARE_GUIDE.md) for wiring and measurement rig setup.

---

## Software Stack

| Component | Technology | Version |
|---|---|---|
| RTOS | FreeRTOS | 10.6.x (pinned) |
| TinyML Runtime | TensorFlow Lite Micro | pinned commit |
| NN Kernels | CMSIS-NN | pinned commit |
| Toolchain | ARM GNU | 12.3+ |
| Build System | CMake | 3.25+ |
| Analysis | Python + pandas + matplotlib | 3.12+ |

---

## Evaluation Metrics

SchedTiny measures (see [docs/SPEC.md](docs/SPEC.md)):

- **Inference latency:** mean, p95, p99 (µs)
- **Scheduling jitter:** interrupt-induced latency delta (µs)
- **Energy per inference:** µJ (INA219)
- **CPU utilization:** % per task (FreeRTOS task stats)
- **Deadline miss rate:** % of control task deadlines missed
- **Memory footprint:** SRAM and Flash usage (bytes)

---

---

## System Architecture

```
+-------------------------------------------------------------------------+
|                        SchedTiny System Framework                       |
+-------------------------------------------------------------------------+
| [Workload Layer]                                                        |
|   Periodic (PID)  |  Fault Injection  |  Mixed-Criticality (Vestal)     |
+-------------------------------------------------------------------------+
| [Adaptive Decision Engine]                                              |
|   16-Dim Feature Extractor  -->  Integer Decision Tree (<450B Flash)    |
|   Selects: HPF (Static) | EDF (Dynamic) | RMS (Optimal) | MC (Degraded) |
+-------------------------------------------------------------------------+
| [OS & Measurement Kernel]                                               |
|   DWT Cycle-Counter Hooks (<0.1us)  |  UART-DMA Streaming Engine        |
+-------------------------------------------------------------------------+
| [Hardware Abstraction Layer (HAL)]                                      |
|   STM32H7 (480 MHz)  |  STM32F7 (216 MHz)  |  STM32F4 (168/84 MHz)      |
+-------------------------------------------------------------------------+
```

---

## Research Publication & Artifact Evaluation

SchedTiny is packaged with a complete IEEE publication manuscript and reproducible artifact evaluation suite:
- **Research Paper (LaTeX):** [`docs/paper/paper.tex`](docs/paper/paper.tex)
- **Research Paper (Markdown):** [`docs/paper/paper.md`](docs/paper/paper.md)
- **Artifact Evaluation Guide:** [`docs/paper/artifact.md`](docs/paper/artifact.md)
- **Mathematical Appendix:** [`docs/paper/appendix.md`](docs/paper/appendix.md)

### Research Roadmap
- [x] **Phase 1-17:** Core RTOS, DWT profiling, mixed-criticality engine, fault injection.
- [x] **Phase 18:** Automated research evaluation & statistical comparison pipeline.
- [x] **Phase 19:** TinyML scheduler training, integer tree generation & validation.
- [x] **Phase 20:** Cross-platform STM32 hardware characterization.
- [x] **Phase 21:** IEEE publication packaging, release v1.0.0 artifacts.
- [ ] **Future (v1.1+):** On-device reinforcement learning, dual-core Cortex-M7/M4 asymmetric scheduling.

---

## Citing SchedTiny

If you use SchedTiny in your research, please cite our paper:

```bibtex
@article{schedtiny2026,
  author  = {Nithin Goud},
  title   = {SchedTiny: An Interrupt-Aware, Multi-Criticality Adaptive Real-Time Scheduling and TinyML Framework for STM32 Microcontrollers},
  journal = {IEEE Transactions on Computer-Aided Design of Integrated Circuits and Systems (TCAD)},
  year    = {2026}
}
```

---

## Contributing

Please read [CONTRIBUTING.md](CONTRIBUTING.md) before opening a pull request.

Key points:
- Every experimental result must come from real hardware
- Every experiment must be reproducible from `run.sh`
- Use Conventional Commits; all PRs target the `dev` branch
- Follow `docs/CODING_STANDARD.md` for C and `docs/STYLE_GUIDE.md` for Python

---

## License

Copyright 2026 SchedTiny Contributors

Licensed under the [Apache License, Version 2.0](LICENSE).

Third-party components:
- FreeRTOS — MIT License
- TensorFlow Lite Micro — Apache 2.0
- CMSIS-NN — Apache 2.0

---

## Contact

- **GitHub Issues:** Bug reports, feature requests, experiment failures
- **GitHub Discussions:** Research questions, design discussions
- **Repository:** [github.com/nithingoud78/SchedTiny](https://github.com/nithingoud78/SchedTiny)
