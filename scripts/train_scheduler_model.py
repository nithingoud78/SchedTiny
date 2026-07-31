#!/usr/bin/env python3
"""
train_scheduler_model.py
Trains a lightweight Decision Tree classifier for SchedTiny's adaptive scheduler
using benchmark results data.

Usage:
    python scripts/train_scheduler_model.py --results-dir results/latest
"""

import argparse
import sys
from pathlib import Path
import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier
from sklearn.model_selection import train_test_split
from sklearn.metrics import accuracy_score, classification_report
import pickle


def load_dataset(results_dir: Path) -> pd.DataFrame:
    csv_file = results_dir / "benchmark_summary.csv"
    if not csv_file.exists():
        print(f"Error: Could not find {csv_file}")
        sys.exit(1)

    df = pd.read_csv(csv_file)
    print(f"Loaded dataset from {csv_file} with {len(df)} records.")
    return df


def prepare_training_data(df: pd.DataFrame):
    # SchedTiny extracts 16 features. We map them from the CSV columns.
    # Note: For synthetic/mock training here, we might need to derive
    # some features if they aren't explicitly in the summary CSV, or
    # rely on the benchmark framework to export them.
    #
    # The actual firmware uses: cpu_utilization_bp, task_count, ready_queue_length,
    # avg_response_time, avg_waiting_time, deadline_miss_rate_bp,
    # context_switch_rate_bp, idle_time, busy_time, fault_injection_rate_bp,
    # recovery_success_rate_bp, energy_consumption, avg_power, criticality_hi_count,
    # hi_criticality_ratio_bp, mode_switch_frequency.

    # Some features might be missing in basic CSV, we'll use available ones
    # or synthesize them for the demo training.

    # In a real scenario, we'd group by workload profile, find the 'best' algorithm
    # (e.g. lowest misses, lowest energy) to act as the label, and use the features.
    # For this script, we'll create a mock target based on simple rules to demonstrate
    # the pipeline since we don't have labeled pairs.

    # 1. Filter out ADAPTIVE rows, we want to train ON the base policies
    df_base = df[df["Algorithm"] != "ADAPTIVE"].copy()
    if df_base.empty:
        print("Warning: No base policy data found to train on.")
        return None, None, None

    # We will synthetically label data based on domain knowledge for the pipeline:
    labels = []
    feature_matrix = []

    # We will map column values to 0-10000 basis points scale as the C struct expects
    for idx, row in df_base.iterrows():
        # Synthesize target label for demonstration of the training pipeline
        util = row["CPUUtilization"]
        misses = row["DeadlineMisses"]

        target = 0  # HPF
        if util > 80 and misses > 10:
            target = 1  # EDF
        elif util < 50:
            target = 2  # RMS

        labels.append(target)

        # Build basis point features
        f_vec = [
            int(row["CPUUtilization"] * 100),  # CPUUtil BP
            int(row["Tasks"]),  # Tasks
            int(row["Tasks"]),  # Ready queue proxy
            int(row["AvgResponseTime"]),
            int(row["AvgWaitingTime"]),
            min(
                10000, int((row["DeadlineMisses"] / (row["Tasks"] * 10 + 1)) * 10000)
            ),  # Miss rate BP
            min(
                10000, int((row["ContextSwitches"] / max(1, row["BusyTime"])) * 10000)
            ),  # CS rate BP
            int(row["IdleTime"]),
            int(row["BusyTime"]),
            0,  # Fault inj rate BP
            0,  # Rec success rate BP
            int(row["EstimatedEnergy"]),
            int(row["EstimatedPower"]),
            int(row["Tasks"] * 0.3),  # HI count proxy
            3000,  # HI ratio BP proxy
            int(row["ModeSwitches"]),
        ]
        feature_matrix.append(f_vec)

    X = pd.DataFrame(
        feature_matrix,
        columns=[
            "cpu_utilization_bp",
            "task_count",
            "ready_queue_length",
            "avg_response_time",
            "avg_waiting_time",
            "deadline_miss_rate_bp",
            "context_switch_rate_bp",
            "idle_time",
            "busy_time",
            "fault_injection_rate_bp",
            "recovery_success_rate_bp",
            "energy_consumption",
            "avg_power",
            "criticality_hi_count",
            "hi_criticality_ratio_bp",
            "mode_switch_frequency",
        ],
    )
    y = pd.Series(labels)

    return X, y, ["HPF", "EDF", "RMS", "MC"]


def main():
    parser = argparse.ArgumentParser(
        description="Train SchedTiny Adaptive Scheduler Model"
    )
    parser.add_argument(
        "--results-dir", type=str, required=True, help="Path to results directory"
    )
    args = parser.parse_args()

    results_dir = Path(args.results_dir)
    df = load_dataset(results_dir)

    X, y, target_names = prepare_training_data(df)
    if X is None:
        return

    print(f"\nTraining on {len(X)} samples with {X.shape[1]} features...")

    # We want a very shallow tree to fit in the embedded C code
    clf = DecisionTreeClassifier(max_depth=6, random_state=42)

    # Train test split
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
    )

    clf.fit(X_train, y_train)

    y_pred = clf.predict(X_test)
    acc = accuracy_score(y_test, y_pred)

    print(f"\nModel Accuracy: {acc * 100:.2f}%")
    print("\nClassification Report:")
    # Handle cases where some classes might not be present in test set
    unique_labels = np.unique(np.concatenate((y_test, y_pred)))
    target_names_subset = [target_names[i] for i in unique_labels]
    print(
        classification_report(
            y_test, y_pred, target_names=target_names_subset, zero_division=0
        )
    )

    print("\nFeature Importances:")
    importances = clf.feature_importances_
    for i, name in enumerate(X.columns):
        if importances[i] > 0:
            print(f"  {name}: {importances[i]:.4f}")

    # Save the model
    model_path = results_dir / "adaptive_model.pkl"
    with open(model_path, "wb") as f:
        pickle.dump(
            {
                "clf": clf,
                "features": X.columns.tolist(),
                "target_names": target_names,
                "accuracy": acc,
            },
            f,
        )

    print(f"\nModel saved to {model_path}")
    print("Run scripts/export_decision_tree.py to generate C header.")


if __name__ == "__main__":
    main()
