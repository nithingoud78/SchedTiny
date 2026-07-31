# Security Policy

## Supported Versions

SchedTiny is a research framework targeting embedded microcontrollers. Security
support is provided for the following versions:

| Version | Supported          |
| ------- | ------------------ |
| main    | ✅ Active          |
| Tagged releases | ✅ Best effort |
| Archived branches | ❌ No |

## Scope

SchedTiny firmware runs on air-gapped embedded hardware (STM32 MCUs) without
network connectivity. The primary security concerns are:

1. **Supply chain integrity** — vendored third-party dependencies (FreeRTOS, TFLM)
   must use verified, pinned commit SHAs. Never pull from an unverified fork.

2. **Build system integrity** — CMake scripts and Python tooling must not download
   unverified binaries at build time.

3. **Model integrity** — TFLite model files (`.tflite`) embedded in firmware must
   be sourced from verified training pipelines. Do not embed third-party models
   without audit.

4. **Measurement data integrity** — Experimental results must not be tampered with.
   Raw UART logs are write-once; processed results are derived deterministically
   from raw logs via versioned scripts.

## Reporting a Vulnerability

If you discover a security vulnerability (e.g., a supply chain issue, a malicious
dependency, or a build system backdoor), please:

1. **Do NOT open a public GitHub issue.**
2. Open a [GitHub Security Advisory](https://github.com/nithingoud78/SchedTiny/security/advisories/new)
   using GitHub's private disclosure mechanism.
3. Alternatively, email the maintainer directly with `[SECURITY]` in the subject line.

We will acknowledge receipt within **72 hours** and provide a resolution timeline
within **7 days**.

## Responsible Disclosure

We ask that you:
- Allow reasonable time for a fix before public disclosure
- Provide sufficient detail to reproduce the issue
- Do not access, modify, or delete any data beyond what is necessary to demonstrate
  the vulnerability

## Third-Party Dependencies

| Dependency | Source | Version Policy |
|---|---|---|
| FreeRTOS | [freertos.org](https://www.freertos.org) | Pinned commit SHA in `firmware/third_party/README.md` |
| TensorFlow Lite Micro | [github.com/tensorflow/tflite-micro](https://github.com/tensorflow/tflite-micro) | Pinned commit SHA |
| CMSIS-NN | [github.com/ARM-software/CMSIS-NN](https://github.com/ARM-software/CMSIS-NN) | Pinned commit SHA |

Any change to a pinned SHA requires explicit maintainer review and a changelog entry.
