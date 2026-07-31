# Results

This directory contains experimental results for all SchedTiny experiments.

---

## Structure

```
results/
├── EXP-001_baseline_baremetal/
│   ├── raw/                  # UART log files (.csv — GITIGNORED)
│   ├── processed/            # Computed metrics (.json — committed)
│   └── figures/              # Journal-quality figures (.pdf — committed)
├── EXP-002_.../
│   └── ...
└── summary_table.csv         # Cross-experiment metric comparison (committed)
```

---

## What Is Committed vs. Gitignored

| Content | Git Status | Rationale |
|---|---|---|
| `raw/*.csv` | ❌ Gitignored | Large files; regenerable from `run.sh` |
| `processed/*.json` | ✅ Committed | Small; essential for cross-experiment analysis |
| `figures/*.pdf` | ✅ Committed | These ARE the paper figures |
| `summary_table.csv` | ✅ Committed | Cross-experiment comparison table |

---

## Regenerating Raw Results

If raw files are missing (normal after a fresh clone):

```bash
cd experiments/EXP-001_baseline_baremetal
bash run.sh
```

Each `run.sh` is self-contained and reproducible.

---

## Data Integrity

Raw log files are **write-once**. Never edit them after capture.
If a run was corrupted, re-run the experiment and store as `run_002.csv`.

All processed results are derived deterministically from raw files via
versioned Python scripts. The analysis notebook runs reproducibly:

```bash
jupyter nbconvert --to notebook --execute analysis.ipynb
```
