import argparse
import pandas as pd
from pathlib import Path


def parse_args():
    parser = argparse.ArgumentParser(description="Generate LaTeX tables from metrics")
    parser.add_argument(
        "--metrics", required=True, help="Directory containing aggregated_results.csv"
    )
    parser.add_argument("--output", required=True, help="Output directory for tables")
    return parser.parse_args()


def generate_performance_table(df: pd.DataFrame, out_dir: Path):
    metrics = [
        "avg_response_time",
        "deadline_misses",
        "context_switches",
        "estimated_energy_uj",
    ]
    available_metrics = [m for m in metrics if m in df.columns]

    if not available_metrics:
        return

    summary = df.groupby("policy")[available_metrics].mean().round(2)

    # Format to LaTeX
    latex = summary.to_latex(
        caption="Average Performance Metrics per Scheduler Policy",
        label="tab:performance_metrics",
        column_format="l" + "c" * len(available_metrics),
    )

    with open(out_dir / "table1_performance.tex", "w") as f:
        f.write(latex)


def generate_workload_table(df: pd.DataFrame, out_dir: Path):
    if "workload_type" not in df.columns:
        return

    summary = (
        df.groupby(["workload_type", "policy"])["deadline_misses"]
        .mean()
        .unstack()
        .round(2)
    )

    latex = summary.to_latex(
        caption="Average Deadline Misses by Workload Type and Policy",
        label="tab:workload_misses",
        column_format="l" + "c" * len(summary.columns),
    )

    with open(out_dir / "table2_workloads.tex", "w") as f:
        f.write(latex)


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
    if "policy" in df.columns and df["policy"].dtype in [int, float]:
        df["policy"] = df["policy"].map(policy_map).fillna(df["policy"])

    generate_performance_table(df, out_dir)
    generate_workload_table(df, out_dir)
    print(f"Generated LaTeX tables in {out_dir}")


if __name__ == "__main__":
    main()
