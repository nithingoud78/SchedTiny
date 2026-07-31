# SchedTiny — Models

This directory contains TinyML model artifacts for SchedTiny benchmark workloads.

---

## Directory Structure

```
models/
├── kws/                    # Keyword Spotting (DS-CNN, INT8)
│   ├── model.tflite        # Quantized TFLite model (tracked via Git LFS or documented)
│   ├── model.cc            # C array for firmware embedding (generated)
│   ├── model.h             # Extern declaration for model.cc
│   ├── README.md           # Model provenance, accuracy, parameters
│   └── training/           # Python training scripts
└── anomaly/                # Anomaly Detection (MLP, INT8)
    ├── model.tflite
    ├── model.cc
    ├── model.h
    ├── README.md
    └── training/
```

---

## Model Versioning Policy

Every model file must have a corresponding `README.md` documenting:

| Field | Example |
|---|---|
| Model architecture | DS-CNN (7 layers, 64K parameters) |
| Input shape | [1, 49, 10, 1] (1s audio, 49 frames, 10 MFCCs) |
| Output classes | 10 (yes/no/up/down/left/right/on/off/stop/go) |
| Quantization | Post-training INT8, per-channel |
| Accuracy (float) | 95.2% on Speech Commands v0.02 test set |
| Accuracy (INT8) | 94.8% (−0.4% quantization degradation) |
| Model size | 60,432 bytes Flash |
| Arena size (min) | 248,832 bytes SRAM |
| Training script | `models/kws/training/train_ds_cnn.py` |
| Framework | TensorFlow 2.14, TFLM 2024-XX-XX commit SHA |
| Training dataset | Speech Commands v0.02 (SHA256 in datasets/kws/README.md) |

---

## Generating the C Array

After modifying a model, regenerate the C array:

```bash
python -c "
import tensorflow as tf
with open('models/kws/model.tflite', 'rb') as f:
    data = f.read()
print('// AUTO-GENERATED — do not edit manually')
print(f'// Source: models/kws/model.tflite')
print(f'// Size: {len(data)} bytes')
print(f'#include <stdint.h>')
print(f'const uint8_t g_kws_model_data[] = {{')
for i, b in enumerate(data):
    if i % 16 == 0: print('  ', end='')
    print(f'0x{b:02x}, ', end='')
    if (i + 1) % 16 == 0: print()
print('};')
print(f'const uint32_t g_kws_model_data_len = {len(data)};')
" > models/kws/model.cc
```

Or use `xxd -i model.tflite > model.cc` (Linux only).
