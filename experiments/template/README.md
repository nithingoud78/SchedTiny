# EXP-TEMPLATE: [Replace with Descriptive Experiment Title]

> Copy this template with: `cp -r experiments/template/ experiments/EXP-NNN_short_name/`
> Fill in every section BEFORE running the experiment.
> See `docs/EXPERIMENT_GUIDE.md` for the full process.

---

## Experiment Header

- **Experiment ID:** EXP-XXX
- **Status:** Planned
- **Researcher:** @your_github_handle
- **Date Started:** YYYY-MM-DD
- **Date Completed:** —
- **Hardware:** STM32 NUCLEO-H743ZI2 (or specify alternate)
- **Firmware SHA:** *(fill in after flash)*
- **Config file:** `experiments/EXP-XXX/config.yaml`
- **Related Gap:** GAP-N (from `docs/SPEC.md`)
- **Related Issue:** #N

---

## Hypothesis

> Write one falsifiable hypothesis here, before running.
>
> Example format:
> "We hypothesize that [independent variable change] will cause [dependent variable
> to change in direction] by approximately [magnitude], because [mechanistic reason]."

---

## Independent Variables

| Variable | Values | Units |
|---|---|---|
| *e.g., ISR rate* | *0, 500, 1000, 5000* | *Hz* |

---

## Dependent Variables

| Metric | Measurement Method | Resolution |
|---|---|---|
| *e.g., KWS p99 latency* | *DWT cycle counter* | *2 ns @ 480 MHz* |
| *e.g., PID miss rate* | *FreeRTOS task stats* | *%* |

---

## Controlled Variables

- FreeRTOS scheduler policy: *specify*
- Task priorities: as per `configs/tasks/`
- Board: NUCLEO-H743ZI2
- Firmware SHA: *fill after flash*
- Sample count: *N* consecutive inferences per condition
- Warmup samples: 100 (discarded)

---

## Expected Outcome

*What do you predict will happen, and why?*

---

## Actual Outcome

*(Fill in after running — do not modify hypothesis)*

---

## Interpretation

*(Fill in after analysis)*

---

## References

- *Cite papers from references/citations.bib that motivated this experiment*
