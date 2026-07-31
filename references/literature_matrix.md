# Literature Gap Analysis Matrix

This matrix maps each surveyed paper (from `research.md`) to the 8 research gaps
defined in `docs/SPEC.md`. It is the primary tool for writing the Related Work section.

**Legend:**
- ✅ Addressed — paper directly and substantially addresses this gap
- ⚠️ Partial — paper partially addresses or mentions this gap
- ❌ Not addressed — gap is not covered

| # | Paper (Short) | GAP-1 No mixed-criticality benchmark | GAP-2 Scheduling-aware latency model | GAP-3 Interrupt jitter quantified | GAP-4 RTOS-integrated TinyML hooks | GAP-5 Energy-aware scheduling | GAP-6 Schedulability analysis for ML | GAP-7 Reproducible task models | GAP-8 Cross-MCU generalization |
|---|---|---|---|---|---|---|---|---|---|
| 1 | Experimental Eval Scheduling Effects TinyML (IEEE 2024) | ⚠️ | ⚠️ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 2 | Real-Time Benchmarking TinyML (IEEE 2024) | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ⚠️ |
| 3 | Benchmarking TinyML Systems (Banbury 2020) | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ⚠️ | ⚠️ |
| 4 | MLPerf Tiny (MLCommons 2021–2026) | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ✅ | ✅ |
| 5 | TinyMLOps (ACM 2023) | ❌ | ❌ | ❌ | ⚠️ | ⚠️ | ❌ | ❌ | ❌ |
| 6 | MLonMCU (ACM 2023) | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ | ✅ |
| 7 | TinyML Progress & Futures Survey (arXiv 2024) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ |
| 8 | TinyML Small Models Big Impact (Springer 2026) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 9 | Ultra-Low Power TinyML Visual (IEEE TCAS-II 2023) | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| 10 | TinyML Energy-Efficient Inference (IEEE 2024) | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| 11 | TinyML Frugal Smart Objects (IEEE CAS Mag 2020) | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ⚠️ |
| 12 | TinyML Resource-Constrained MCUs (Sensors 2022) | ❌ | ❌ | ❌ | ⚠️ | ⚠️ | ❌ | ❌ | ❌ |
| 13 | Energy-Efficient TinyML UAVs (Drones 2021) | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| 14 | SPARK Sparsity-Aware Scheduling (ACM 2024) | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| 15 | STM32 FreeRTOS Complex Scheduling (IEEE 2024) | ❌ | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ⚠️ | ❌ |
| 16 | Scheduling Energy FreeRTOS (Springer 2023) | ❌ | ⚠️ | ❌ | ❌ | ✅ | ⚠️ | ⚠️ | ❌ |
| 17 | TinyML Tools Survey (arXiv 2023) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ |
| 18 | Predicting TinyML Latency (IEEE 2024) | ❌ | ⚠️ | ❌ | ❌ | ❌ | ❌ | ❌ | ✅ |
| 19 | Edge AI Survey & Framework (MDPI 2025) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ |
| 20 | Review TinyML State-of-Art (Elsevier 2023) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 21 | TinyML Resource-Constrained (Springer 2026) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ |
| 22 | On-Device Personalization HAR STM32 (IEEE ESL 2023) | ❌ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ❌ | ❌ |
| 23 | DNN Not All You Need (arXiv 2022) | ❌ | ❌ | ❌ | ❌ | ✅ | ❌ | ❌ | ❌ |
| 24 | Edge AI Landscape (Elsevier 2026) | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ❌ | ⚠️ |
| 25 | ST MLPerf Tiny v1.3 Results (ST 2025) | ⚠️ | ❌ | ❌ | ❌ | ⚠️ | ❌ | ✅ | ❌ |
| **SchedTiny** | **This work** | **✅** | **✅** | **✅** | **✅** | **✅** | **✅** | **✅** | **✅** |

---

## Gap Summary

| Gap | Papers Addressing | SchedTiny Novelty |
|---|---|---|
| GAP-1: No mixed-criticality benchmark | Papers 1, 2, 3, 4, 6 (partial only) | First open benchmark combining PID + TinyML |
| GAP-2: Scheduling-aware latency model | Papers 1, 15, 16, 18 (partial) | Interrupt-aware WCET with measured ISR delta |
| GAP-3: Interrupt jitter quantified | Paper 1 (partial) | Systematic per-source jitter measurement API |
| GAP-4: RTOS TinyML hooks | Paper 12 (partial) | First `sched_isr_on_entry/exit()` hook API |
| GAP-5: Energy-aware scheduling | Papers 9, 13, 16 (single-task) | Mixed-task energy measurement with INA219 |
| GAP-6: Schedulability for ML | Papers 15, 16 (no ML) | RTA-compatible WCET model for TFLM tasks |
| GAP-7: Reproducible task models | Papers 4, 25 (single-task) | YAML-driven, versioned task parameter system |
| GAP-8: Cross-MCU generalization | Papers 4, 6, 18 (limited) | Three-board evaluation (M7@480/216 MHz, M4@80 MHz) |

---

*Update this table when new papers are added to `research.md`.*
