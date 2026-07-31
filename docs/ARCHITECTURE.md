# SchedTiny System Architecture

**Version:** 0.1.0-draft  
**Status:** Draft  
**Owner:** Lead Researcher / Lead Software Engineer  
**Last Updated:** 2026-07-31

---

## 1. Overview

SchedTiny is organized as four independent, layered pipelines that flow into each other:

```
┌─────────────────────────────────────────────────────────────────┐
│  PIPELINE 1: Research                                           │
│  references/research.md → docs/SPEC.md → GitHub Issues         │
└───────────────────────────────┬─────────────────────────────────┘
                                │ requirements drive
┌───────────────────────────────▼─────────────────────────────────┐
│  PIPELINE 2: Implementation                                     │
│  configs/*.yaml → firmware/src/ → firmware/boards/             │
└───────────────────────────────┬─────────────────────────────────┘
                                │ firmware produces
┌───────────────────────────────▼─────────────────────────────────┐
│  PIPELINE 3: Experiments                                        │
│  experiments/EXP-NNN/ → results/EXP-NNN/raw/ → processed/      │
└───────────────────────────────┬─────────────────────────────────┘
                                │ figures flow to
┌───────────────────────────────▼─────────────────────────────────┐
│  PIPELINE 4: Publication                                        │
│  results/*/figures/ → papers/journal_v1/                        │
└─────────────────────────────────────────────────────────────────┘
```

---

## 2. Firmware Architecture

### 2.1 Component Diagram

```
firmware/
│
├── core/            ← THE RESEARCH FRAMEWORK (Stable)
│   ├── scheduler/     Core OS, ISR hooks, scheduling policies
│   ├── bench/         Measurement layer (DWT timing, JSON logging)
│   ├── drivers/       Hardware abstraction (was hal/)
│   └── ml/            TFLM/CMSIS-NN abstraction
│
├── research/        ← BENCHMARK WORKLOADS (Experimental)
│   └── tasks/         PID control, MPU6050, Inference task wrappers
│
├── boards/          ← BOARD-SPECIFIC CONFIGURATION
│   └── nucleo_h743zi2/ (V1 single board)
│
└── main.c           ← Application entry point
```

### 2.2 Layering Rules

These rules enforce separation of concerns and board independence:

| Rule | Description |
|---|---|
| **Rule 1** | `drivers/` MUST NOT include `scheduler/` or `tasks/`. It knows only hardware. |
| **Rule 2** | `scheduler/` MUST NOT include model-specific headers. It calls `model_runner.h` only. |
| **Rule 3** | `tasks/` MUST NOT include `bench_measure.h` directly. Measurement is injected by the scheduler via hooks. |
| **Rule 4** | All board-specific register addresses live in `firmware/boards/<name>/`. No `#ifdef BOARD_X` in `firmware/core/`. |
| **Rule 5** | `bench_log.c` is the ONLY component that writes to UART. No `printf()` elsewhere. |

### 2.3 Measurement Architecture

The measurement layer is unique in SchedTiny: it is **not an afterthought**. It runs
inside the scheduler hooks with DWT cycle counter reads (no interrupt, no overhead)
and logs asynchronously via UART-DMA.

```
ISR Entry
    │
    ▼
sched_isr_entry_hook(irq_source)
    │  DWT read → delta stored in ring buffer
    ▼
[ISR body executes]
    │
    ▼
sched_isr_exit_hook(irq_source)
    │  DWT read → jitter computed, written to ring buffer
    ▼
bench_log_flush() [called from low-priority idle task]
    │  Serializes ring buffer entries to JSON
    ▼
UART DMA TX → Host PC → parse_uart_log.py
```

### 2.4 Configuration Flow

```
configs/scheduler/fp_default.yaml
        │
        ▼
cmake/generate_config.py (runs at configure time)
        │
        ▼
firmware/include/schedtiny_config.h (generated, gitignored)
        │
        ▼
All firmware components include schedtiny_config.h
```

---

## 3. Experiment Architecture

Every experiment is self-contained and hypothesis-driven:

```
experiments/
├── baseline/          ← Bare-metal + RTOS overhead baselines (no ISR load)
├── interrupt/         ← ISR rate sweep experiments
├── scheduler/         ← Policy comparison (FPP vs RM vs EDF)
└── energy/            ← Power measurement experiments
```

**Results structure** (mirrors experiment categories):

```
results/interrupt/
├── raw/               ← UART log files (.csv, gitignored)
│   ├── run_001.csv
│   └── run_002.csv    ← Multiple runs for statistical confidence
├── processed/         ← Computed metrics (.json, committed)
│   ├── latency_stats.json
│   └── jitter_stats.json
└── figures/           ← Journal-quality plots (PDF/SVG, committed)
    ├── fig_latency_cdf.pdf
    └── fig_jitter_boxplot.pdf
```

---

## 4. Data Flow Diagram

```
STM32 UART (JSON log)
        │
        ▼
scripts/analysis/parse_uart_log.py
        │  produces
        ▼
results/*/raw/*.csv  (gitignored)
        │
        ▼
scripts/analysis/compute_metrics.py
        │  produces
        ▼
results/*/processed/*.json  (committed)
        │
        ▼
scripts/analysis/plot_figures.py
        │  produces
        ▼
results/*/figures/*.pdf  (committed)
        │
        ▼ (copy/symlink)
papers/journal_v1/figures/
        │
        ▼
IEEE Journal Submission
```

---

## 5. Board Support Architecture

```
firmware/
├── core/
├── research/
└── boards/
    └── nucleo_h743zi2/
        ├── board_config.h   ← Clock, pin assignments, IRQ mappings
        ├── linker.ld        ← Memory layout
        └── CMakeLists.txt   ← Board-specific compiler flags, HAL sources
```

Board selection at configure time: `-DBOARD=nucleo_h743zi2`

---

## 6. Third-Party Dependency Policy

All third-party code is **vendored** (copied into `firmware/third_party/`) with exact
commit SHAs pinned in `firmware/third_party/README.md`.

**Rationale:** Reproducibility across years. A paper published in 2026 must be
reproducible in 2030. Submodules or network downloads cannot guarantee this.

| Library | License | Pinned In |
|---|---|---|
| FreeRTOS 10.6.x | MIT | `firmware/third_party/README.md` |
| TensorFlow Lite Micro | Apache 2.0 | `firmware/third_party/README.md` |
| CMSIS-NN | Apache 2.0 | `firmware/third_party/README.md` |

---

## 7. Key Design Decisions and Rationale

| Decision | Rationale |
|---|---|
| DWT cycle counter for timing | Zero interrupt overhead; 1-cycle resolution at 480 MHz = 2 ns. No OS timer needed. |
| JSON over UART for logging | Human-readable; directly parseable by Python without custom protocol. |
| YAML config → C header codegen | Eliminates magic numbers; makes every experiment parameter explicit and versioned. |
| Vendored third-party | Multi-year reproducibility. Critical for journal artifacts. |
| Board HAL layer | Three-board support without `#ifdef` soup in research code. |
| Experiment-first design | Hypothesis written before code. Prevents HARKing (Hypothesizing After Results are Known). |
