#!/usr/bin/env python3
"""
plot_figures.py — Generate journal-quality figures from SchedTiny metrics.

Produces PDF/SVG figures suitable for IEEE journal submission from the JSON
metrics files in results/<EXP>/processed/.

Figure standards (see docs/EXPERIMENT_GUIDE.md Section 9):
    - Format: PDF (vector)
    - Font size: 10pt minimum for labels, 8pt for ticks
    - Colorblind-safe palette (tab10)
    - Line width: 1.5pt minimum
    - Single-column width: 3.5 inches
    - Double-column width: 7.16 inches

Usage:
    python plot_figures.py \\
        --metrics results/EXP-003/processed/ \\
        --output  results/EXP-003/figures/ \\
        --exp-id  EXP-003
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path
from typing import Any

import matplotlib

matplotlib.use("Agg")  # Non-interactive backend for CI environments
import matplotlib.pyplot as plt  # noqa: E402
import matplotlib.ticker as ticker  # noqa: E402
import numpy as np  # noqa: E402

# ---------------------------------------------------------------------------
# Style configuration (journal-quality settings)
# ---------------------------------------------------------------------------

JOURNAL_STYLE: dict[str, Any] = {
    "figure.dpi": 300,
    "font.family": "serif",
    "font.size": 10,
    "axes.labelsize": 10,
    "axes.titlesize": 10,
    "xtick.labelsize": 8,
    "ytick.labelsize": 8,
    "legend.fontsize": 8,
    "lines.linewidth": 1.5,
    "lines.markersize": 4,
    "axes.linewidth": 0.8,
    "grid.linewidth": 0.5,
    "grid.alpha": 0.4,
    "axes.grid": True,
    "axes.spines.top": False,
    "axes.spines.right": False,
    "savefig.bbox": "tight",
    "savefig.pad_inches": 0.05,
}

SINGLE_COL_WIDTH = 3.5  # IEEE single-column width in inches
DOUBLE_COL_WIDTH = 7.16  # IEEE double-column width in inches
ASPECT_RATIO = 0.618  # Golden ratio for height


# ---------------------------------------------------------------------------
# Figure generators
# ---------------------------------------------------------------------------


def plot_latency_cdf(
    latency_data: dict[str, list[float]],
    output_path: Path,
    exp_id: str,
) -> None:
    """Plot empirical CDF of inference latency for multiple conditions.

    Args:
        latency_data: Dict mapping condition label to list of latency samples (µs).
        output_path:  Output PDF file path.
        exp_id:       Experiment ID for the figure title.
    """
    with plt.rc_context(JOURNAL_STYLE):  # type: ignore[arg-type]
        fig, ax = plt.subplots(
            figsize=(SINGLE_COL_WIDTH, SINGLE_COL_WIDTH * ASPECT_RATIO)
        )

        colors = plt.get_cmap("tab10").colors  # type: ignore[attr-defined]

        for i, (label, samples) in enumerate(latency_data.items()):
            samples_sorted = np.sort(samples)
            cdf = np.arange(1, len(samples_sorted) + 1) / len(samples_sorted)
            ax.plot(samples_sorted, cdf, label=label, color=colors[i % 10])

        ax.set_xlabel("Inference Latency (µs)")
        ax.set_ylabel("Empirical CDF")
        ax.set_ylim(0, 1.02)
        ax.set_title(f"{exp_id}: Inference Latency CDF")
        ax.legend(loc="lower right", framealpha=0.9)
        ax.xaxis.set_major_formatter(ticker.FuncFormatter(lambda x, _: f"{x:.0f}"))

        fig.savefig(output_path, format="pdf")
        plt.close(fig)

    print(f"  Saved: {output_path}")


def plot_jitter_boxplot(
    jitter_data: dict[str, list[float]],
    output_path: Path,
    exp_id: str,
) -> None:
    """Box plot of ISR-induced latency jitter per condition.

    Args:
        jitter_data: Dict mapping condition label to list of jitter samples (µs).
        output_path: Output PDF file path.
        exp_id:      Experiment ID for the figure title.
    """
    with plt.rc_context(JOURNAL_STYLE):  # type: ignore[arg-type]
        fig, ax = plt.subplots(
            figsize=(SINGLE_COL_WIDTH, SINGLE_COL_WIDTH * ASPECT_RATIO)
        )

        labels = list(jitter_data.keys())
        data = list(jitter_data.values())

        bp = ax.boxplot(
            data,
            tick_labels=labels,
            patch_artist=True,
            medianprops={"color": "black", "linewidth": 1.5},
            whiskerprops={"linewidth": 1.0},
            flierprops={"marker": ".", "markersize": 2, "alpha": 0.5},
        )

        colors = plt.get_cmap("tab10").colors  # type: ignore[attr-defined]
        for patch, color in zip(bp["boxes"], colors):
            patch.set_facecolor(color)
            patch.set_alpha(0.7)

        ax.set_xlabel("Condition")
        ax.set_ylabel("ISR-Induced Jitter (µs)")
        ax.set_title(f"{exp_id}: Interrupt-Induced Latency Jitter")

        fig.savefig(output_path, format="pdf")
        plt.close(fig)

    print(f"  Saved: {output_path}")


def plot_miss_rate_bar(
    miss_data: dict[str, float],
    output_path: Path,
    exp_id: str,
) -> None:
    """Bar chart of deadline miss rates across conditions.

    Args:
        miss_data:   Dict mapping condition label to miss rate (%).
        output_path: Output PDF file path.
        exp_id:      Experiment ID.
    """
    with plt.rc_context(JOURNAL_STYLE):  # type: ignore[arg-type]
        fig, ax = plt.subplots(
            figsize=(SINGLE_COL_WIDTH, SINGLE_COL_WIDTH * ASPECT_RATIO)
        )

        labels = list(miss_data.keys())
        values = list(miss_data.values())
        colors = plt.get_cmap("tab10").colors  # type: ignore[attr-defined]

        ax.bar(labels, values, color=colors[: len(labels)], alpha=0.85)
        ax.set_xlabel("Condition")
        ax.set_ylabel("Deadline Miss Rate (%)")
        ax.set_title(f"{exp_id}: PID Control Deadline Miss Rate")
        ax.set_ylim(0, max(max(values) * 1.2, 1.0))

        fig.savefig(output_path, format="pdf")
        plt.close(fig)

    print(f"  Saved: {output_path}")


# ---------------------------------------------------------------------------
# Argument parsing and entry point
# ---------------------------------------------------------------------------


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--metrics",
        type=Path,
        required=True,
        help="Path to directory containing processed metrics JSON files",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Path to output figures directory",
    )
    parser.add_argument(
        "--exp-id",
        type=str,
        default="EXP-XXX",
        help="Experiment ID for figure titles",
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()

    if not args.metrics.exists():
        print(f"ERROR: Metrics directory not found: {args.metrics}", file=sys.stderr)
        return 1

    args.output.mkdir(parents=True, exist_ok=True)

    # Load all metrics JSON files from the processed directory
    metric_files = sorted(args.metrics.glob("metrics_*.json"))
    if not metric_files:
        print(
            f"ERROR: No metrics_*.json files found in {args.metrics}", file=sys.stderr
        )
        return 1

    print(f"Generating figures for {args.exp_id} from {len(metric_files)} run(s)...")

    # For multi-run experiments: aggregate across runs
    # For single-run experiments: use first file only
    all_metrics = []
    for f in metric_files:
        with f.open() as fh:
            all_metrics.append(json.load(fh))

    # Example: plot p99 latency per run as a simple demonstration
    # Real experiments will call specific plot functions with experiment-specific data
    p99_values = [m["latency"].get("p99_us", 0) for m in all_metrics]
    run_labels = [f"Run {i + 1}" for i in range(len(p99_values))]

    plot_miss_rate_bar(
        {label: v for label, v in zip(run_labels, p99_values)},
        args.output / "fig_p99_latency_per_run.pdf",
        args.exp_id,
    )

    print(f"\nAll figures saved to {args.output}")
    return 0


if __name__ == "__main__":
    sys.exit(main())
