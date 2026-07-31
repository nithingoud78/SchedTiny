# Research Guide

## How to Add a New Research Finding to SchedTiny

This guide defines the **research → code contract**: the process by which a finding in
`references/research.md` becomes a traceable implementation, experiment, or documented gap.

Every sentence in `references/research.md` that affects implementation must be traceable
to one of: a `docs/` file, a `configs/` entry, an `experiments/` folder, or a GitHub issue.

---

## 1. Where Research Lives

| Document | Purpose |
|---|---|
| `references/research.md` | Primary input. Live literature survey. Add findings here first. |
| `references/literature_matrix.md` | Gap analysis table. One row per paper. Tracks which gaps each paper addresses and leaves open. |
| `references/glossary.md` | Domain terminology. Every SchedTiny-specific term defined here first. |
| `references/ideas.md` | Unvalidated ideas. Not yet in SPEC. No commitment to implement. |
| `references/open_questions.md` | Unanswered research questions that block progress. |
| `references/citations.bib` | BibTeX database. Every citation in code comments and papers comes from here. |

---

## 2. The Research Integration Workflow

### Step 1 — Add the Finding to `research.md`

When you read a new paper, add a structured entry to `references/research.md` following
the existing Section A format:

```markdown
### N. Paper Title
- Authors, Year, Venue, DOI
- Research problem
- Proposed solution
- Hardware, RTOS, MCU, TinyML framework
- Scheduling algorithm
- Evaluation metrics
- Main results
- Limitations
- Future work
- **Relevance to SchedTiny:** [specific connection]
- **Remaining gap:** [what they did NOT address]
```

### Step 2 — Update the Literature Matrix

Open `references/literature_matrix.md` and add a row for the new paper.
The matrix has one column per research gap (GAP-1 through GAP-8 from `docs/SPEC.md`).
Mark each cell: `✅ Addressed`, `⚠️ Partial`, or `❌ Not addressed`.

This matrix is what you will use to write the Related Work section of the paper.

### Step 3 — Classify the Finding

Determine which category the finding belongs to:

| Category | Action |
|---|---|
| **Confirms an existing gap** | Add note to the relevant gap in `docs/SPEC.md` citing the new paper |
| **Proposes a technique to adopt** | Add to `references/ideas.md`; if validated, open a GitHub feature request |
| **Identifies a mistake to avoid** | Add to `docs/EXPERIMENT_GUIDE.md` Section "Known Pitfalls" |
| **Provides a new metric** | Propose addition to `docs/SPEC.md` REQ-MEAS-* and `scripts/analysis/compute_metrics.py` |
| **Provides new hardware data** | Update `docs/HARDWARE_GUIDE.md` and `firmware/boards/` if a new board is needed |
| **Provides a baseline result** | Document in `experiments/EXP-001/README.md` as a literature baseline |
| **Opens a new research question** | Add to `references/open_questions.md` |

### Step 4 — Update the Bibliography

Add the BibTeX entry to `references/citations.bib`:

```bibtex
@inproceedings{authorYYYYkeyword,
  author    = {Last, First and Last2, First2},
  title     = {Title of the Paper},
  booktitle = {Conference or Journal Name},
  year      = {YYYY},
  doi       = {10.XXXX/XXXXX},
  url       = {https://...}
}
```

Use the key format: `authorYYYYfirstword` (e.g., `banbury2020benchmarking`).

### Step 5 — Open a GitHub Issue (if actionable)

If the finding requires a code change, configuration change, or new experiment:

1. Open a GitHub issue using the appropriate template
2. Label it with the research gap it addresses (e.g., `GAP-3`)
3. Link to the paper entry in `research.md`

---

## 3. Citation Policy

**Every** factual claim in `docs/`, `experiments/`, and firmware comments that
originates from external literature must include a BibTeX key in the format:

- In Markdown files: `(Author et al., YEAR [key])` or a footnote
- In C comments: `/* See [banbury2020benchmarking] */`
- In paper: `\cite{banbury2020benchmarking}`

**Do not paraphrase a result without a citation.** This is a journal-quality project.

---

## 4. Research Diary Protocol

After every research session, create a session note in `meeting_notes/`:

**File name:** `YYYY-MM-DD_session-title.md`

**Required sections:**

```markdown
## Date: YYYY-MM-DD
## Duration: X hours

### Papers Read
- [paper title](DOI) — key takeaway

### Findings Added to research.md
- Finding description → Gap N

### Ideas Added to ideas.md
- Idea description

### Open Questions Added
- Question

### Implementation Impact
- What firmware/experiment/config change does this require?

### Next Session Plan
- What to do next
```

**Rule:** Session notes are permanent. Never delete them. They are part of the
research provenance record for the MS portfolio.

---

## 5. Literature Review Timeline

Based on `references/research.md` current state (25 papers surveyed, 8 gaps confirmed):

| Phase | Goal | Status |
|---|---|---|
| Phase 1 | Core TinyML + real-time papers (papers 1–10) | ✅ Done |
| Phase 2 | Benchmarking + MLPerf context (papers 11–20) | ✅ Done |
| Phase 3 | Scheduling theory papers (EDF, RM, MCS) | 🔄 Pending |
| Phase 4 | TFLM internals + CMSIS-NN architecture | 🔄 Pending |
| Phase 5 | STM32 HAL + FreeRTOS internals | 🔄 Pending |

Target: 40–50 papers before first experiment, 60–80 before paper submission.
