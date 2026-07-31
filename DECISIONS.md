# DECISIONS.md

This document serves as the formal engineering decision log for the SchedTiny framework.
All major architectural, hardware, and software design decisions must be recorded here.

## Template

```markdown
### D-XXX: [Short Title]
- **Date:** YYYY-MM-DD
- **Status:** [Proposed | Accepted | Rejected | Superseded]
- **Context:** [What is the problem or choice being made?]
- **Options Considered:**
  1. [Option 1]
  2. [Option 2]
- **Decision:** [What was chosen]
- **Rationale:** [Why it was chosen. Cite literature if applicable.]
- **Consequences:** [Impact on the rest of the system]
```

---

## Active Decisions

### D-001: STM32 NUCLEO-H743ZI2 as Primary Board
- **Date:** 2026-07-31
- **Status:** Accepted
- **Context:** Multi-board support creates significant maintenance overhead before V1 firmware is validated.
- **Options Considered:**
  1. Support F767, L496, and H743 immediately.
  2. Target H743ZI2 exclusively for V1.
- **Decision:** Target H743ZI2 exclusively.
- **Rationale:** Minimizes scope. Ensures CI and tests are stable before addressing cross-board portability (GAP-8).
- **Consequences:** F767 and L496 board files are archived until Milestone 3.

### D-002: core/ vs research/ Firmware Separation
- **Date:** 2026-07-31
- **Status:** Accepted
- **Context:** Experimental benchmark tasks (PID, ML) shouldn't be mixed with the stable scheduler framework.
- **Options Considered:**
  1. Flat `firmware/src/` structure.
  2. Separating `core/` (framework) from `research/tasks/` (experiments).
- **Decision:** Separated structure.
- **Rationale:** Clarifies the research contribution (the framework) vs. the evaluation workload (the tasks).
- **Consequences:** Build system and CMake paths must reflect this strict separation.
