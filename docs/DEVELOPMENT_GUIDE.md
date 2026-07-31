# Development Guide

## Firmware Development Workflow for SchedTiny

---

## 1. Prerequisites

Before writing any firmware, ensure you have read:
- `docs/ARCHITECTURE.md` — system design and layering rules
- `docs/SPEC.md` — what the system must do
- `docs/CODING_STANDARD.md` — C coding rules
- `docs/BUILD_GUIDE.md` — toolchain setup

---

## 2. Codebase Orientation

```
firmware/src/
├── bench/         Measurement layer — DWT timing, JSON log output
├── scheduler/     Core research: interrupt-aware scheduler
├── ml/            TinyML integration: TFLM/CMSIS-NN abstraction
├── tasks/         Benchmark tasks: PID, sensor, inference
├── hal/           Hardware abstraction: timer, UART, GPIO, power
└── main.c         Entry point: initializes hardware, creates tasks
```

**Where to add new code:**

| What you're adding | Where it goes |
|---|---|
| New scheduling policy | `firmware/src/scheduler/sched_policy.c/h` |
| New ISR hook | `firmware/src/scheduler/sched_isr.c/h` |
| New measurement metric | `firmware/src/bench/bench_measure.c/h` + update `scripts/analysis/` |
| New TinyML model | `firmware/src/ml/model_<name>.c/h` + `models/<name>/` |
| New benchmark task | `firmware/src/tasks/task_<name>.c/h` |
| New board support | `firmware/boards/<board_name>/` |
| New peripheral driver | `firmware/src/hal/hal_<name>.c/h` |

---

## 3. Adding a New Firmware Component

### 3.1 Create the source and header files

Every component has exactly one `.c` and one `.h` file (with exceptions for large components).

Header template:
```c
/**
 * @file    component_name.h
 * @brief   One-sentence description of this component.
 *
 * Detailed description. Explain the design rationale.
 * Reference research gap or SPEC requirement it satisfies.
 *
 * @see     docs/SPEC.md REQ-SCHED-001
 * @see     [authorYYYYkeyword] in references/citations.bib
 *
 * @author  @nithingoud78
 * @date    YYYY-MM-DD
 * @version 0.1.0
 *
 * @copyright Copyright (c) 2026 SchedTiny Contributors
 *            Apache License, Version 2.0
 */

#ifndef SCHEDTINY_COMPONENT_NAME_H
#define SCHEDTINY_COMPONENT_NAME_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * Public Types
 * ========================================================================= */

/**
 * @brief   Status codes returned by this component.
 */
typedef enum {
    COMPONENT_OK    = 0,  /**< Operation succeeded */
    COMPONENT_ERROR = 1,  /**< General error */
    COMPONENT_BUSY  = 2,  /**< Resource busy */
} ComponentStatus_t;

/* =========================================================================
 * Public API
 * ========================================================================= */

/**
 * @brief   Initialize the component.
 * @return  COMPONENT_OK on success, error code otherwise.
 *
 * Must be called once before any other function in this module.
 * Thread-safe: No (call before FreeRTOS scheduler starts).
 */
ComponentStatus_t component_init(void);

#ifdef __cplusplus
}
#endif

#endif /* SCHEDTINY_COMPONENT_NAME_H */
```

### 3.2 Register in CMakeLists.txt

Add the source file to `firmware/CMakeLists.txt`:
```cmake
target_sources(schedtiny PRIVATE
    src/your_component/component_name.c
)
```

### 3.3 Write a unit test

Add a test in `firmware/tests/test_component_name.c`. See existing tests for the
CMocka pattern. Every new component must have at least one unit test before merging.

---

## 4. The Measurement Discipline

**Rule:** Never add `printf()` for debugging. Use the bench log system.

```c
/* WRONG — blocks UART, corrupts measurements */
printf("Task started\n");

/* CORRECT — non-blocking DMA log, parsed by parse_uart_log.py */
bench_log_event(BENCH_EVENT_TASK_START, TASK_ID_INFER, dwt_cycles_now());
```

Measurement points must be defined in `firmware/src/bench/bench_config.h` and
documented in `docs/EXPERIMENT_GUIDE.md`.

---

## 5. FreeRTOS Task Creation Pattern

All tasks follow this pattern:

```c
/* Task handle — declared in task_pid.h */
TaskHandle_t g_task_pid_handle = NULL;

/* Stack size — defined in schedtiny_config.h (from configs/*.yaml) */
#define TASK_PID_STACK_DEPTH   (configMINIMAL_STACK_SIZE * 4)

/* Task function */
static void task_pid_body(void *pvParameters)
{
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xPeriod = pdMS_TO_TICKS(TASK_PID_PERIOD_MS);

    for (;;) {
        bench_log_event(BENCH_EVENT_TASK_START, TASK_ID_PID, dwt_now());

        /* === Task body === */
        pid_compute();
        /* ================ */

        bench_log_event(BENCH_EVENT_TASK_END, TASK_ID_PID, dwt_now());

        vTaskDelayUntil(&xLastWakeTime, xPeriod);
    }
}

/* Task creation — called from main.c */
void task_pid_create(void)
{
    BaseType_t result = xTaskCreate(
        task_pid_body,
        "PID",
        TASK_PID_STACK_DEPTH,
        NULL,
        TASK_PID_PRIORITY,   /* from schedtiny_config.h */
        &g_task_pid_handle
    );
    configASSERT(result == pdPASS);
}
```

---

## 6. Git Workflow

```bash
# 1. Create a feature branch from dev
git checkout dev
git pull upstream dev
git checkout -b feat/scheduler-isr-hooks

# 2. Write code following CODING_STANDARD.md

# 3. Format
clang-format -i -style=file:tools/clang-format/.clang-format \
             firmware/src/scheduler/sched_isr.c \
             firmware/src/scheduler/sched_isr.h

# 4. Build check (all boards)
cmake -S firmware -B build/h743 -DBOARD=nucleo_h743zi2 && cmake --build build/h743
cmake -S firmware -B build/f767 -DBOARD=nucleo_f767zi  && cmake --build build/f767
cmake -S firmware -B build/l496 -DBOARD=nucleo_l496zg  && cmake --build build/l496

# 5. Run unit tests
cmake -S firmware/tests -B build/tests && cmake --build build/tests
cd build/tests && ctest --output-on-failure

# 6. Commit using Conventional Commits
git add -p
git commit -m "feat(scheduler): add ISR entry/exit hook API"

# 7. Push and open PR against dev
git push origin feat/scheduler-isr-hooks
```

---

## 7. Common Pitfalls

| Pitfall | Prevention |
|---|---|
| Magic numbers in source | All numbers come from `schedtiny_config.h` (generated from YAML) |
| Board-specific code in `src/` | Board-specific code ONLY in `firmware/boards/<name>/` |
| Blocking UART in measurement path | Use `bench_log.c` DMA path exclusively |
| DWT overflow not handled | Use `dwt_elapsed_cycles_safe()` which handles 32-bit wrap |
| Forgetting to update CHANGELOG | Every PR must have a `CHANGELOG.md` entry |
| Testing only on Debug builds | Always benchmark with `CMAKE_BUILD_TYPE=Release` |
