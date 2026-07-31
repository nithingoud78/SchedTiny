# Experiment Guide

## How to Design, Run, and Document Experiments in SchedTiny

This guide is the authoritative process for all experiments.  
**An experiment that does not follow this process is not eligible for inclusion in the paper.**

---

## Core Principle: Hypothesis First

Every experiment begins with a written hypothesis **before any code is written or run**.
This prevents HARKing (Hypothesizing After Results are Known) — a common and serious
methodological flaw in systems research.

If you cannot state a falsifiable hypothesis, you are exploring, not experimenting.
Exploration is fine, but it must be documented in `references/ideas.md`, not in
an `experiments/EXP-NNN/` folder.

---

## 1. Experiment Categories

Instead of claiming sequential numbers, experiments should be placed directly in their respective category folder (`baseline`, `interrupt`, `scheduler`, `energy`). If an experiment is a variant, it can be named descriptively inside the category folder.

---

## 2. Experiment Folder Structure

Experiments are organized into four canonical categories:

```
experiments/
├── baseline/      ← Bare-metal + RTOS overhead baselines (no ISR load)
├── interrupt/     ← ISR rate sweep experiments
├── scheduler/     ← Policy comparison (FPP vs RM vs EDF)
└── energy/        ← Power measurement experiments
```

Each directory contains a `README.md`, `config.yaml`, `run.sh`, and `analysis.ipynb`.
Sequential numbering (EXP-NNN) can be introduced inside these directories if multiple variants are needed.

---

## 3. Writing the Experiment README

The `README.md` must contain these sections in order:

### 3.1 Experiment Header

```markdown
# EXP-NNN: Short Descriptive Title

- **Status:** Planned / In Progress / Complete / Failed
- **Researcher:** @your_github_handle
- **Date Started:** YYYY-MM-DD
- **Date Completed:** YYYY-MM-DD
- **Hardware:** STM32 NUCLEO-H743ZI2
- **Firmware SHA:** (fill in after run)
- **Related Gap:** GAP-N (from docs/SPEC.md)
```

### 3.2 Hypothesis

Write one falsifiable hypothesis:

> "We hypothesize that increasing the ISR rate from 0 to 5000 Hz on the
> NUCLEO-H743ZI2 will increase the p99 KWS inference latency by more than
> 10× relative to the zero-ISR baseline (EXP-001), due to preemption overhead
> of the FreeRTOS scheduler."

### 3.3 Independent Variables

List exactly what you are changing:

| Variable | Values | Units |
|---|---|---|
| ISR rate (TIM2) | 0, 500, 1000, 2000, 5000, 10000 | Hz |

### 3.4 Dependent Variables

List exactly what you are measuring:

| Metric | Measurement Method | Resolution |
|---|---|---|
| KWS inference latency (p99) | DWT cycle counter | 2 ns |
| PID deadline miss rate | FreeRTOS task stats | % |
| Energy per inference | INA219 I²C | 1 mW |

### 3.5 Controlled Variables

List what is held constant:

- FreeRTOS scheduler: Fixed-Priority Preemptive
- Task priorities: as per `configs/tasks/`
- Board: NUCLEO-H743ZI2 only
- Firmware version: commit SHA X
- Ambient temperature: room temperature (~25°C)
- Sample count: 1000 consecutive inferences per ISR rate

### 3.6 Expected Outcome

What do you predict will happen and why?

### 3.7 Actual Outcome

*(Filled in after running)*

### 3.8 Interpretation

*(Filled in after analysis)*

---

## 4. Writing `config.yaml`

The experiment config overrides the base configuration in `configs/`:

```yaml
# experiments/EXP-003_interrupt_sweep/config.yaml
# Inherits defaults from configs/scheduler/fp_default.yaml and configs/tasks/

experiment:
  id: "EXP-003"
  name: "interrupt_sweep_fp"
  description: "ISR rate sweep under FP scheduling"

# Override only what changes for this experiment
isr_load:
  source: "TIM2"
  rates_hz: [0, 500, 1000, 2000, 5000, 10000]

benchmark:
  samples_per_rate: 1000
  warmup_samples: 100
```

---

## 5. Writing `run.sh`

