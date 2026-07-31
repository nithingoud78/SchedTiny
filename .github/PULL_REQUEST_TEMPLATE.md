## Pull Request Summary

<!--
Fill out each section. PRs with empty sections will not be reviewed.
Reference: CONTRIBUTING.md
-->

### What does this PR do?

<!-- One sentence summary. Be specific about the component and behavior change. -->

### Motivation / Research Context

<!--
Why is this change needed?
If this is experiment-related: state the hypothesis.
If this is firmware: cite the SPEC.md requirement it satisfies.
If this is a bugfix: link to the bug report issue.
-->

Closes # <!-- issue number -->

---

### Type of Change

<!-- Check all that apply. -->

- [ ] `feat` — New feature (firmware, scripts, docs)
- [ ] `fix` — Bug fix
- [ ] `exp` — New or updated experiment
- [ ] `refactor` — No behavior change
- [ ] `test` — Tests only
- [ ] `docs` — Documentation only
- [ ] `ci` — CI/CD changes
- [ ] `chore` — Build, dependencies

---

### Affected Components

- [ ] `firmware/src/scheduler/`
- [ ] `firmware/src/bench/`
- [ ] `firmware/src/hal/`
- [ ] `firmware/src/ml/`
- [ ] `firmware/src/tasks/`
- [ ] `firmware/boards/`
- [ ] `models/`
- [ ] `experiments/`
- [ ] `scripts/analysis/`
- [ ] `scripts/build/`
- [ ] `configs/`
- [ ] `docs/`
- [ ] `ci/`
- [ ] Other: ___

---

### For Firmware Changes

- [ ] Passes `clang-format` (`tools/clang-format/.clang-format`)
- [ ] All public functions have Doxygen comments
- [ ] Unit tests added or updated (`firmware/tests/`)
- [ ] Tested on hardware: <!-- board name, firmware version -->
- [ ] No magic numbers — parameters are in `configs/*.yaml`
- [ ] Board-specific code is in `firmware/boards/`, not `firmware/src/`

---

### For Experiment Changes

- [ ] Uses `experiments/template/` structure
- [ ] Hypothesis written in `experiments/EXP-NNN/README.md` **before** running
- [ ] Config stored in `experiments/EXP-NNN/config.yaml` (mirrors `configs/`)
- [ ] Raw results stored in `results/EXP-NNN/raw/` (gitignored for `.csv`)
- [ ] Processed metrics in `results/EXP-NNN/processed/`
- [ ] Figures in `results/EXP-NNN/figures/` (PDF or SVG)
- [ ] Analysis notebook runs end-to-end: `jupyter nbconvert --execute analysis.ipynb`

---

### For Script Changes

- [ ] Passes `flake8 scripts/` and `black --check scripts/`
- [ ] All functions have type annotations and docstrings
- [ ] Script accepts CLI arguments (no hardcoded paths)
- [ ] Script is idempotent (running twice gives same output)
- [ ] `scripts/requirements.txt` updated if dependencies changed

---

### Documentation Updated

<!-- List all docs files updated in this PR -->

- [ ] Relevant `docs/` file updated
- [ ] `CHANGELOG.md` entry added under `[Unreleased]`
- [ ] `README.md` updated if user-facing behavior changed

---

### Testing

<!-- Describe how you verified this change. For firmware: board + measurements. -->

---

### Screenshots / Figures (optional)

<!-- Attach plots, oscilloscope captures, or UART log excerpts if relevant. -->
