# SchedTiny — Examples

Minimal working examples for new contributors. Start here before reading the full codebase.

---

## Examples

### `hello_freertos/`

**What it demonstrates:** Bare FreeRTOS task creation, DWT timer initialization, and
UART log output on the NUCLEO-H743ZI2. No TinyML, no scheduling research code.

**Build:**
```bash
cmake -S examples/hello_freertos -B build/examples/hello_freertos \
      -DCMAKE_TOOLCHAIN_FILE=tools/cmake/arm-none-eabi.cmake \
      -DBOARD=nucleo_h743zi2
cmake --build build/examples/hello_freertos
python scripts/build/flash.py --elf build/examples/hello_freertos/hello_freertos.elf
```

---

### `hello_tflm/`

**What it demonstrates:** Minimal TFLM inference on STM32 — loads a tiny model, runs
one forward pass, and logs the result over UART. No FreeRTOS.

**Prerequisite:** Third-party TFLM must be vendored (`firmware/third_party/tflite-micro/`).

---

### `hello_sched/`

**What it demonstrates:** Minimal SchedTiny scheduler — registers two tasks (PID + inference),
enables ISR hooks, and logs latency to UART. The minimal end-to-end SchedTiny pipeline.

---

## Contributing an Example

Examples must be:
- **Minimal** — the smallest possible code that demonstrates the concept
- **Buildable** — must pass CI build check for all three boards
- **Documented** — a `README.md` in the example folder explaining what it does

Do not add examples that replicate full experiments — those belong in `experiments/`.
