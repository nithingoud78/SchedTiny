# Build Guide

## Toolchain Setup and Build System Reference for SchedTiny

---

## 1. Required Software

| Tool | Version | Purpose | Download |
|---|---|---|---|
| Git | 2.40+ | Version control | https://git-scm.com |
| ARM GNU Toolchain | 12.3+ | Cross-compiler for Cortex-M | https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain |
| CMake | 3.25+ | Build system generator | https://cmake.org |
| STM32CubeProgrammer | 2.14+ | Flashing firmware | https://www.st.com/en/development-tools/stm32cubeprog.html |
| Python | 3.12+ | Analysis scripts, build automation | https://www.python.org |
| Doxygen | 1.9+ | API documentation generation | https://doxygen.nl |
| OpenOCD | 0.12+ | Optional: alternative debugger | https://openocd.org |

**Optional (for development):**
| Tool | Purpose |
|---|---|
| STM32CubeIDE 1.14+ | GUI IDE, debugger, CubeMX integration |
| VS Code + Cortex-Debug extension | Lightweight IDE + JTAG debugging |
| clang-format 16+ | C code formatting |
| Logic Analyzer software (Sigrok/Pulseview) | Timing capture |

---

## 2. Installation (Windows)

### ARM GNU Toolchain

```powershell
# Using winget
winget install Arm.GnuArmEmbeddedToolchain

# Or download installer from:
# https://developer.arm.com/downloads/-/arm-gnu-toolchain-downloads
# Choose: arm-gnu-toolchain-XX.X-mingw-w64-i686-arm-none-eabi.exe

# Verify
arm-none-eabi-gcc --version
```

### CMake

```powershell
winget install Kitware.CMake
cmake --version
```

### Python Environment

```powershell
# Create virtual environment
python -m venv .venv
.venv\Scripts\activate

# Install all dependencies
pip install -r scripts/requirements.txt
```

---

## 3. Installation (Ubuntu/WSL2)

```bash
# ARM GNU Toolchain
sudo apt-get install gcc-arm-none-eabi cmake

# Verify
arm-none-eabi-gcc --version   # Should be 12.x

# Python
python3 -m venv .venv
source .venv/bin/activate
pip install -r scripts/requirements.txt
```

---

## 4. Building the Firmware

### 4.1 Configure

```bash
# Primary target: NUCLEO-H743ZI2 with default config
cmake -S firmware -B build/h743 \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DBOARD=nucleo_h743zi2 \
      -DCMAKE_BUILD_TYPE=Release

# Alternate boards
cmake -S firmware -B build/f767 \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DBOARD=nucleo_f767zi \
      -DCMAKE_BUILD_TYPE=Release

cmake -S firmware -B build/l496 \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DBOARD=nucleo_l496zg \
      -DCMAKE_BUILD_TYPE=Release

# With a specific experiment config
cmake -S firmware -B build/exp003 \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DBOARD=nucleo_h743zi2 \
      -DCONFIG_FILE="experiments/EXP-003_interrupt_sweep/config.yaml" \
      -DCMAKE_BUILD_TYPE=Release
```

### 4.2 Build

```bash
cmake --build build/h743 --parallel 8

# Check sizes
arm-none-eabi-size build/h743/schedtiny.elf
```

### 4.3 Expected Outputs

```
build/h743/
├── schedtiny.elf       ← Flash with debugger (contains debug info)
├── schedtiny.bin       ← Raw binary for STM32CubeProgrammer
├── schedtiny.hex       ← Intel HEX format
└── schedtiny.map       ← Memory map (check flash/SRAM usage)
```

---

## 5. Flashing

### Using STM32CubeProgrammer (recommended)

```bash
# Via Python wrapper (handles board detection)
python scripts/build/flash.py --board nucleo_h743zi2 --elf build/h743/schedtiny.elf

# Directly via STM32_Programmer_CLI
STM32_Programmer_CLI -c port=SWD -w build/h743/schedtiny.bin 0x08000000 -rst
```

### Using OpenOCD

```bash
openocd -f tools/openocd/stm32h7.cfg \
        -c "program build/h743/schedtiny.elf verify reset exit"
```

---

## 6. UART Log Capture

SchedTiny outputs JSON log records over UART at 115200 baud, 8N1.

```bash
# Capture to file (Linux/WSL2)
python scripts/build/capture_uart.py --port /dev/ttyACM0 --duration 60 \
       --output results/EXP-001/raw/run_001.csv

# Windows
python scripts/build/capture_uart.py --port COM3 --duration 60 \
       --output results/EXP-001/raw/run_001.csv
```

---

## 7. Host-Side Unit Tests

Unit tests run on the host PC (no hardware required) using CMocka:

```bash
# Linux
sudo apt-get install libcmocka-dev

cmake -S firmware/tests -B build/tests -DCMAKE_BUILD_TYPE=Debug
cmake --build build/tests
cd build/tests && ctest --output-on-failure
```

---

## 8. CMake Build Parameters Reference

| Parameter | Values | Default | Description |
|---|---|---|---|
| `BOARD` | `nucleo_h743zi2`, `nucleo_f767zi`, `nucleo_l496zg` | `nucleo_h743zi2` | Target board |
| `CMAKE_BUILD_TYPE` | `Debug`, `Release`, `RelWithDebInfo` | `Release` | Build mode |
| `CONFIG_FILE` | Path to YAML | `configs/scheduler/fp_default.yaml` | Experiment config |
| `SCHED_POLICY` | `FPP`, `RM`, `EDF` | `FPP` | Scheduler policy override |
| `SCHED_BARE_METAL` | `0`, `1` | `0` | 1 = bare-metal baseline (no RTOS) |

---

## 9. Docker Build (CI)

The `ci/Dockerfile` provides a reproducible build environment:

```bash
# Build the container
docker build -t schedtiny-ci ci/

# Build firmware inside container
docker run --rm -v $(pwd):/workspace schedtiny-ci \
    cmake -S /workspace/firmware -B /workspace/build/docker \
          -DCMAKE_TOOLCHAIN_FILE=/workspace/tools/cmake/arm-none-eabi.cmake \
          -DBOARD=nucleo_h743zi2 && \
    cmake --build /workspace/build/docker
```

---

## 10. Verifying a Reproducible Build

A reproducible build means the same source + toolchain = bit-identical binary:

```bash
# Build twice and compare
cmake --build build/h743 -t clean
cmake --build build/h743
sha256sum build/h743/schedtiny.bin > run1.sha256

cmake --build build/h743 -t clean
cmake --build build/h743
sha256sum build/h743/schedtiny.bin > run2.sha256

diff run1.sha256 run2.sha256  # Should produce no output
```

Note: Reproducible builds require `SOURCE_DATE_EPOCH` to be set if the toolchain
embeds build timestamps. This is handled in `firmware/CMakeLists.txt`.
