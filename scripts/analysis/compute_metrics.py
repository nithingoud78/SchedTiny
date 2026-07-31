#!/usr/bin/env python3
"""
compute_metrics.py — Compute SchedTiny benchmark metrics from parsed UART log.

Reads the CSV produced by parse_uart_log.py and computes all metrics defined in
docs/SPEC.md REQ-MEAS-001 through REQ-MEAS-005:

    - Inference latency: min, max, mean, std, p95, p99 (µs)
    - Interrupt-induced jitter: per-source mean, std, max (µs)
    - Deadline miss rate: per task (%)
    - CPU utilization: per task (%)

Outputs a JSON file in results/<EXP>/processed/ for downstream plotting.

Usage:
    python compute_metrics.py \\
        --input  results/EXP-001/raw/run_001.csv \\
        --output results/EXP-001/processed/metrics_run_001.json \\
        --sysclk_hz 480000000
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import numpy as np
import pandas as pd

# ---------------------------------------------------------------------------
# Event type constants (must match firmware bench_measure.h BenchEvent_t)
# ---------------------------------------------------------------------------
EV_TASK_START = "TASK_START"
EV_TASK_END = "TASK_END"
EV_INFER_START = "INFER_START"
EV_INFER_END = "INFER_END"
EV_ISR_ENTRY = "ISR_ENTRY"
EV_ISR_EXIT = "ISR_EXIT"
EV_DEADLINE_MISS = "DEADLINE_MISS"

TASK_ID_INFER = 0
TASK_ID_PID = 1
TASK_ID_SENSOR = 2


# ---------------------------------------------------------------------------
# Core metric computation functions
# ---------------------------------------------------------------------------


def compute_latency_stats(df: pd.DataFrame) -> dict:
    """Compute inference latency statistics.

    Pairs INFER_START and INFER_END events by sequential matching.
    Assumes no nested inference calls (single-threaded TFLM).

    Args:
        df: Parsed UART log DataFrame.

    Returns:
        Dict with keys: count, min_us, max_us, mean_us, std_us, p95_us, p99_us.
    """
    starts = df[df["event_type"] == EV_INFER_START]["timestamp_us"].to_numpy()
    ends = df[df["event_type"] == EV_INFER_END]["timestamp_us"].to_numpy()

    n = min(len(starts), len(ends))
    if n == 0:
        return {"count": 0, "error": "No INFER_START/END pairs found"}

    if len(starts) != len(ends):
        print(
            f"WARNING: {len(starts)} INFER_START vs {len(ends)} INFER_END events. "
            f"Using {n} matched pairs.",
            file=sys.stderr,
        )

    latencies = ends[:n] - starts[:n]
    # Sanity check: negative latencies indicate timestamp corruption
    negative_count = int((latencies < 0).sum())
    if negative_count > 0:
        print(
            f"WARNING: {negative_count} negative latency samples. "
            "Firmware DWT overflow handling may be incorrect.",
            file=sys.stderr,
        )
        latencies = latencies[latencies >= 0]

    return {
        "count": int(len(latencies)),
        "min_us": float(np.min(latencies)),
        "max_us": float(np.max(latencies)),
        "mean_us": float(np.mean(latencies)),
        "std_us": float(np.std(latencies)),
        "p95_us": float(np.percentile(latencies, 95)),
        "p99_us": float(np.percentile(latencies, 99)),
    }


def compute_jitter_stats(df: pd.DataFrame) -> dict:
    """Compute interrupt-induced jitter statistics per ISR source.

    Pairs ISR_ENTRY and ISR_EXIT events per source (stored in task_id field).

    Args:
        df: Parsed UART log DataFrame.

    Returns:
        Dict keyed by ISR source ID with jitter stats.
    """
    isr_df = df[df["event_type"].isin([EV_ISR_ENTRY, EV_ISR_EXIT])].copy()
    results: dict = {}

    for source_id in isr_df["task_id"].unique():
        source_df = isr_df[isr_df["task_id"] == source_id]
        entries = source_df[source_df["event_type"] == EV_ISR_ENTRY][
            "timestamp_us"
        ].to_numpy()
        exits = source_df[source_df["event_type"] == EV_ISR_EXIT][
            "timestamp_us"
        ].to_numpy()

        n = min(len(entries), len(exits))
        if n == 0:
            continue

        durations = exits[:n] - entries[:n]
        durations = durations[durations >= 0]

        results[f"isr_source_{source_id}"] = {
            "count": int(len(durations)),
            "mean_us": float(np.mean(durations)),
            "std_us": float(np.std(durations)),
            "max_us": float(np.max(durations)),
        }

    return results


def compute_deadline_miss_rate(df: pd.DataFrame) -> dict:
    """Compute deadline miss rate per task.

    Args:
        df: Parsed UART log DataFrame.

    Returns:
        Dict keyed by task_id with miss_count and miss_rate_percent.
    """
    miss_df = df[df["event_type"] == EV_DEADLINE_MISS]
    task_starts = df[df["event_type"] == EV_TASK_START]

    results: dict = {}
    for task_id in task_starts["task_id"].unique():
        total = int((task_starts["task_id"] == task_id).sum())
        misses = int((miss_df["task_id"] == task_id).sum())
        results[f"task_{task_id}"] = {
            "total_executions": total,
            "miss_count": misses,
            "miss_rate_percent": (float(misses / total * 100) if total > 0 else 0.0),
        }

    return results


def compute_cpu_utilization(df: pd.DataFrame) -> dict:
    """Compute approximate CPU utilization per task.

    Uses TASK_START / TASK_END pairs to compute total execution time per task.

    Args:
        df: Parsed UART log DataFrame.

    Returns:
        Dict keyed by task_id with total_us and utilization_percent.
    """
    observation_span = float(df["timestamp_us"].max() - df["timestamp_us"].min())
    if observation_span <= 0:
        return {"error": "Zero observation span"}

    results: dict = {}
    task_df = df[df["event_type"].isin([EV_TASK_START, EV_TASK_END])]

    for task_id in task_df["task_id"].unique():
        t_df = task_df[task_df["task_id"] == task_id]
        starts = t_df[t_df["event_type"] == EV_TASK_START]["timestamp_us"].to_numpy()
        ends = t_df[t_df["event_type"] == EV_TASK_END]["timestamp_us"].to_numpy()
        n = min(len(starts), len(ends))
        total_us = float(np.sum(ends[:n] - starts[:n]))
        results[f"task_{task_id}"] = {
            "total_us": total_us,
            "utilization_percent": total_us / observation_span * 100.0,
        }

    return results


# ---------------------------------------------------------------------------
# Argument parsing and entry point
# ---------------------------------------------------------------------------


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--input", type=Path, required=True, help="Path to parsed UART log CSV"
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Path to output metrics JSON file",
    )
    parser.add_argument(
        "--sysclk_hz",
        type=int,
        default=480_000_000,
        help="Target board SYSCLK in Hz (default: 480 MHz)",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if not args.input.exists():
        print(f"ERROR: Input file not found: {args.input}", file=sys.stderr)
        return 1

    df = pd.read_csv(args.input)

    metrics = {
        "source_file": str(args.input),
        "sysclk_hz": args.sysclk_hz,
        "total_records": len(df),
        "latency": compute_latency_stats(df),
        "jitter": compute_jitter_stats(df),
        "deadline_miss": compute_deadline_miss_rate(df),
        "cpu_util": compute_cpu_utilization(df),
    }

    args.output.parent.mkdir(parents=True, exist_ok=True)
    with args.output.open("w") as fh:
        json.dump(metrics, fh, indent=2)

    print(f"Metrics written to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
