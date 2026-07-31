# SchedTiny — Ideas (Unvalidated)

This document captures research ideas that are **not yet validated** and have
**no implementation commitment**. Ideas are promoted to `docs/SPEC.md` only
after literature review confirms they are novel and feasible.

See `docs/RESEARCH_GUIDE.md` Step 3 for the classification process.

---

## IDEA-001: TFLM Micro-Scheduler Yield Hook

**Idea:** Patch TFLM's `Invoke()` method to check for a pending higher-priority
FreeRTOS task between operator kernel calls. If a high-priority task is pending,
yield the CPU via `taskYIELD()` and resume inference in the next activation.

**Motivation:** From `research.md` Section D, Contribution D.6.

**Status:** 🔄 Under investigation (Q-002 in `open_questions.md`)

**Risk:** TFLM may not maintain deterministic state between operator calls.
Need to verify that resuming inference after a yield produces correct results.

---

## IDEA-002: Schedulability Utilization Dashboard

**Idea:** Python script that reads `configs/*.yaml` task parameters and computes
the RM schedulability test (U ≤ n(2^(1/n) − 1)) and RTA for the configured
task set. Display as a terminal dashboard before each experiment.

**Motivation:** Researchers should know before running an experiment whether the
task set is theoretically schedulable.

**Status:** 🔄 Simple to implement; no blocking questions

---

## IDEA-003: Cross-Board Retargeting Script

**Idea:** Automated script that runs the same experiment on all three boards
sequentially and generates a comparison table (H743ZI2 vs. F767ZI vs. L496ZG).

**Motivation:** From `research.md` Section D, Contribution D.7 (MLonMCU-inspired).

**Status:** 🔄 Depends on all three boards being wired and functional

---

## IDEA-004: jitter-bounded infer() API for Applications

**Idea:** Expose `sched_infer_with_budget(uint32_t budget_us)` as a library API
that application developers can call without knowing FreeRTOS internals.

**Motivation:** From `research.md` Section D, Contribution D.8.

**Status:** ✅ Accepted — moved to `docs/SPEC.md` REQ-SCHED-002

---

## Promotion Process

To promote an idea to `docs/SPEC.md`:
1. Validate novelty: check `references/literature_matrix.md` — is this genuinely novel?
2. Validate feasibility: check `references/open_questions.md` — is this blocked?
3. Open a GitHub feature request issue
4. After maintainer review, add as REQ-* to `docs/SPEC.md`
