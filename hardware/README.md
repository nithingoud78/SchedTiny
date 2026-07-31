# Hardware Documentation

This directory contains all hardware-related documentation for SchedTiny.

---

## Contents

```
hardware/
├── schematics/         # KiCad or PDF schematics (wiring diagrams)
├── bom/
│   └── bom.csv         # Bill of Materials
├── pinouts/            # Board-specific pinout reference tables
└── measurement_rig/    # INA219 + logic analyzer setup documentation
```

---

## Primary Hardware

| Component | Part Number | Purpose |
|---|---|---|
| MCU Board | NUCLEO-H743ZI2 | Primary benchmark target |
| IMU Sensor | MPU6050 | Sensor acquisition task source |
| Power Monitor | INA219 | Energy-per-inference measurement |
| Logic Analyzer | 24 MHz clone | Hardware ground truth for timing |

See `docs/HARDWARE_GUIDE.md` for complete wiring instructions and measurement rig setup.

---

## Pin Assignment Reference

Full pin assignments are in `docs/HARDWARE_GUIDE.md` Section 2.
Board-specific pin constants are in `firmware/boards/<board>/board_config.h`.

---

## Measurement Rig Calibration

Before every experiment:
1. Measure shunt resistance with precision multimeter
2. Update `configs/boards/nucleo_h743zi2.yaml` with measured value
3. Document in session note (`meeting_notes/YYYY-MM-DD_*.md`)
