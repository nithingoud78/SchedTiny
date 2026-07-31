# Changelog

All notable changes to SchedTiny are documented in this file.

The format follows [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).
This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

> **Research versioning note:** Major versions correspond to journal submission
> milestones. Minor versions correspond to new experiment sets. Patch versions
> correspond to bugfixes and documentation improvements.

---

## [Unreleased]

### Added
- Complete repository architecture and project skeleton
- Root documentation: README, CONTRIBUTING, CODE_OF_CONDUCT, SECURITY, ROADMAP
- `docs/` documentation suite: ARCHITECTURE, SPEC, RESEARCH_GUIDE, EXPERIMENT_GUIDE,
  DEVELOPMENT_GUIDE, BUILD_GUIDE, HARDWARE_GUIDE, DATASET_GUIDE, CODING_STANDARD,
  STYLE_GUIDE
- `firmware/` skeleton: CMakeLists.txt, HAL, scheduler, bench, ML layer, board configs
- `experiments/` template and EXP-001 skeleton
- `scripts/` analysis and build automation pipeline
- `configs/` YAML configuration system
- `tools/` build system: Doxyfile, clang-format, ARM toolchain file
- `ci/` Docker reproducible build environment
- GitHub issue templates (bug, feature, experiment failure, paper feedback)
- GitHub Actions CI/CD workflows

### Changed
- Nothing (initial release)

### Deprecated
- Nothing

### Removed
- Nothing

### Fixed
- Nothing

### Security
- Nothing

---

## [0.1.0] — 2026-07-31

### Added
- Repository initialized with literature survey (`references/research.md`)
- Initial project structure proposal (`references/reference.md`)
- Hardware selection: STM32 NUCLEO-H743ZI2 (Cortex-M7 @ 480 MHz, primary target)
- Software stack confirmed: FreeRTOS + TFLM + CMSIS-NN

---

[Unreleased]: https://github.com/nithingoud78/SchedTiny/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/nithingoud78/SchedTiny/releases/tag/v0.1.0
