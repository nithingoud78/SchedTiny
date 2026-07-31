# SchedTiny Measurement Validation Guide

This guide details the procedure for scientifically validating the Measurement Infrastructure of SchedTiny. It is mandatory to complete this validation step on the hardware before executing any TinyML or RTOS workload benchmarks.

## 1. Test Architecture

The measurement framework relies on physical Cortex-M7 DWT register manipulation and hardware UART DMA. Validation is therefore split into two tiers:
1. **Host-Side Logic Validation (CMocka)**
2. **On-Target Hardware Validation (Logic Analyzer)**

## 2. Host-Side Validation (CMocka)

These tests execute natively on your development machine (Linux/Windows/macOS) using a mocked HAL layer (`mock_hal.c`). They verify pointer arithmetic, rollover logic, and boundary conditions that are difficult to predictably trigger on physical hardware.

### Procedure
1. Install `cmake` and `libcmocka-dev`.
2. Configure tests: `cmake -S firmware/tests -B build/tests`
3. Build tests: `cmake --build build/tests`
4. Run tests: `cd build/tests && ctest -V`

### Expected Output
- **`test_timer`**: Passes monotonicity and 32-bit `0xFFFFFFFF` overflow assertions.
- **`test_ring_buffer`**: Verifies that when flooded with >256 events, the buffer drops the latest events (incrementing `bench_get_dropped_events()`) rather than corrupting the unread tail pointer.
- **`test_logging`**: Validates standard string formatting into valid JSON syntax (`{"ts":..., "ev":...}`) without buffer overflows.

## 3. On-Target Hardware Validation

This verifies the true instruction overhead and physical timing characteristics.

### Hardware Required
- NUCLEO-H743ZI2 Evaluation Board
- Logic Analyzer (e.g., Saleae Logic 8 or compatible) capable of >24 MS/s.

### Procedure
1. Flash a firmware test app that toggles `DRV_GPIO_MARKER_0` via `drv_gpio_set()` and `drv_gpio_reset()`.
2. Wrap `bench_log_event()` between the set and reset calls.
3. Capture the output on the Logic Analyzer.

### Pass Criteria
1. **Overhead Budget**: The time delta between the rising edge (set) and falling edge (reset) must not exceed **50 cycles** (~104 nanoseconds at 480 MHz).
2. **UART Jitter**: The DMA transmission on the TX pin must begin asynchronously without stalling the CPU execution of subsequent toggles.

## 4. Common Failure Modes

*   **DWT Desynchronization**: If the system enters a deep sleep mode (STOP/STANDBY) where the core clock halts, the DWT counter will freeze. `bench_dwt_cycles64()` will then report a time significantly shorter than wall-clock time. *Mitigation: Disable sleep modes during experiments.*
*   **JSON Buffer Truncation**: If `BENCH_JSON_BUFFER_SIZE` is too small, a batch flush of 16 events might hit the boundary and truncate the JSON. *Mitigation: Ensure it is always sized > 1500 bytes.*
*   **Dropped Events**: If `bench_get_dropped_events() > 0`, the test is scientifically invalid. The idle task did not get enough CPU time to flush the ring buffer to UART before it overflowed. *Mitigation: Decrease test workload density or increase `BENCH_RING_BUFFER_SIZE`.*
