# SchedTiny — Assets

Project branding, architecture diagrams, and presentation materials.

---

## Structure

```
assets/
├── logo/               # Project logo (SVG source + PNG exports)
├── diagrams/           # Architecture diagrams (draw.io / SVG source)
│   ├── system_architecture.svg
│   ├── measurement_pipeline.svg
│   └── experiment_workflow.svg
└── presentations/      # Conference/seminar slides
```

---

## Diagram Policy

Architecture diagrams must be:
- **Vector format** (SVG preferred, draw.io XML acceptable)
- **Source included** — not just PNG exports
- **Consistent** with `docs/ARCHITECTURE.md` descriptions

When diagrams appear in the paper, export as PDF from SVG:
```bash
inkscape --export-type=pdf assets/diagrams/system_architecture.svg
```

---

## Logo

The SchedTiny logo is pending design. Placeholder: text-based header in README.md.
