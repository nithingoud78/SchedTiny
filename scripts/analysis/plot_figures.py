import argparse
import pandas as pd
import matplotlib.pyplot as plt
import seaborn as sns
import numpy as np
from pathlib import Path

# Journal Style Settings
plt.style.use("seaborn-v0_8-whitegrid")
plt.rcParams.update(
    {
        "figure.dpi": 300,
        "font.family": "serif",
        "font.size": 10,
        "axes.labelsize": 10,
        "axes.titlesize": 10,
        "xtick.labelsize": 8,
        "ytick.labelsize": 8,
        "legend.fontsize": 8,
        "lines.linewidth": 1.5,
        "savefig.bbox": "tight",
    }
)


def parse_args():
    parser = argparse.ArgumentParser(description="Plot publication-quality figures")
    parser.add_argument(
        "--metrics", required=True, help="Directory containing aggregated_results.csv"
    )
    parser.add_argument("--output", required=True, help="Output directory for figures")
    return parser.parse_args()


def plot_box_violin(df: pd.DataFrame, out_dir: Path):
    fig, ax = plt.subplots(figsize=(5, 3.5))
    sns.boxplot(data=df, x="policy", y="avg_response_time", ax=ax, showfliers=False)
    ax.set_ylabel("Avg Response Time (ticks)")
    ax.set_xlabel("Scheduler Policy")
    fig.savefig(out_dir / "fig1_response_time_box.pdf")
    plt.close(fig)

    fig, ax = plt.subplots(figsize=(5, 3.5))
    sns.violinplot(data=df, x="policy", y="max_scheduling_latency", ax=ax)
    ax.set_ylabel("Max Scheduling Latency (ticks)")
    ax.set_xlabel("Scheduler Policy")
    fig.savefig(out_dir / "fig2_latency_violin.pdf")
    plt.close(fig)


def plot_heatmap(df: pd.DataFrame, out_dir: Path):
    if "adaptive_switches" not in df.columns:
        return
    pivot = df.pivot_table(
        values="adaptive_switches",
        index="workload_type",
        columns="utilization",
        aggfunc="mean",
    )
    if not pivot.empty:
        fig, ax = plt.subplots(figsize=(4, 3))
        sns.heatmap(pivot, annot=True, fmt=".1f", cmap="YlGnBu", ax=ax)
        ax.set_title("Adaptive Mode Switches")
        fig.savefig(out_dir / "fig3_adaptive_switches_heatmap.pdf")
        plt.close(fig)


def plot_cdf(df: pd.DataFrame, out_dir: Path):
    fig, ax = plt.subplots(figsize=(5, 3.5))
    for policy in df["policy"].unique():
        data = df[df["policy"] == policy]["deadline_misses"].sort_values()
        cdf = np.arange(1, len(data) + 1) / len(data)
        ax.plot(data, cdf, label=str(policy))
    ax.set_xlabel("Deadline Misses")
    ax.set_ylabel("CDF")
    ax.legend()
    fig.savefig(out_dir / "fig4_deadline_misses_cdf.pdf")
    plt.close(fig)


def plot_scatter(df: pd.DataFrame, out_dir: Path):
    fig, ax = plt.subplots(figsize=(5, 3.5))
    sns.scatterplot(
        data=df,
        x="avg_response_time",
        y="estimated_energy_uj",
        hue="policy",
        style="workload_type",
        ax=ax,
    )
    ax.set_xlabel("Avg Response Time (ticks)")
    ax.set_ylabel("Estimated Energy (uJ)")
    fig.savefig(out_dir / "fig5_energy_vs_latency_scatter.pdf")
    plt.close(fig)


def plot_scalability(df: pd.DataFrame, out_dir: Path):
    if "total_tasks" in df.columns:
        fig, ax = plt.subplots(figsize=(5, 3.5))
        sns.lineplot(
            data=df,
            x="total_tasks",
            y="context_switches",
            hue="policy",
            marker="o",
            ax=ax,
        )
        ax.set_xlabel("Number of Tasks")
        ax.set_ylabel("Context Switches")
        fig.savefig(out_dir / "fig6_scalability.pdf")
        plt.close(fig)


def main():
    args = parse_args()
    metrics_dir = Path(args.metrics)
    out_dir = Path(args.output)
    out_dir.mkdir(parents=True, exist_ok=True)

    csv_file = metrics_dir / "aggregated_results.csv"
    if not csv_file.exists():
        print(f"File not found: {csv_file}")
        return

    df = pd.read_csv(csv_file)
    if df.empty:
        print("Dataset is empty.")
        return

    # Standardize policy names
    policy_map = {0: "HPF", 1: "EDF", 2: "RMS", 3: "MC", 4: "Adaptive"}
    if "policy" in df.columns and df["policy"].dtype in [
        np.int64,
        np.float64,
        int,
        float,
    ]:
        df["policy"] = df["policy"].map(policy_map).fillna(df["policy"])

    plot_box_violin(df, out_dir)
    plot_heatmap(df, out_dir)
    plot_cdf(df, out_dir)
    plot_scatter(df, out_dir)
    plot_scalability(df, out_dir)
    print(f"Generated figures in {out_dir}")


if __name__ == "__main__":
    main()
