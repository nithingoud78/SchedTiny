#!/usr/bin/env python3
"""
trace_statistics.py — Generate statistical summary for SchedTiny traces.
"""

from __future__ import annotations

import pandas as pd
from pathlib import Path


def generate_statistics(df: pd.DataFrame, output_path: Path) -> None:
    """Generate and save trace statistics."""
    if df.empty:
        print("Empty DataFrame, skipping statistics.")
        return

    stats = []
    stats.append("Trace Statistics Summary")
    stats.append("========================")

    total_events = len(df)
    stats.append(f"Total Events: {total_events}")

    context_switches = len(df[df["Event"] == "CONTEXT_SWITCH"])
    stats.append(f"Context Switches: {context_switches}")

    deadline_misses = len(df[df["Event"] == "DEADLINE_MISS"])
    stats.append(f"Deadline Misses: {deadline_misses}")

    faults = len(df[df["Event"] == "FAULT_TRIGGERED"])
    stats.append(f"Faults Triggered: {faults}")

    mc_switches = len(df[df["Event"] == "MC_MODE_SWITCH"])
    stats.append(f"MC Mode Switches: {mc_switches}")

    adaptive_switches = len(df[df["Event"] == "ADAPTIVE_POLICY_CHANGE"])
    stats.append(f"Adaptive Policy Changes: {adaptive_switches}")

    stats_str = "\n".join(stats)
    print(stats_str)

    output_path.parent.mkdir(parents=True, exist_ok=True)
    with open(output_path, "w") as f:
        f.write(stats_str)
    print(f"Statistics saved to {output_path}")
