#!/usr/bin/env python3
"""
timeline_visualizer.py — Visualize SchedTiny execution timelines.
"""

from __future__ import annotations

import argparse
from pathlib import Path
from parse_uart_log import parse_log_file
from gantt_chart import plot_gantt_chart
from trace_statistics import generate_statistics


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Visualize SchedTiny execution timelines."
    )
    parser.add_argument(
        "--input", type=Path, required=True, help="Input JSON trace file"
    )
    parser.add_argument(
        "--output-dir",
        type=Path,
        required=True,
        help="Output directory for visualizations",
    )
    args = parser.parse_args()

    args.output_dir.mkdir(parents=True, exist_ok=True)

    print(f"Loading trace from {args.input}...")
    df = parse_log_file(args.input)

    if df.empty:
        print("Empty trace, nothing to visualize.")
        return

    gantt_path = args.output_dir / "timeline_gantt.png"
    plot_gantt_chart(df, gantt_path)

    stats_path = args.output_dir / "trace_statistics.txt"
    generate_statistics(df, stats_path)

    print("Visualization complete.")


if __name__ == "__main__":
    main()
