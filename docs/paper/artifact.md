# SchedTiny: Artifact Evaluation & Reproduction Guide

This document specifies the complete Artifact Evaluation package for reproducing all empirical evaluations, hardware benchmarks, and TinyML validation experiments described in the SchedTiny research paper.

---

## 1. Hardware Requirements

### Host Machine (Simulation & Analysis)
- **Architecture:** x86_64 or ARM64 (Linux, macOS, or Windows 10/11 with WSL2/PowerShell)
- **CPU:** 4+ cores, 2.0 GHz or higher
- **RAM:** 4 GB minimum (8 GB recommended)
- **Disk Space:** 2 GB free disk space

### Target Embedded Hardware (Physical Validation - Optional)
- **Recommended Board:** STM32H743ZI2 Nucleo-144 (ARM Cortex-M7 @ 480 MHz)
- **Alternative Boards:**
  - STM32F746ZG Nucleo-144 (ARM Cortex-M7 @ 216 MHz)
  - STM32F401RE Nucleo-64 (ARM Cortex-M4 @ 84 MHz)
  - STM32F407VG Discovery (ARM Cortex-M4 @ 168 MHz)
- **Debugger / Programmer:** Onboard ST-LINK/V2-1 or ST-LINK/V3 via Micro-USB
- **Serial Interface:** FTDI or onboard Virtual COM Port (115200 baud, 8-N-1)

---

## 2. Software & Toolchain Prerequisites

### Compiler & Build Systems
- **Host C Compiler:** `gcc` / `clang` / `MSVC` (C11 support required)
- **Target Cross Compiler:** `arm-none-eabi-gcc` v10.3+ (for STM32 flashing)
- **Build Generator:** `CMake` $\ge 3.20$ and `Ninja` or `make`
- **Unit Testing Framework:** `cmocka` or bundled mock suite

### Python Environment
- **Python Version:** Python 3.9+ (Python 3.10 / 3.11 tested)
- **Required Packages:** Install via `pip install -r requirements.txt`:
  ```txt
  numpy>=1.22.0
  pandas>=1.4.0
  scipy>=1.8.0
  scikit-learn>=1.1.0
  matplotlib>=3.5.0
  seaborn>=0.12.0
  tabulate>=0.9.0
  pyyaml>=6.0
  ```

---

## 3. Repository Layout

```
SchedTiny/
├── firmware/
│   ├── core/               # Core scheduler, bench measurement, drivers, TinyML hooks
│   ├── include/            # Public API headers (bench_hal.h, sched_policy.h, etc.)
│   └── tests/              # CMocka-based C unit and regression tests
├── scripts/
│   ├── analysis/           # Metric computation, Gantt visualizer, hardware plotting
│   ├── workloads/          # Synthetic workload generators (Periodic, Fault, MC)
│   ├── run_all_experiments.py # Complete automated benchmarking campaign runner
│   ├── train_scheduler_model.py # TinyML decision tree trainer
│   ├── export_decision_tree.py  # C header code generator
│   └── make_report.py      # Automated report aggregator
├── docs/
│   └── paper/              # Full research paper (paper.tex, paper.md, tables, figures)
└── datasets/               # Workload profiles and benchmark traces
```

---

## 4. Build Instructions

### 4.1 Building Host Unit Tests
```bash
cmake -S firmware/tests -B build_tests
cmake --build build_tests
ctest --test-dir build_tests --output-on-failure
```

### 4.2 Building STM32 Embedded Firmware
```bash
cmake -S firmware -B build_stm32 -DCMAKE_TOOLCHAIN_FILE=cmake/arm-none-eabi.cmake -DMCU_VARIANT=STM32H7
cmake --build build_stm32
```

---

## 5. Experiment Reproduction & Workflow

### Step 1: Run Full Benchmark Campaign
To execute all scheduler policies (HPF, EDF, RMS, MC, Adaptive) across synthetic, periodic, and fault-injection workloads:
```bash
python scripts/run_all_experiments.py --workloads periodic,random,fault,mixed_criticality --runs 10
```

### Step 2: Train and Export TinyML Model
Train the 16-feature Decision Tree model and export it to embedded C:
```bash
python scripts/train_scheduler_model.py --dataset datasets/benchmark_summary.csv --output models/
python scripts/export_decision_tree.py --model models/decision_tree.joblib --output firmware/include/sched_adaptive_model.h
python scripts/evaluate_scheduler_model.py --model models/decision_tree.joblib --dataset datasets/benchmark_summary.csv
```

### Step 3: Run Hardware Validation & Plotting
Generate comparison metrics and IEEE-compliant figures:
```bash
python scripts/analysis/compare_hardware.py --sim-json results/sim_benchmark.json --hw-json results/stm32h7_benchmark.json
python scripts/analysis/plot_hardware.py --out-dir docs/paper/figures/
python scripts/make_report.py --input-dir results/ --output-file docs/paper/benchmark_report.md
```

---

## 6. Expected Outputs

| Output File | Description | Expected Values / Trend |
|---|---|---|
| `docs/paper/figures/hw_radar_chart.pdf` | MCU performance radar | STM32H7 dominates throughput; F4 dominates energy efficiency |
| `docs/paper/figures/hw_latency_comparison.pdf` | Scheduling latency across policies | Adaptive latency $\approx 2.15\,\mu\text{s}$, RMS $\approx 1.95\,\mu\text{s}$ |
| `docs/paper/figures/hw_error_distribution.pdf` | Boxplot of validation errors | MAE $< 5.0\%$, MAPE $< 4.15\%$ |
| `firmware/include/sched_adaptive_model.h` | Generated integer tree | Static const code $< 500\text{ bytes}$ Flash, $0\text{ bytes}$ RAM |

---

## 7. Dataset Descriptions

- **`datasets/periodic_tasks.csv`:** 500+ periodic task permutations with periods between 1 ms and 500 ms, utilization $0.1 \le U \le 0.95$.
- **`datasets/fault_injection_traces.csv`:** Execution profiles simulating transient execution overruns, memory faults, and deadline pressures.
- **`datasets/mixed_criticality_workloads.csv`:** Dual-criticality workload configurations matching the Vestal specification.

---

## 8. Licensing and Accessibility

All source code, scripts, datasets, and documentation are provided under the open-source **Apache License 2.0**.
