# Papers

This directory contains LaTeX source for SchedTiny journal publications.

---

## Structure

```
papers/
├── journal_v1/             # First journal submission
│   ├── main.tex            # Root LaTeX file
│   ├── references.bib      # Symlink or copy of references/citations.bib
│   ├── figures/            # Figures from results/*/figures/ (copy here before submission)
│   └── README.md           # Submission target, revision history
└── README.md               # This file
```

---

## Figure Policy

All figures in `papers/journal_v1/figures/` must originate from
`results/EXP-NNN/figures/`. Never hand-draw or import figures from external sources.

Before submission, copy the final figures:
```bash
cp results/EXP-*/figures/*.pdf papers/journal_v1/figures/
```

---

## Submission Targets

| Venue | Target | Impact Factor | Status |
|---|---|---|---|
| IEEE Trans. on Embedded Computing Systems (TECS) | v1 | ~4.0 | 🔄 Planned |
| IEEE Embedded Systems Letters (ESL) | Backup | ~3.5 | 🔄 Planned |

---

## Revision History

| Date | Action | Notes |
|---|---|---|
| 2026-07-31 | Repository created | Paper folder initialized |

---

## LaTeX Template

Use the IEEE Transactions template:
- Download from https://www.ieee.org/conferences/publishing/templates.html
- Select "LaTeX Template" for transactions/journals
- Place files in `papers/journal_v1/`
