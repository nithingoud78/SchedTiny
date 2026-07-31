# C Coding Standard

## SchedTiny Embedded C Style and Quality Rules

**Applies to:** All C source files in `firmware/`  
**Enforced by:** `clang-format` + CI lint check  
**Standard:** C11 (ISO/IEC 9899:2011)

---

## 1. Guiding Principles

1. **Clarity over cleverness.** This code must be maintained by researchers who
   may not be embedded experts. Choose the obvious implementation.
2. **Explicit over implicit.** No hidden control flow, no magic numbers, no
   reliance on undefined behavior.
3. **Measurability.** The code must be instrumentable. Every critical path must
   have a measurement hook point.
4. **Determinism.** Real-time code must be analyzable. Avoid dynamic allocation,
   recursion, and unbounded loops in task code.

---

## 2. File Organization

### 2.1 File Header

Every `.c` and `.h` file must begin with the Doxygen file header block
(see `docs/DEVELOPMENT_GUIDE.md` for the template). No exceptions.

### 2.2 Include Guard

Every `.h` file uses a `#ifndef` include guard with the format:

```c
#ifndef SCHEDTINY_MODULE_NAME_H
#define SCHEDTINY_MODULE_NAME_H
/* ... */
#endif /* SCHEDTINY_MODULE_NAME_H */
```

Never use `#pragma once` (not standard C).

### 2.3 Include Order (enforced by clang-format)

```c
/* 1. This file's own header */
#include "sched_isr.h"

/* 2. Other SchedTiny headers (alphabetical within group) */
#include "bench_measure.h"
#include "hal_timer.h"

/* 3. FreeRTOS headers */
#include "FreeRTOS.h"
#include "task.h"

/* 4. CMSIS/STM32 HAL headers */
#include "stm32h7xx_hal.h"

/* 5. C standard library (last) */
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
```

---

## 3. Naming Conventions

| Category | Convention | Example |
|---|---|---|
| Files | `snake_case` | `sched_isr.c`, `hal_timer.h` |
| Functions | `module_verb_noun()` | `sched_isr_register_hook()` |
| Types (typedef struct/enum) | `PascalCase_t` | `SchedPolicy_t`, `BenchEvent_t` |
| Constants (`#define`) | `UPPER_SNAKE_CASE` | `SCHED_MAX_TASKS`, `DWT_CYCLES_PER_US` |
| Global variables | `g_` prefix + `snake_case` | `g_sched_task_count` |
| Static (file-scope) variables | `s_` prefix | `s_isr_entry_time` |
| Local variables | `snake_case` | `latency_us`, `task_id` |
| Enum values | `MODULE_VALUE` | `SCHED_POLICY_EDF` |
| FreeRTOS handles | `x` prefix (FreeRTOS convention) | `xTaskHandle`, `xQueueHandle` |

---

## 4. Types

- Always use fixed-width types from `<stdint.h>`: `uint32_t`, `int32_t`, etc.
- Never use `int`, `long`, `unsigned` alone. Their width is platform-defined.
- Use `bool` from `<stdbool.h>` for boolean values. Never use `int` as a boolean.
- Size types: use `size_t` for object sizes; `uint32_t` for DWT cycle counts.

---

## 5. Functions

- **Single responsibility:** One function does one thing.
- **Maximum length:** 60 lines. If longer, split into sub-functions.
- **Parameters:** Maximum 5. If more are needed, use a configuration struct.
- **Return values:** All functions return a status code (`ModuleStatus_t`) unless
  the return value is the primary output and errors cannot occur (e.g., a getter).
- **Error handling:** Check all return values. Never silently ignore errors.

```c
/* WRONG */
xTaskCreate(task_body, "Task", 512, NULL, 1, NULL);

/* CORRECT */
BaseType_t result = xTaskCreate(task_body, "Task", 512, NULL, 1, &handle);
configASSERT(result == pdPASS);
```

---

## 6. Memory

- **No dynamic allocation in task code.** No `malloc()`, no `pvPortMalloc()` in
  any function called from a FreeRTOS task body after scheduler start.
- Static allocation is preferred for all long-lived objects.
- Stack usage: each task's stack depth is calculated and documented in its header.
- Check stack high-water mark in unit tests: `uxTaskGetStackHighWaterMark()`.

---

## 7. Real-Time Rules

These rules apply to all code that runs in FreeRTOS task or ISR context:

| Rule | Rationale |
|---|---|
| No `printf()` | Blocks on UART TX; use `bench_log_event()` |
| No `malloc()` / `free()` | Non-deterministic; causes heap fragmentation |
| No recursion | Stack overflow risk; non-analyzable WCET |
| No unbounded loops | Prevents WCET analysis |
| No floating point in ISRs | FPU context save/restore overhead (unless FPU context is explicitly managed) |
| `configASSERT()` for all critical invariants | Debug traps; stripped in release via `NDEBUG` |

---

## 8. Comments

- Public API functions: Doxygen `/** ... */` block (see template in `DEVELOPMENT_GUIDE.md`)
- Non-obvious logic: inline comment explaining WHY, not what
- Research context: cite the paper key from `references/citations.bib`
- No commented-out code: use `git stash` or a branch instead

```c
/* WRONG */
// uint32_t old_latency = measure_old(); // TODO: remove

/* CORRECT */
/*
 * We use DWT instead of SysTick here because SysTick is already used by
 * FreeRTOS and reusing it would require modifying the FreeRTOS port.
 * See [immonen2022tiny] for a discussion of timing overhead on STM32L4.
 */
uint32_t cycles = DWT->CYCCNT;
```

---

## 9. Formatting (enforced by clang-format)

The `.clang-format` file in `tools/clang-format/` defines all formatting rules.
Key settings:
- Indent: 4 spaces (no tabs)
- Brace style: Allman (opening brace on new line)
- Line length: 100 characters
- Pointer alignment: right (`uint32_t *ptr`)

Run before every commit:
```bash
clang-format -i -style=file:tools/clang-format/.clang-format firmware/src/**/*.c firmware/src/**/*.h
```

---

## 10. Static Analysis

Before merging any firmware PR:

1. Run `clang-format --dry-run --Werror` — no formatting violations
2. Build with `-Wall -Wextra -Werror` — no warnings
3. Run unit tests with AddressSanitizer (host build): `cmake -DCMAKE_C_FLAGS="-fsanitize=address"`

The CI pipeline enforces all three automatically.
