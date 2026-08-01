# SchedTiny: Quick Start Guide

This guide provides step-by-step instructions to clone, build, test, and execute the complete SchedTiny benchmark, TinyML training, and hardware validation workflow from scratch.

---

## 1. Prerequisites

Ensure your development environment contains:
- **Git** ($\ge 2.30$)
- **CMake** ($\ge 3.20$) & **Ninja** or **Make**
- **C Compiler:** GCC, Clang, or MSVC (C11 support)
- **Target Cross-Compiler (Optional for STM32 targets):** `arm-none-eabi-gcc` v10.3+
- **Python 3.9+** (Python 3.10 / 3.11 recommended)

---

## 2. Setup and Installation

```bash
# Clone repository
git clone https://github.com/nithingoud78/SchedTiny.git
cd SchedTiny

# Install Python analysis & ML dependencies
pip install -r requirements.txt
```

---

## 3. Build and Run Firmware Unit Tests

SchedTiny includes comprehensive CMocka-based host unit tests for all schedulers, data structures, and HAL timing routines:

```bash
# Configure and build test suite
cmake -S firmware/tests -B build_tests
cmake --build build_tests

# Execute all tests
ctest --test-dir build_tests --output-on-failure
```

---

## 4. Run Automated Benchmarking Campaign

To execute the multi-policy benchmark campaign comparing HPF, EDF, RMS, Mixed-Criticality, and Adaptive scheduling:

```bash
# Run multi-workload benchmark campaign (10 runs per configuration)
python scripts/run_all_experiments.py --workloads periodic,random,fault,mixed_criticality --runs 10
```

Raw metrics and summaries will be written to `datasets/benchmark_summary.csv`.

---

## 5. Train and Deploy the Adaptive TinyML Model

Train the 16-feature Decision Tree model and export it as an embedded C header:

```bash
# 1. Train Decision Tree classifier
python scripts/train_scheduler_model.py --dataset datasets/benchmark_summary.csv --output models/

# 2. Evaluate model performance & metrics
python scripts/evaluate_scheduler_model.py --model models/decision_tree.joblib --dataset datasets/benchmark_summary.csv

# 3. Export to embedded C header (sched_adaptive_model.h)
python scripts/export_decision_tree.py --model models/decision_tree.joblib --output firmware/include/sched_adaptive_model.h
```

---

## 6. Hardware Validation & Comparison

Compare host simulation traces against physical STM32 Nucleo hardware execution traces:

```bash
# Compare simulation vs STM32 hardware JSON traces
python scripts/analysis/compare_hardware.py --sim-json results/sim_benchmark.json --hw-json results/stm32h7_benchmark.json

# Generate publication-quality hardware validation figures
python scripts/analysis/plot_hardware.py --out-dir docs/paper/figures/
```

---

## 7. Generate Research Report & Figures

Compile the aggregated benchmark results into publication-ready figures and markdown reports:

```bash
# Generate comprehensive markdown report
python scripts/make_report.py --input-dir results/ --output-file docs/paper/benchmark_report.md
```

---

## 8. Cross-Compiling for Target STM32 Hardware (Physical Flashing)

```bash
# Build firmware for STM32H743ZI2 target
cmake -S firmware -B build_stm32 \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DMCU_VARIANT=STM32H7
cmake --build build_stm32

# Flash binary via OpenOCD or STM32CubeProgrammer
openocd -f interface/stlink.cfg -f target/stm32h7x.cfg -c "program build_stm32/schedtiny.elf verify reset exit"
```
