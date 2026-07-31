# Style Guide

## Python and Markdown Style Rules for SchedTiny

---

## 1. Python Style

### 1.1 Tooling

| Tool | Purpose | Config |
|---|---|---|
| `black` | Code formatter (non-negotiable) | `pyproject.toml` (line length 100) |
| `flake8` | Style linter | `.flake8` (max-line-length 100, W503/E203 ignored) |
| `mypy` | Static type checker | `mypy.ini` |

Run before every commit:
```bash
black scripts/
flake8 scripts/ --max-line-length=100 --ignore=E203,W503
mypy scripts/ --ignore-missing-imports
```

### 1.2 Structure

Every Python script must follow this structure:

```python
#!/usr/bin/env python3
"""
Module docstring: one-paragraph description of what this script does,
what it consumes (input format), and what it produces (output format).

Usage:
    python parse_uart_log.py --input results/EXP-001/raw/run_001.csv \\
                             --output results/EXP-001/processed/metrics.json
"""

from __future__ import annotations

import argparse
import sys
from pathlib import Path
from typing import Optional  # Python 3.12 can use builtin Optional

import pandas as pd  # third-party imports after stdlib


def parse_args() -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    parser.add_argument("--input", type=Path, required=True, help="Path to raw UART log CSV")
    parser.add_argument("--output", type=Path, required=True, help="Path to output JSON file")
    return parser.parse_args()


def main() -> int:
    """Entry point. Returns 0 on success, non-zero on error."""
    args = parse_args()
    # ... implementation ...
    return 0


if __name__ == "__main__":
    sys.exit(main())
```

### 1.3 Naming Conventions

| Category | Convention | Example |
|---|---|---|
| Modules/files | `snake_case` | `parse_uart_log.py` |
| Functions | `snake_case` | `compute_percentile()` |
| Classes | `PascalCase` | `UartLogParser` |
| Constants | `UPPER_SNAKE_CASE` | `DEFAULT_WINDOW_SIZE = 1000` |
| Variables | `snake_case` | `latency_us`, `jitter_df` |

### 1.4 Type Annotations

All function signatures must include type annotations:

```python
# WRONG
def compute_percentile(data, p):
    ...

# CORRECT
def compute_percentile(data: list[float], p: float) -> float:
    """Compute the p-th percentile of data.

    Args:
        data: List of latency measurements in microseconds.
        p:    Percentile in [0, 100].

    Returns:
        The p-th percentile value.

    Raises:
        ValueError: If data is empty or p is out of range.
    """
    ...
```

### 1.5 Path Handling

- Always use `pathlib.Path`, never string concatenation for paths
- All scripts accept `Path` objects for file arguments
- Use relative paths from the repo root in documentation; use `Path` for actual code

```python
# WRONG
output_path = results_dir + "/" + "metrics.json"

# CORRECT
output_path = results_dir / "metrics.json"
```

### 1.6 Data Conventions

- Raw UART log columns: `timestamp_us`, `event_type`, `task_id`, `value_us`
- Processed metrics: stored as JSON with keys matching SPEC metric names
  (e.g., `"latency_p99_us"`, `"jitter_std_us"`, `"deadline_miss_rate"`)
- Figure file names: `fig_<metric>_<plot_type>.pdf`
  (e.g., `fig_latency_cdf.pdf`, `fig_jitter_boxplot.pdf`)

---

## 2. Markdown Style

### 2.1 Document Structure

Every Markdown document must have:
- A `# H1` title (only one per document)
- A brief intro paragraph (no heading) explaining the document's purpose
- Sections using `## H2` and `### H3`
- A consistent, logical reading order

### 2.2 Formatting Rules

- **Bold** for important terms on first use
- `code` for all file names, paths, commands, and code symbols
- Line length: soft limit 100 characters for prose; no limit for tables/code blocks
- Lists: use `-` for unordered lists; `1.` for numbered steps
- Tables: always include a header row; align columns with spaces

### 2.3 Links

- Internal links: use relative paths from the document's location
  ```markdown
  See [SPEC.md](SPEC.md) for requirements.          ✅
  See [SPEC.md](/docs/SPEC.md)                       ✅ from repo root
  See [SPEC.md](https://github.com/.../SPEC.md)      ❌ breaks on local clones
  ```
- External links: always include the URL; prefer stable DOI links for papers

### 2.4 Research Documents

In `references/research.md` and related files:
- Every claim from a paper must include the paper's key from `citations.bib`
- Use consistent section headers matching the existing format
- Never edit existing paper entries without a git commit message explaining why

---

## 3. YAML Style

All YAML files in `configs/` follow these rules:

```yaml
# File-level comment: what this config is for and what it overrides

experiment:
  id: "EXP-001"              # Quoted strings for IDs
  description: "Baseline"

scheduler:
  policy: FPP                # Enum values unquoted
  tick_rate_hz: 1000         # Numbers unquoted

tasks:
  pid:
    period_ms: 1             # Use descriptive suffixes (_ms, _hz, _us)
    priority: 3              # Higher = higher priority in FreeRTOS
    wcet_budget_us: 200
```

Rules:
- 2-space indentation
- Quoted strings for IDs, names, descriptions; unquoted for numbers and enums
- Units in key names where applicable (`_ms`, `_us`, `_hz`, `_kb`)
- Comments on non-obvious fields
- yamllint must pass (enforced by CI)

---

## 4. Commit Message Style

See `CONTRIBUTING.md` for the full Conventional Commits specification.

Additional rules:
- Subject line: imperative mood, ≤ 72 characters, no period at end
- Body: explain WHY the change was made, not what
- Footer: `Closes #NNN` for issues; `See also: EXP-NNN` for experiments
- Avoid vague messages: `"fix bug"`, `"update stuff"`, `"WIP"` are rejected at review
