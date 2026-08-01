#!/usr/bin/env python3
"""
train_scheduler_model.py
Trains a lightweight Decision Tree classifier for SchedTiny's adaptive scheduler
using the dynamically generated aggregated benchmark results.
"""

import argparse
import sys
from pathlib import Path
import pandas as pd
import numpy as np
from sklearn.tree import DecisionTreeClassifier
from sklearn.model_selection import train_test_split, StratifiedKFold
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score
import pickle


def load_dataset(results_dir: Path) -> pd.DataFrame:
    csv_file = results_dir / "aggregated_results.csv"
    if not csv_file.exists():
        print(f"Error: Could not find {csv_file}")
        sys.exit(1)
    df = pd.read_csv(csv_file)
    return df


def prepare_training_data(df: pd.DataFrame):
    # Filter out ADAPTIVE rows because we train ON the base policies to mimic the best one
    df_base = df[df["policy"] != 4].copy()  # Assuming 4 = ADAPTIVE
    if df_base.empty:
        df_base = df[df["policy"] != "Adaptive"].copy()

    if df_base.empty:
        print("Warning: No base policy data found to train on.")
        return None, None, None

    # Group by [workload_type, utilization, seed] to find the winning policy
    groups = df_base.groupby(["workload_type", "utilization", "seed"])

    feature_matrix = []
    labels = []

    for name, group in groups:
        # Sort by deadline misses (ascending), then energy (ascending)
        best = group.sort_values(
            by=["deadline_misses", "estimated_energy_uj"], ascending=[True, True]
        ).iloc[0]

        target_policy = best["policy"]
        if isinstance(target_policy, str):
            policy_map = {"HPF": 0, "EDF": 1, "RMS": 2, "MC": 3}
            target = policy_map.get(target_policy, 0)
        else:
            target = int(target_policy)

        # Features should be normalized to basis points (0-10000) for embedded C
        # In actual firmware, features are populated before calling the scheduler.
        # We approximate them based on the best policy's run (since workload
        # parameters are invariant across policies).

        util = best["utilization"]
        tasks = best["total_tasks"]

        f_vec = [
            min(10000, int(util * 10000)),  # cpu_utilization_bp
            int(tasks),  # task_count
            int(tasks),  # ready_queue_length (proxy)
            int(best["avg_response_time"]),  # avg_response_time
            int(best["avg_response_time"] * 0.7),  # avg_waiting_time (proxy)
            min(
                10000, int((best["deadline_misses"] / max(1, tasks * 10)) * 10000)
            ),  # deadline_miss_rate_bp
            min(
                10000, int((best["context_switches"] / 100000) * 10000)
            ),  # context_switch_rate_bp
            0,  # idle_time (omitted for simplicity)
            100000,  # busy_time (ticks proxy)
            0,  # fault_injection_rate_bp
            0,  # recovery_success_rate_bp
            int(best["estimated_energy_uj"]),  # energy_consumption
            int(best["estimated_energy_uj"] / 10),  # avg_power
            int(tasks * 0.3),  # criticality_hi_count
            3000,  # hi_criticality_ratio_bp
            0,  # mode_switch_frequency
        ]
        feature_matrix.append(f_vec)
        labels.append(target)

    columns = [
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
    ]

    X = pd.DataFrame(feature_matrix, columns=columns)
    y = pd.Series(labels)
    target_names = ["HPF", "EDF", "RMS", "MC"]

    return X, y, target_names


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
    if X is None or len(X) == 0:
        print("Failed to prepare dataset.")
        return

    print(
        f"\nTraining on {len(X)} unique workload states with {X.shape[1]} features..."
    )

    # Define model
    clf = DecisionTreeClassifier(max_depth=6, random_state=42)

    # 5-Fold Stratified CV
    skf = StratifiedKFold(n_splits=5, shuffle=True, random_state=42)
    acc_scores, f1_scores = [], []

    # Need at least 5 instances per class for 5-fold, if not, skip CV or reduce splits.
    try:
        for train_idx, val_idx in skf.split(X, y):
            X_tr, X_val = X.iloc[train_idx], X.iloc[val_idx]
            y_tr, y_val = y.iloc[train_idx], y.iloc[val_idx]

            clf.fit(X_tr, y_tr)
            preds = clf.predict(X_val)
            acc_scores.append(accuracy_score(y_val, preds))
            f1_scores.append(
                f1_score(y_val, preds, average="weighted", zero_division=0)
            )

        acc_mean = np.mean(acc_scores) * 100
        acc_std = np.std(acc_scores) * 100
        print(f"Cross-Validation Accuracy: {acc_mean:.2f}% (+/- {acc_std:.2f}%)")
        print(f"Cross-Validation F1-Score: {np.mean(f1_scores):.4f}")
    except ValueError:
        print("Dataset too small for 5-Fold CV, skipping cross-validation.")

    # Train final model on full Train/Test split for export
    X_train, X_test, y_train, y_test = train_test_split(
        X, y, test_size=0.2, random_state=42
    )
    clf.fit(X_train, y_train)

    y_pred = clf.predict(X_test)
    final_acc = accuracy_score(y_test, y_pred)
    final_prec = precision_score(y_test, y_pred, average="weighted", zero_division=0)
    final_rec = recall_score(y_test, y_pred, average="weighted", zero_division=0)
    final_f1 = f1_score(y_test, y_pred, average="weighted", zero_division=0)

    print("\nFinal Test Set Metrics:")
    print(f"  Accuracy:  {final_acc*100:.2f}%")
    print(f"  Precision: {final_prec:.4f}")
    print(f"  Recall:    {final_rec:.4f}")
    print(f"  F1 Score:  {final_f1:.4f}")

    # Save the model and metrics
    model_path = results_dir / "adaptive_model.pkl"
    with open(model_path, "wb") as f:
        pickle.dump(
            {
                "clf": clf,
                "features": X.columns.tolist(),
                "target_names": target_names,
                "metrics": {
                    "accuracy": final_acc,
                    "precision": final_prec,
                    "recall": final_rec,
                    "f1": final_f1,
                },
            },
            f,
        )

    print(f"\nModel saved to {model_path}")


if __name__ == "__main__":
    main()
