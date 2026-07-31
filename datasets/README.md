# Datasets

This directory contains dataset metadata and acquisition scripts.

**Policy: No large binary files in git.**

Raw dataset files are in `*/data/` which is gitignored.
Download datasets using the scripts in `scripts/datasets/`.

---

## Registered Datasets

| Dataset | Task | Source | Size | License | Status |
|---|---|---|---|---|---|
| `kws/` | Keyword Spotting | Google Speech Commands v0.02 | ~2.3 GB | CC BY 4.0 | ✅ Scripted |
| `anomaly/` | Anomaly Detection | Synthetic (SchedTiny) | ~10 MB | Apache 2.0 | ✅ Scripted |
| `har/` | Human Activity Recognition | ST HAR Dataset | ~50 MB | ST License | 🔄 Planned |

---

## Acquiring Datasets

```bash
# KWS — Google Speech Commands
python scripts/datasets/download_kws.py --output datasets/kws/data/

# Anomaly — capture from board (requires hardware connected)
python scripts/datasets/capture_sensor.py --output datasets/anomaly/data/
```

See `docs/DATASET_GUIDE.md` for full instructions.

---

## Data Policy

All datasets used in the paper must have:
1. A public, citable source with DOI or official URL
2. A license that permits research publication
3. An SHA256 checksum committed in `*/README.md`
4. A download script in `scripts/datasets/`
