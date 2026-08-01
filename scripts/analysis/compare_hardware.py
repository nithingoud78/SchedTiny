#!/usr/bin/env python3
"""
compare_hardware.py
Compares simulation results against physical/simulated hardware platform results.
Calculates Absolute Error, Percentage Error, RMSE, and MAPE.
"""

import argparse
import json
from pathlib import Path
import pandas as pd
import numpy as np


def compute_metrics(sim_vals: np.ndarray, hw_vals: np.ndarray) -> dict:
    abs_err = np.abs(sim_vals - hw_vals)
    pct_err = np.where(hw_vals != 0, (abs_err / hw_vals) * 100.0, 0.0)
    rmse = np.sqrt(np.mean((sim_vals - hw_vals) ** 2))
    mape = np.mean(pct_err)

    return {
        "mean_absolute_error": float(np.mean(abs_err)),
        "max_absolute_error": float(np.max(abs_err)),
        "rmse": float(rmse),
        "mape": float(mape),
    }


def main():
    parser = argparse.ArgumentParser(description="Compare Sim vs HW Results")
    parser.add_argument(
        "--sim-results", type=str, required=True, help="Path to simulation JSON"
    )
    parser.add_argument(
        "--hw-results", type=str, required=True, help="Path to hardware JSON"
    )
    parser.add_argument(
        "--out-dir", type=str, required=True, help="Path to output directory"
    )
    args = parser.parse_args()

    with open(args.sim_results, "r") as f:
        sim_data = json.load(f)

    with open(args.hw_results, "r") as f:
        hw_data = json.load(f)

    df_sim = pd.DataFrame(sim_data)
    df_hw = pd.DataFrame(hw_data)

    # Correlate on Algorithm and Tasks
    merged = pd.merge(
        df_sim, df_hw, on=["Algorithm", "Tasks"], suffixes=("_sim", "_hw")
    )

    if merged.empty:
        print("Warning: No matching Algorithm/Task configurations found to compare.")
        return

    results = {}
    target_columns = [
        ("AvgResponseTime", "response_time"),
        ("DeadlineMisses", "deadline_misses"),
        ("Latency", "scheduling_latency"),
        ("Energy_uJ", "energy_uj"),
    ]

    for col_name, metric_key in target_columns:
        if f"{col_name}_sim" in merged and f"{col_name}_hw" in merged:
            sim_arr = merged[f"{col_name}_sim"].to_numpy(dtype=float)
            hw_arr = merged[f"{col_name}_hw"].to_numpy(dtype=float)
            results[metric_key] = compute_metrics(sim_arr, hw_arr)

    out_path = Path(args.out_dir)
    out_path.mkdir(parents=True, exist_ok=True)

    with open(out_path / "hardware_validation_metrics.json", "w") as f:
        json.dump(results, f, indent=4)

    print(
        f"Hardware validation metrics exported to {out_path / 'hardware_validation_metrics.json'}"
    )


if __name__ == "__main__":
    main()
