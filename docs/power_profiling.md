# SchedTiny Energy & Power Profiling Framework

The Phase 13 Energy & Power Profiling Framework adds simulated and hardware-ready energy evaluation capabilities to SchedTiny. This allows researchers to compare the energy efficiency of different scheduling algorithms (HPF, EDF, RMS) across varying workloads.

## Architecture & Power Models

Currently, the framework uses simulated power models (`sched_power.c`) to estimate energy consumption based on exact CPU cycle counts and idle/active times tracked by the dispatcher.

We provide three baseline software power models:

1. **Constant Power Model (`SCHED_POWER_MODEL_CONSTANT`)**
   - Assumes a uniform power draw for the entire execution duration, regardless of whether the system is active or idle.
   - Useful as a baseline for systems that do not employ any low-power sleep states.

2. **Idle/Active Power Model (`SCHED_POWER_MODEL_IDLE_ACTIVE`)**
   - Distinguishes between `busy_time` and `idle_time`.
   - Default constants for a standard Cortex-M7:
     - Active Power: 100 mW (100,000 µW)
     - Idle Power: 20 mW (20,000 µW)
   - Evaluates how effectively an algorithm maximizes deep sleep (Idle Time).

3. **Frequency-Scaled Model (`SCHED_POWER_MODEL_FREQ_SCALED`)**
   - Simulates Dynamic Voltage and Frequency Scaling (DVFS).
   - Multiplies active power by a scaling factor to approximate a burst frequency during active execution, demonstrating trade-offs in race-to-sleep algorithms.

## Exported Metrics

All benchmark runs export the following new metrics in both CSV and JSON formats:

- **Energy_uJ**: Total estimated energy consumption in microjoules.
- **Power_uW**: Average power draw in microwatts over the benchmark duration.
- **EnergyPerTask_uJ**: Energy efficiency metric dividing total energy by the number of successfully completed tasks.
- **EnergyPerCS_uJ**: Estimated overhead per context switch. We attribute a baseline overhead (e.g., 2 µJ) to every context switch to model pipeline flush and register stacking costs.

## Hardware Integration & Future Work

While current values use static constants for simulation, the `sched_power_compute()` API is designed to easily integrate with physical hardware measurements.

Future publication opportunities and extensions include:
- **INA219 / STLINK Integration**: Replacing static equations with real-time sampled current/voltage from an external ADC or STLINK-V3 power measurement tool.
- **Joulescope Validation**: Validating the software Idle/Active model against ground-truth physical traces.
- **Dynamic Voltage and Frequency Scaling (DVFS)**: Implementing an advanced policy that actively modifies the STM32 PLL clocks based on the scheduler's predicted idle time to minimize the energy integral.
