# Dataset Guide

## Dataset Acquisition, Preprocessing, and Usage Policy for SchedTiny

---

## 1. Dataset Policy

**No large binary files in git.** The datasets used by SchedTiny are large (multi-GB)
and cannot be version-controlled. Instead:

- Raw dataset files live in `datasets/*/data/` (gitignored)
- Download scripts live in `scripts/datasets/`
- Dataset metadata (source, license, version, SHA256) is committed in `datasets/*/README.md`
- Preprocessed features (small, < 1 MB) MAY be committed if needed for tests

**Every dataset used in the paper must have:**
1. A clear public source (URL, DOI, or official dataset page)
2. A license that permits research use
3. An SHA256 checksum of the downloaded file, committed in `datasets/*/README.md`
4. A download script in `scripts/datasets/download_<name>.py`

---

## 2. Datasets Used

### 2.1 Google Speech Commands (KWS)

| Field | Value |
|---|---|
| **Task** | Keyword Spotting (KWS) |
| **Source** | https://www.tensorflow.org/datasets/catalog/speech_commands |
| **Version** | v0.02 |
| **Size** | ~2.3 GB |
| **License** | Creative Commons BY 4.0 |
| **Classes** | 35 keywords + unknown + silence |
| **Sample rate** | 16 kHz, 1-second WAV clips |
| **SchedTiny use** | 10-class subset (yes/no/up/down/left/right/on/off/stop/go) |

**Download:**
```bash
python scripts/datasets/download_kws.py --output datasets/kws/data/
```

**Preprocessing** (WAV → MFCC features → C array):
```bash
python scripts/datasets/preprocess.py \
       --dataset kws \
       --input datasets/kws/data/ \
       --output models/kws/features/
```

**Model:** Quantized DS-CNN (Depthwise Separable CNN), INT8, ~60 KB Flash.
Architecture based on Zhang et al. (2017). See `models/kws/README.md`.

---

### 2.2 Synthetic Anomaly Detection

| Field | Value |
|---|---|
| **Task** | Anomaly Detection |
| **Source** | Synthetically generated (see `scripts/datasets/generate_anomaly.py`) |
| **Version** | SchedTiny v0.1 |
| **Size** | ~10 MB |
| **License** | Apache 2.0 (this project) |
| **Classes** | Normal, Anomalous (binary) |
| **Features** | Accelerometer + gyroscope (MPU6050), 100 Hz, windows of 50 samples |

This dataset is generated on-the-fly from the MPU6050 sensor connected to the
NUCLEO board. It is NOT a public dataset — it is an experimental workload.

**Generate:**
```bash
# Capture normal operation data from board
python scripts/datasets/capture_sensor.py \
       --duration 300 \
       --label normal \
       --output datasets/anomaly/data/

# Inject anomalous patterns (simulated vibration)
python scripts/datasets/generate_anomaly.py \
       --normal datasets/anomaly/data/normal/ \
       --output datasets/anomaly/data/
```

**Model:** MLP with 2 hidden layers (32, 16 neurons), INT8 quantized, ~8 KB Flash.
See `models/anomaly/README.md`.

---

### 2.3 Human Activity Recognition (HAR) — Future

| Field | Value |
|---|---|
| **Task** | HAR (planned for future experiments) |
| **Source** | ST HAR dataset: https://www.st.com/en/embedded-software/stsw-stm32119.html |
| **Status** | 🔄 Not yet integrated |

---

## 3. Reproducibility Requirements

Before any experiment result can be included in the paper:

1. The dataset version and SHA256 must be recorded in `datasets/*/README.md`
2. The preprocessing script must be deterministic (same input → same output)
3. The model accuracy on the dataset must be within ±1% of the reported value
   across 3 independent training runs

---

## 4. Dataset Versioning

We use SHA256 checksums for integrity verification:

```bash
# After downloading, record the checksum
sha256sum datasets/kws/data/speech_commands_v0.02.tar.gz

# Output is added to datasets/kws/README.md:
# SHA256: 9b3fc1e585c65e09234e8073...
```

The CI pipeline verifies checksums before any analysis run.

---

## 5. Adding a New Dataset

To add a new dataset:

1. Create `datasets/<name>/README.md` with source, license, size, SHA256
2. Create `scripts/datasets/download_<name>.py` download script
3. Create `scripts/datasets/preprocess_<name>.py` if preprocessing is needed
4. Add SHA256 verification in `ci/check_datasets.py`
5. Update this file with the new dataset entry
6. Open a PR with label `dataset`