The run script must be fully automated — no manual steps after starting:

```bash
#!/usr/bin/env bash
# EXP-NNN run script
# Usage: bash run.sh [--board nucleo_h743zi2]
set -euo pipefail

BOARD="${1:-nucleo_h743zi2}"
EXP_ID="EXP-NNN"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
RAW_DIR="../../results/${EXP_ID}/raw/${TIMESTAMP}"

mkdir -p "$RAW_DIR"

# Step 1: Build firmware with experiment config
cmake -S ../../firmware -B ../../build/${EXP_ID} \
      -DCMAKE_TOOLCHAIN_FILE=../../tools/cmake/arm-none-eabi.cmake \
      -DBOARD=${BOARD} \
      -DCONFIG_FILE="$(pwd)/config.yaml"
cmake --build ../../build/${EXP_ID} --parallel

# Step 2: Flash
python ../../scripts/build/flash.py \
       --board ${BOARD} \
       --elf ../../build/${EXP_ID}/schedtiny.elf

# Step 3: Capture UART output
python ../../scripts/build/capture_uart.py \
       --duration 120 \
       --output "${RAW_DIR}/run_001.csv"

echo "Raw results saved to ${RAW_DIR}"
echo "Run analysis.ipynb to generate figures."
```

---

## 6. Running the Analysis Notebook

The notebook must run end-to-end without manual intervention:

```bash
jupyter nbconvert --to notebook --execute analysis.ipynb \
        --ExecutePreprocessor.timeout=300
```

All figures are saved to `results/EXP-NNN/figures/` automatically.

---

## 7. Statistical Requirements

| Requirement | Minimum Standard |
|---|---|
| Sample count | ≥ 1000 per condition |
| Repeated runs | ≥ 3 independent runs per condition |
| Reported statistics | Mean, std, p95, p99 minimum |
| Outlier handling | Document and justify if any samples are removed |
| Confidence intervals | Report 95% CI for all comparative claims |

**Do not report only means.** Latency distributions on embedded systems are
often non-Gaussian (bimodal, heavy-tailed). Always plot the CDF and the boxplot.

---

## 8. Known Pitfalls (from Literature)

Based on `references/research.md` Section I (Mistakes Made by Previous Researchers):

1. **Ignoring DWT overflow** — DWT wraps at 2³² cycles. At 480 MHz, this is ~8.9
   seconds. Your measurement window must be shorter or you must handle overflow.
   See `firmware/src/bench/bench_measure.c` for the overflow-safe implementation.

2. **Compiler optimization artifacts** — Measure with the same optimization level
   as the final binary. Use `Release` mode (`-O2`) for all benchmarks.
   Never report results from `Debug` mode (`-O0`) as representative.

3. **UART blocking** — If UART TX is blocking, it adds latency that is not part
   of the measured quantity. All UART output uses DMA. Verify this.

4. **Temperature effects** — STM32 clock accuracy can vary with temperature.
   Run experiments at stable room temperature and note the ambient temperature
   in the experiment README.

5. **First-run cache effects** — The M7 has a 16 KB I-cache. The first inference
   is always slower. Use warmup samples (100 default) and discard them.

6. **Single-run reporting** — Never report results from a single run. Run at
   least 3 times and check that results are within ±5%.

7. **Ignoring FreeRTOS tick interrupt** — The FreeRTOS tick timer (default 1 ms)
   is itself an ISR that causes jitter. Always measure with the FreeRTOS tick running
   unless specifically studying the bare-metal baseline.

---

## 9. Figure Standards for Journal Submission

All figures must meet these standards before inclusion in the paper:

- **Format:** PDF or SVG (vector). Never PNG for line plots.
- **Font size:** Minimum 10pt for axis labels, 8pt for tick labels
- **Color:** Use a colorblind-safe palette (tab10 is acceptable; avoid pure red/green)
- **Line width:** Minimum 1.5pt
- **Caption:** Self-contained — a reader must understand the figure without reading the body
- **Size:** Single-column width: 3.5 inches; double-column: 7.16 inches
- **DPI:** 300 minimum for raster exports (if needed for supplementary material)

See `scripts/analysis/plot_figures.py` for the standard matplotlib style sheet.
