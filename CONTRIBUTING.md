# Contributing to SchedTiny

Thank you for contributing to SchedTiny — an open, research-grade benchmark and
interrupt-aware scheduling framework for real-time TinyML on STM32 microcontrollers.

> **This is not a student project or a demo.** Contributions are held to the standards
> of an industrial open-source framework intended for journal publication and long-term
> maintenance. Please read this document fully before opening a pull request.

---

## Table of Contents

1. [Who Should Contribute](#who-should-contribute)
2. [Code of Conduct](#code-of-conduct)
3. [Research Integrity Rules](#research-integrity-rules)
4. [Getting Started](#getting-started)
5. [Branch Strategy](#branch-strategy)
6. [Commit Message Convention](#commit-message-convention)
7. [Pull Request Process](#pull-request-process)
8. [Contribution Types](#contribution-types)
9. [Review Standards](#review-standards)
10. [Documentation Requirements](#documentation-requirements)
11. [Experiment Contributions](#experiment-contributions)
12. [Firmware Contributions](#firmware-contributions)

---

## Who Should Contribute

SchedTiny welcomes contributions from:

- Embedded systems researchers (RTOS, real-time scheduling)
- TinyML practitioners (TFLM, CMSIS-NN, model optimization)
- Hardware engineers (STM32 platform expertise)
- Research software engineers (Python analysis pipelines, reproducibility)

If you are unfamiliar with real-time systems theory or embedded C, please read
`docs/DEVELOPMENT_GUIDE.md` and `docs/RESEARCH_GUIDE.md` before contributing.

---

## Code of Conduct

All contributors must follow our [Code of Conduct](CODE_OF_CONDUCT.md).
Academic integrity violations (fabricated results, plagiarism) will result in
permanent ban from the project.

---

## Research Integrity Rules

These rules are **non-negotiable**:

1. **Every graph must come from real experimental data.** No illustrative plots.
2. **Every experiment must be reproducible** from `run.sh` on a fresh hardware setup.
3. **Never claim a result without a corresponding `results/EXP-NNN/` folder.**
4. **Never change a raw log file.** Raw logs are write-once; derive everything else.
5. **Cite every claim** that comes from external literature with a BibTeX key from
   `references/citations.bib`.

---

## Getting Started

```bash
# 1. Fork the repository on GitHub
# 2. Clone your fork
git clone https://github.com/YOUR_USERNAME/SchedTiny.git
cd SchedTiny

# 3. Set up upstream remote
git remote add upstream https://github.com/nithingoud78/SchedTiny.git

# 4. Set up Python environment
python -m venv .venv
source .venv/bin/activate   # Windows: .venv\Scripts\activate
pip install -r scripts/requirements.txt

# 5. Read the docs before writing any code
# docs/DEVELOPMENT_GUIDE.md    — firmware workflow
# docs/EXPERIMENT_GUIDE.md     — how to design experiments
# docs/CODING_STANDARD.md      — C coding rules
# docs/STYLE_GUIDE.md          — Python + C style rules
```

For toolchain setup (ARM GNU, STM32CubeProgrammer, CMake), see `docs/BUILD_GUIDE.md`.

---

## Branch Strategy

| Branch | Purpose |
|---|---|
| `main` | Protected. Stable, reviewed, reproducible state. |
| `dev` | Integration branch. All PRs target `dev`. |
| `feat/your-description` | Feature branches. Branch from `dev`. |
| `exp/EXP-NNN-description` | Experiment branches. Branch from `dev`. |
| `fix/short-description` | Bugfix branches. |
| `docs/short-description` | Documentation-only changes. |

**Never commit directly to `main` or `dev`.** Open a pull request.

---

## Commit Message Convention

Use [Conventional Commits](https://www.conventionalcommits.org/en/v1.0.0/):

```
<type>(<scope>): <short summary in imperative mood>

[optional body — explain WHY, not what]

[optional footer — references issues, breaking changes]
```

**Types:**

| Type | When to Use |
|---|---|
| `feat` | New feature in firmware or scripts |
| `fix` | Bug fix |
| `exp` | New or updated experiment |
| `docs` | Documentation only |
| `refactor` | Code change with no behavior change |
| `test` | Adding or fixing tests |
| `ci` | CI/CD changes |
| `chore` | Build system, dependencies |

**Scopes:** `scheduler`, `bench`, `hal`, `ml`, `tasks`, `scripts`, `docs`, `configs`, `ci`

**Examples:**

```
feat(scheduler): add ISR hook API for interrupt-induced jitter measurement

fix(bench): correct DWT overflow handling for measurements > 89 seconds

exp(EXP-003): add interrupt sweep experiment config and analysis notebook

docs(ARCHITECTURE): update scheduler component diagram
```

---

## Pull Request Process

1. **Branch** from `dev` using the naming convention above.
2. **Write tests** for any new firmware component (see `firmware/tests/`).
3. **Update documentation** — no undocumented public APIs.
4. **Run CI locally** before opening a PR:
   ```bash
   # Build check (all three boards)
   cmake -S firmware -B build -DBOARD=nucleo_h743zi2 && cmake --build build
   # Python lint
   flake8 scripts/ && black --check scripts/
   ```
5. **Open a PR** against `dev` using the pull request template.
6. **Request review** from the appropriate CODEOWNER (see `.github/CODEOWNERS`).
7. **Address all review comments** before merging.
8. **Squash and merge** — one commit per feature on `dev`.

---

## Contribution Types

### Firmware (C)

- Follow `docs/CODING_STANDARD.md` strictly
- All public functions must have Doxygen comments
- All new scheduler policies must have a unit test in `firmware/tests/`
- Board-specific code goes in `firmware/boards/`, never in `firmware/src/`

### Experiments

- Use the template in `experiments/template/`
- Write the hypothesis in `README.md` **before** running the experiment
- Configs go in `configs/`, not hardcoded in C or Python
- Raw results go in `results/EXP-NNN/raw/` (gitignored for `.csv`)
- Processed results and figures go in `results/EXP-NNN/processed/` and `figures/`

### Python Scripts

- Follow `docs/STYLE_GUIDE.md`
- All scripts must accept command-line arguments (no hardcoded paths)
- Functions must have type annotations and docstrings
- Scripts must be idempotent — running twice produces the same output

### Documentation

- Use plain, precise English — this is read by international researchers
- Cross-reference other docs using relative links
- All hardware connections must include a wiring diagram or table

---

## Review Standards

PRs are reviewed for:

| Criterion | Standard |
|---|---|
| Correctness | Code does what the PR description claims |
| Reproducibility | Experiments run end-to-end on a fresh setup |
| Research integrity | Results come from real data |
| Documentation | Public APIs documented; experiment hypothesis documented |
| Test coverage | New firmware components have unit tests |
| Style | Passes `clang-format` and `black`/`flake8` |
| Commit history | Clean conventional commits |

---

## Documentation Requirements

Every PR that changes code **must** update the relevant documentation:

| Change | Required Doc Update |
|---|---|
| New firmware component | Doxygen header + `docs/ARCHITECTURE.md` if architecture changes |
| New scheduler policy | Entry in `docs/SPEC.md` + config in `configs/scheduler/` |
| New experiment | `experiments/EXP-NNN/README.md` with hypothesis |
| New dataset | `datasets/*/README.md` with source, license, download instructions |
| New hardware target | `firmware/boards/<name>/`, `docs/HARDWARE_GUIDE.md` |
| Changed metric | `docs/EXPERIMENT_GUIDE.md` + `scripts/analysis/compute_metrics.py` |

---

## Experiment Contributions

Experiments are the scientific output of SchedTiny. They must be:

1. **Hypothesis-driven** — write what you expect to happen before running
2. **Self-contained** — `experiments/EXP-NNN/run.sh` produces all raw results
3. **Reproducible** — another researcher must be able to reproduce your results on
   the same hardware within ±5% of reported metrics
4. **Numbered** — use the next available `EXP-NNN` index; claim it in a PR first

See `docs/EXPERIMENT_GUIDE.md` for the full experiment design process.

---

## Firmware Contributions

Firmware changes carry the highest review bar because they affect measurement validity.

- **HAL changes** require testing on all three supported boards
- **Scheduler changes** require a new unit test and a benchmark run
- **Bench layer changes** require validation against a known-good result
- **Third-party dependency updates** require explicit SHA update in
  `firmware/third_party/README.md` and a changelog entry

---

*Questions? Open a GitHub Discussion or check `references/open_questions.md`.*
