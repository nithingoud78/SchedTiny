import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path


def get_latest_experiment_dir(root: Path) -> Path:
    latest_file = root / "results" / "latest.txt"
    if not latest_file.exists():
        raise FileNotFoundError("No experiments found. Run run_all.py first.")
    with open(latest_file, "r") as f:
        dirname = f.read().strip()
    return root / "results" / dirname


def set_style():
    # Use standard matplotlib style that looks clean and publication-ready
    plt.style.use("seaborn-v0_8-whitegrid")
    plt.rcParams.update(
        {
            "font.size": 12,
            "axes.labelsize": 14,
            "axes.titlesize": 16,
            "legend.fontsize": 12,
            "xtick.labelsize": 12,
            "ytick.labelsize": 12,
            "figure.dpi": 300,
        }
    )


def plot_metric(df, metric, title, ylabel, filename, figures_dir, log_scale=False):
    # We want grouped bar chart: X-axis = Profile, Hue = Algorithm
    profiles = ["small", "medium", "large", "stress"]
    algorithms = ["HPF", "EDF", "RMS"]

    # Filter df to ensure order
    df = df[df["Profile"].isin(profiles)]

    x = np.arange(len(profiles))
    width = 0.25

    fig, ax = plt.subplots(figsize=(10, 6))

    for i, algo in enumerate(algorithms):
        algo_data = df[df["Algorithm"] == algo]
        # Ensure data aligns with profiles
        means = []
        errs = []
        for p in profiles:
            row = algo_data[algo_data["Profile"] == p]
            if not row.empty:
                means.append(row[f"{metric}_mean"].values[0])
                errs.append(row[f"{metric}_ci95"].values[0])
            else:
                means.append(0)
                errs.append(0)

        offset = (i - 1) * width
        ax.bar(x + offset, means, width, label=algo, yerr=errs, capsize=5, alpha=0.9)

    ax.set_ylabel(ylabel)
    ax.set_title(title)
    ax.set_xticks(x)
    ax.set_xticklabels([p.capitalize() for p in profiles])
    ax.legend()

    if log_scale:
        ax.set_yscale("log")

    fig.tight_layout()

    for ext in ["png", "pdf", "svg"]:
        fig.savefig(figures_dir / f"{filename}.{ext}")
    plt.close(fig)


def main():
    root = Path(__file__).parent.parent.absolute()
    exp_dir = get_latest_experiment_dir(root)

    summary_csv = exp_dir / "summary.csv"
    if not summary_csv.exists():
        print("summary.csv not found. Run compare_algorithms.py first.")
        return

    df = pd.read_csv(summary_csv)
    figures_dir = exp_dir / "figures"
    figures_dir.mkdir(exist_ok=True)

    set_style()

    print(f"Generating figures in {figures_dir}...")

    # Figure 2: CPU Utilization
    plot_metric(
        df,
        "CPUUtilization",
        "CPU Utilization by Scheduler and Workload",
        "Utilization (%)",
        "figure2_cpu_utilization",
        figures_dir,
    )

    # Figure 3: Deadline Misses
    plot_metric(
        df,
        "DeadlineMisses",
        "Deadline Misses",
        "Misses (Count)",
        "figure3_deadline_misses",
        figures_dir,
    )

    # Figure 4: Context Switches
    plot_metric(
        df,
        "ContextSwitches",
        "Context Switches",
        "Switches (Count)",
        "figure4_context_switches",
        figures_dir,
        log_scale=True,
    )

    # Figure 5: Scheduler Latency
    plot_metric(
        df,
        "Latency",
        "Average Scheduler Latency",
        "Latency (ticks)",
        "figure5_scheduler_latency",
        figures_dir,
    )

    # Figure 6: Waiting Time
    plot_metric(
        df,
        "AvgWaitingTime",
        "Average Waiting Time",
        "Waiting Time (ticks)",
        "figure6_waiting_time",
        figures_dir,
    )

    # Figure 7: Response Time
    plot_metric(
        df,
        "AvgResponseTime",
        "Average Response Time",
        "Response Time (ticks)",
        "figure7_response_time",
        figures_dir,
    )

    # Figure 8: Throughput
    plot_metric(
        df,
        "Throughput",
        "System Throughput",
        "Tasks Completed per 10000 ticks",
        "figure8_throughput",
        figures_dir,
    )

    print("Figure generation complete.")


if __name__ == "__main__":
    main()
