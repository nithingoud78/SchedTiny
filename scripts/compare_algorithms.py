import pandas as pd
import numpy as np
import scipy.stats as st  # type: ignore
from pathlib import Path


def get_latest_experiment_dir(root: Path) -> Path:
    latest_file = root / "results" / "latest.txt"
    if not latest_file.exists():
        raise FileNotFoundError("No experiments found. Run run_all.py first.")
    with open(latest_file, "r") as f:
        dirname = f.read().strip()
    return root / "results" / dirname


def compute_stats(group):
    # For a series of values, compute descriptive statistics
    stats = {}
    for col in [
        "CPUUtilization",
        "Latency",
        "DeadlineMisses",
        "ContextSwitches",
        "AvgResponseTime",
        "AvgWaitingTime",
        "Throughput",
        "IdleTime",
        "BusyTime",
        "Energy_uJ",
        "Power_uW",
        "EnergyPerTask_uJ",
        "EnergyPerCS_uJ",
    ]:
        data = group[col].dropna()
        if len(data) == 0:
            continue

        mean = data.mean()
        std = data.std() if len(data) > 1 else 0

        # 95% CI
        ci = 0
        if len(data) > 1 and std > 0:
            df_deg = len(data) - 1
            t_val = st.t.ppf((1 + 0.95) / 2.0, df_deg)
            ci = t_val * (std / np.sqrt(len(data)))

        stats[f"{col}_mean"] = mean
        stats[f"{col}_median"] = data.median()
        stats[f"{col}_min"] = data.min()
        stats[f"{col}_max"] = data.max()
        stats[f"{col}_std"] = std
        stats[f"{col}_var"] = data.var() if len(data) > 1 else 0
        stats[f"{col}_ci95"] = ci

    return pd.Series(stats)


def main():
    root = Path(__file__).parent.parent.absolute()
    exp_dir = get_latest_experiment_dir(root)

    # Load all CSVs
    all_dfs = []
    for f in exp_dir.glob("metrics_*.csv"):
        # filename format: metrics_{profile}_{seed}.csv
        parts = f.stem.split("_")
        profile = parts[1]
        seed = parts[2]

        df = pd.read_csv(f)
        df["Profile"] = profile
        df["Seed"] = seed
        all_dfs.append(df)

    if not all_dfs:
        print("No CSV files found in latest experiment.")
        return

    full_df = pd.concat(all_dfs, ignore_index=True)

    # Compute summary statistics grouped by Algorithm and Profile
    summary_df = (
        full_df.groupby(["Algorithm", "Profile"]).apply(compute_stats).reset_index()
    )

    # Export summary CSV
    summary_csv = exp_dir / "summary.csv"
    summary_df.to_csv(summary_csv, index=False)

    # Export summary JSON
    summary_json = exp_dir / "comparison.json"
    summary_df.to_json(summary_json, orient="records", indent=4)

    # Export summary Markdown
    summary_md = exp_dir / "comparison.md"
    with open(summary_md, "w") as f:
        f.write("# Algorithm Comparison\n\n")
        f.write(
            "This table summarizes the performance of HPF, EDF, "
            "and RMS across different workload profiles and seeds.\n\n"
        )

        # To make it readable, we'll pivot to show Mean (± CI) for key metrics
        f.write("## Key Metrics (Mean ± 95% CI)\n\n")

        # Select key columns for the markdown table
        md_table = pd.DataFrame()
        md_table["Algorithm"] = summary_df["Algorithm"]
        md_table["Profile"] = summary_df["Profile"]

        def format_metric(row, col):
            return f"{row[col + '_mean']:.2f} ± {row[col + '_ci95']:.2f}"

        for col in [
            "CPUUtilization",
            "Latency",
            "DeadlineMisses",
            "ContextSwitches",
            "AvgResponseTime",
            "AvgWaitingTime",
            "Throughput",
            "IdleTime",
            "BusyTime",
        ]:
            md_table[col] = summary_df.apply(
                lambda r, c=col: format_metric(r, c), axis=1
            )

        f.write(md_table.to_markdown(index=False))
        f.write("\n")

    print(f"Comparison complete. Output saved to {exp_dir}")


if __name__ == "__main__":
    main()
