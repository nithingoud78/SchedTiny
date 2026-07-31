# SchedTiny — Third-Party Dependencies

This directory contains **vendored** copies of all third-party libraries
used by SchedTiny firmware. Vendoring (copying sources directly) is chosen
over git submodules or CMake FetchContent for the following reasons:

1. **Long-term reproducibility:** A paper published today must be reproducible
   in 5–10 years. Network-based downloads (submodules, FetchContent) cannot
   guarantee this.
2. **Offline builds:** Firmware must build in environments without internet access
   (lab workstations, secure build servers).
3. **Audit trail:** All source changes are visible in `git log`.

---

## Pinned Versions

| Library | Version | Commit SHA | Date Pinned | License |
|---|---|---|---|---|
| FreeRTOS | 10.6.2 | `TODO: fill in SHA after vendoring` | 2026-07-31 | MIT |
| TensorFlow Lite Micro | — | `TODO: fill in SHA after vendoring` | 2026-07-31 | Apache 2.0 |
| CMSIS-NN | 4.1.0 | `TODO: fill in SHA after vendoring` | 2026-07-31 | Apache 2.0 |

**Policy:** Any change to a pinned SHA requires:
1. A PR with the new SHA in this table
2. A review by the project lead
3. A CHANGELOG.md entry

---

## Directory Structure

```
third_party/
├── FreeRTOS/               # FreeRTOS kernel only (no demo or plus)
│   ├── include/            # FreeRTOS public headers
│   ├── portable/           # Port layer (GCC/ARM_CM7)
│   └── *.c                 # Kernel source files
├── tflite-micro/           # TensorFlow Lite Micro
│   ├── tensorflow/
│   └── third_party/        # TFLM's own vendored deps (flatbuffers, etc.)
└── CMSIS-NN/               # ARM CMSIS-NN kernels
    ├── Include/
    └── Source/
```

---

## How to Vendor a New Version

```bash
# 1. Clone the library at the target commit
git clone https://github.com/FreeRTOS/FreeRTOS-Kernel.git /tmp/freertos
cd /tmp/freertos
git checkout <target-sha>

# 2. Copy only the needed files (NOT the full repo)
rsync -av --include="*.c" --include="*.h" --include="*/" --exclude="*" \
      /tmp/freertos/ firmware/third_party/FreeRTOS/

# 3. Record the SHA in this README
# 4. Commit with message: "chore(third_party): vendor FreeRTOS 10.6.2 @ <sha>"
```

---

## License Files

Each vendored library's original LICENSE file is preserved in its subdirectory.
SchedTiny's Apache 2.0 license applies only to code in `firmware/src/`.
