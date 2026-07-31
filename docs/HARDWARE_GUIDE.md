# Hardware Guide

## Board Setup, Wiring, and Measurement Rig Documentation

---

## 1. Supported Hardware

### Primary Target

| Item | Part | Notes |
|---|---|---|
| MCU Board | STM32 NUCLEO-H743ZI2 | Cortex-M7, 480 MHz, 1 MB SRAM, 2 MB Flash |
| IMU | MPU6050 | I²C, 400 kHz, accelerometer + gyroscope |
| Power Monitor | INA219 | I²C, 12-bit, 0.1Ω shunt, measures board current |
| Logic Analyzer | 24 MHz clone (e.g., Saleae Logic 8) | GPIO timing measurement |
| Breadboard | Full-size | Prototyping |
| Jumper Wires | M-M, M-F | Various lengths |

### Alternate Targets

| Board | MCU | Core | Clock | SRAM | Flash |
|---|---|---|---|---|---|
| NUCLEO-F767ZI | STM32F767ZI | Cortex-M7 | 216 MHz | 512 KB | 2 MB |
| NUCLEO-L496ZG | STM32L496ZG | Cortex-M4 | 80 MHz | 320 KB | 1 MB |

---

## 2. Pin Assignments (NUCLEO-H743ZI2)

### MPU6050 (I²C Sensor)

| MPU6050 Pin | NUCLEO Pin | Signal |
|---|---|---|
| VCC | 3.3V (CN8 pin 7) | Power |
| GND | GND (CN8 pin 11) | Ground |
| SCL | PB8 (CN7 pin 1) | I²C1 SCL |
| SDA | PB9 (CN7 pin 2) | I²C1 SDA |
| INT | PB3 (CN7 pin 15) | Interrupt (optional) |
| AD0 | GND | I²C address = 0x68 |

### INA219 (Power Monitor)

| INA219 Pin | NUCLEO Pin | Signal |
|---|---|---|
| VCC | 3.3V (CN8 pin 7) | Power |
| GND | GND (CN8 pin 11) | Ground |
| SCL | PF0 (CN9 pin 30) | I²C2 SCL |
| SDA | PF1 (CN9 pin 28) | I²C2 SDA |
| A0 | GND | |
| A1 | GND | I²C address = 0x40 |

**Shunt resistor:** 0.1Ω connected in series with the 3.3V rail feeding the board
(between the INA219 VIN+ and VIN- pins). Do NOT use the onboard shunt if available —
measure the actual rail, not just the USB supply.

### Logic Analyzer Trigger Pins (GPIO)

| Signal | NUCLEO Pin | Purpose |
|---|---|---|
| PID task start | PC0 (CN9 pin 3) | Toggle HIGH on task entry |
| PID task end | PC1 (CN9 pin 5) | Toggle LOW on task exit |
| Inference start | PC2 (CN9 pin 6) | Toggle HIGH on inference start |
| Inference end | PC3 (CN9 pin 8) | Toggle LOW on inference end |
| ISR entry | PG2 (CN9 pin 33) | Toggle HIGH on ISR entry |

These GPIO toggles are the **hardware ground truth** for timing measurements.
DWT cycle counter readings are validated against logic analyzer captures.

### UART Debug Output

| Signal | NUCLEO Pin | Settings |
|---|---|---|
| UART TX | PD8 (USART3 TX) | 115200 baud, 8N1 |
| UART RX | PD9 (USART3 RX) | (reception not used) |

On NUCLEO boards, USART3 is connected to the ST-LINK virtual COM port.
Connect via USB-B port; appears as `COMx` (Windows) or `/dev/ttyACM0` (Linux).

---

## 3. Measurement Rig Setup

### 3.1 INA219 Power Measurement

The INA219 must be wired **in series** with the power supply rail, not as a
parallel voltage sensor. Incorrect wiring will produce zero current readings.

```
USB 5V
  │
  ▼
 [AMS1117 3.3V regulator]
  │
  ├──[INA219 VIN+]──[0.1Ω shunt]──[INA219 VIN-]──[Board 3.3V rail]
  │
  GND
```

**Calibration:** Before every experiment session, measure the shunt resistance
with a precision multimeter and update `configs/boards/nucleo_h743zi2.yaml`:

```yaml
power_monitor:
  shunt_ohms: 0.1002   # Measured, not nominal
  max_expected_ma: 500
```

### 3.2 Logic Analyzer Connections

Connect the 24 MHz logic analyzer probes to the GPIO trigger pins listed above.
Set the sampling rate to ≥ 12 MHz (Nyquist for 6 MHz signals).

**Recommended Sigrok command for batch capture:**

```bash
sigrok-cli -d fx2lafw:conn=1.7 \
           --config samplerate=12m \
           --samples 12000000 \
           -o results/EXP-001/raw/logic_run_001.sr
```

### 3.3 Logic Analyzer Validation

After each experiment, verify DWT measurements against logic analyzer:

```bash
python scripts/analysis/validate_dwt.py \
       --uart results/EXP-001/raw/run_001.csv \
       --logic results/EXP-001/raw/logic_run_001.sr
```

The tolerance between DWT and logic analyzer measurements should be ≤ 1 µs.
If greater, check for DWT overflow or clock configuration errors.

---

## 4. Board-Specific Notes

### NUCLEO-H743ZI2

- **Clock configuration:** HSE 8 MHz → PLL → 480 MHz SYSCLK. Verify with `SystemCoreClock` variable.
- **DWT enabling:** DWT is disabled by default. Enable in startup code:
  ```c
  CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
  DWT->CYCCNT = 0;
  DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  ```
- **Cache:** I-cache and D-cache enabled by default via `SCB_EnableICache()` / `SCB_EnableDCache()`.
  For deterministic measurements, disable D-cache during calibration runs.
- **UART:** USART3 is the ST-LINK VCP. Configure at 115200 baud with DMA1 stream 3.

### NUCLEO-F767ZI

- Same DWT setup as H743ZI2
- Clock: HSE 8 MHz → 216 MHz SYSCLK
- Lower SRAM (512 KB): check TFLM model fits

### NUCLEO-L496ZG

- Cortex-M4: no FPU double precision; ensure TFLM is compiled for single precision
- Clock: 80 MHz (significantly slower — inference will be 6× slower than H743ZI2)
- Lower SRAM (320 KB): carefully monitor footprint

---

## 5. Hardware Procurement

All parts are commonly available. Estimated cost:

| Item | Source | Approx. Cost (USD) |
|---|---|---|
| NUCLEO-H743ZI2 | STMicroelectronics, Mouser, Digi-Key | ~$25 |
| MPU6050 module | AliExpress, Amazon | ~$2 |
| INA219 module | AliExpress, Amazon | ~$3 |
| 0.1Ω precision resistor | Digi-Key | ~$1 |
| 24 MHz Logic Analyzer | Amazon | ~$10 |
| Breadboard + jumpers | Any | ~$5 |
| **Total** | | **~$46** |

---

## 6. Hardware Checklist (Before Any Experiment)

- [ ] Board powered and ST-LINK detected (LED green)
- [ ] UART detected on host: `COM3` or `/dev/ttyACM0`
- [ ] MPU6050 detected on I²C: `python scripts/build/scan_i2c.py` shows `0x68`
- [ ] INA219 detected on I²C: `python scripts/build/scan_i2c.py` shows `0x40`
- [ ] Logic analyzer connected and channels labeled
- [ ] Shunt resistance measured and updated in YAML
- [ ] DWT validation passed (tolerance ≤ 1 µs)
- [ ] Board and sensor connections documented in `meeting_notes/YYYY-MM-DD_*.md`
