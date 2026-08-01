# SchedTiny: Developer & Contributor Deep Dive Guide

This developer guide provides an architectural and implementation deep dive into SchedTiny's internal components, scheduler design patterns, measurement engines, and TinyML pipelines.

---

## 1. Architectural Principles & Layering

SchedTiny follows a strict layered separation of concerns:

```
firmware/
├── core/
│   ├── scheduler/      # Core dispatching, ready queue, HPF/EDF/RMS/MC policies
│   ├── bench/          # DWT cycle measurement, ring buffer, UART-DMA logging
│   └── drivers/        # MCU peripheral drivers (GPIO, Timer, UART)
├── include/            # Clean, decoupled public C API headers
└── tests/              # CMocka unit testing suite
```

### Layering Constraints
1. **Zero Dynamic Memory Allocation:** All task descriptors, queues, event ring buffers, and decision models are statically pre-allocated at compile time.
2. **Fixed-Width Integer Arithmetic:** All feature engineering, time conversions, and threshold checks use Basis Points ($0 - 10000$ representing $0.00\% - 100.00\%$) to guarantee zero-FPU overhead on Cortex-M0/M3/M4 cores.
3. **Interrupt Safety:** Measurement hooks (`bench_measure`) and ring buffers operate with atomic critical sections or lockless single-producer single-consumer (SPSC) semantics.

---

## 2. Scheduler Internals & Policies

The core dispatcher (`sched_dispatcher.c`) evaluates the highest-priority eligible task from the Ready Queue (`sched_ready_queue.c`) upon each tick or interrupt event.

### 2.1 High Priority First (HPF)
Static priority policy ordering tasks monotonically by numerical priority ($0-255$, where $0$ is highest).

### 2.2 Earliest Deadline First (EDF)
Dynamic deadline-driven policy. Upon task admission or arrival, absolute deadline is calculated as:
$$d_i(k) = r_i(k) + D_i$$
Tasks are sorted in the ready queue by lowest $d_i$. Ties are resolved by static task ID.

### 2.3 Rate Monotonic Scheduling (RMS)
Static optimal policy for periodic task sets where priority is assigned strictly inversely proportional to period:
$$T_i < T_j \implies \text{Priority}(\tau_i) > \text{Priority}(\tau_j)$$

### 2.4 Mixed-Criticality (Vestal Model)
Implements dual-criticality mode switching:
- **Normal Mode (LO-Mode):** All tasks execute up to $C_i(\text{LO})$.
- **Overrun Detection:** If a HI-criticality task exceeds $C_i(\text{LO})$, the scheduler immediately transitions to **HI-Mode**, suppressing or shedding LO-criticality tasks to ensure zero deadline misses for HI-tasks.
- **Hysteresis Recovery:** When HI-tasks return to nominal execution for $K$ consecutive cycles, LO-tasks are safely restored.

---

## 3. Adaptive Scheduler & TinyML Pipeline

The Adaptive Scheduler acts as an orchestrator above individual policies. Every evaluation window ($N$ ticks), it extracts a 16-dimensional feature vector:

$$\vec{f} = \langle U_{\text{cpu}}, N_{\text{tasks}}, L_{\text{queue}}, R_{\text{avg}}, W_{\text{avg}}, M_{\text{deadline}}, C_{\text{switch}}, T_{\text{idle}}, T_{\text{busy}}, F_{\text{rate}}, S_{\text{recovery}}, E_{\mu\text{J}}, P_{\mu\text{W}}, N_{\text{HI}}, \rho_{\text{HI}}, f_{\text{mode}} \rangle$$

### Embedded Decision Tree Realization
The offline-trained Scikit-Learn tree is compiled by `scripts/export_decision_tree.py` into a nested static C structure (`sched_adaptive_model.h`). It requires:
- **Flash Footprint:** $<450\text{ bytes}$
- **RAM Footprint:** $0\text{ bytes}$
- **Execution Overhead:** $216\text{ cycles}$ ($0.45\,\mu\text{s}$ at 480 MHz)

---

## 4. Non-Intrusive DWT Trace & Measurement Subsystem

Sub-microsecond measurement is achieved via the ARM Cortex-M Data Watchpoint and Trace (DWT) cycle counter register (`DWT_CYCCNT`):

```c
static inline uint32_t bench_dwt_now(void) {
    return DWT->CYCCNT;
}
```

- **Overhead:** 2 CPU cycles per read.
- **Asynchronous Flush:** Measured deltas are pushed to an in-memory ring buffer (`bench_ring_buffer.c`) and flushed over UART-DMA from a low-priority background worker, guaranteeing zero measurement perturbation.

---

## 5. Hardware Abstraction Layer (`bench_hal.h`)

The hardware abstraction layer dynamically configures clock frequencies and hardware metadata via target preprocessor macros:

```c
#if defined(STM32H7)
    #define BENCH_CORE_CLOCK_MHZ   480U
    #define BENCH_MCU_VARIANT      "STM32H743ZI2"
#elif defined(STM32F7)
    #define BENCH_CORE_CLOCK_MHZ   216U
    #define BENCH_MCU_VARIANT      "STM32F767ZI"
#elif defined(STM32F4)
    #define BENCH_CORE_CLOCK_MHZ   168U
    #define BENCH_MCU_VARIANT      "STM32F407VG"
#else
    #define BENCH_CORE_CLOCK_MHZ   100U
    #define BENCH_MCU_VARIANT      "Generic-Sim"
#endif
```

---

## 6. Coding & Contribution Rules

- **C Standards:** ISO C11, MISRA-C aligned guidelines, zero warnings with `-Wall -Wextra -Werror`.
- **Python Standards:** PEP 8, formatted with `black`, checked with `flake8 --max-line-length=100 --ignore=E203,W503`, strictly typed with `mypy`.
- **Commit Messages:** Follow Conventional Commits format (`feat(...)`, `fix(...)`, `docs(...)`).
