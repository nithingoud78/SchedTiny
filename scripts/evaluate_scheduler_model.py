#!/usr/bin/env python3
"""
evaluate_scheduler_model.py
Evaluates the trained decision tree model and generates visualizations.

Usage:
    python scripts/evaluate_scheduler_model.py --results-dir results/latest
"""

import argparse
import sys
from pathlib import Path
import pickle
import numpy as np
import matplotlib.pyplot as plt
from sklearn.metrics import confusion_matrix, ConfusionMatrixDisplay
from sklearn.tree import plot_tree


def set_style():
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


def plot_confusion_matrix(y_true, y_pred, target_names, out_path):
    cm = confusion_matrix(y_true, y_pred)
    # Filter target names if some are missing
    unique_labels = np.unique(np.concatenate((y_true, y_pred)))
    target_names_subset = [target_names[i] for i in unique_labels]

    disp = ConfusionMatrixDisplay(
        confusion_matrix=cm, display_labels=target_names_subset
    )
    fig, ax = plt.subplots(figsize=(8, 6))
    disp.plot(cmap=plt.cm.Blues, ax=ax)
    plt.title("Adaptive Scheduler Confusion Matrix")
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def plot_feature_importance(importances, feature_names, out_path):
    indices = np.argsort(importances)[::-1]

    # Filter out zero importance
    non_zero = sum(importances > 0)
    indices = indices[:non_zero]

    plt.figure(figsize=(10, 6))
    plt.title("Feature Importances")
    plt.bar(
        range(non_zero),
        importances[indices],
        align="center",
        alpha=0.8,
        color="steelblue",
    )
    plt.xticks(
        range(non_zero), [feature_names[i] for i in indices], rotation=45, ha="right"
    )
    plt.xlim([-1, non_zero])
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def plot_decision_tree(clf, feature_names, target_names, out_path):
    plt.figure(figsize=(20, 10))
    plot_tree(
        clf,
        feature_names=feature_names,
        class_names=target_names,
        filled=True,
        rounded=True,
        fontsize=10,
    )
    plt.title("Decision Tree for Adaptive Scheduling")
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def plot_selection_frequency(df_adaptive, out_path):
    # This plots the actual selections made by the firmware if we have adaptive logs
    pass


def main():
    parser = argparse.ArgumentParser(
        description="Evaluate SchedTiny Adaptive Scheduler Model"
    )
    parser.add_argument(
        "--results-dir", type=str, required=True, help="Path to results directory"
    )
    args = parser.parse_args()

    results_dir = Path(args.results_dir)
    model_path = results_dir / "adaptive_model.pkl"

    if not model_path.exists():
        print(f"Error: Model not found at {model_path}")
        sys.exit(1)

    with open(model_path, "rb") as f:
        model_data = pickle.load(f)

    clf = model_data["clf"]
    features = model_data["features"]
    target_names = model_data["target_names"]

    set_style()

    out_dir = results_dir / "figures"
    out_dir.mkdir(parents=True, exist_ok=True)

    # Generate mock test data based on the dataset to create the plots
    from train_scheduler_model import load_dataset, prepare_training_data
    from sklearn.model_selection import train_test_split

    df = load_dataset(results_dir)
    X, y, _ = prepare_training_data(df)
    if X is None:
        return

    _, X_test, _, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    y_pred = clf.predict(X_test)

    print("Generating Visualizations...")

    # 1. Confusion Matrix
    cm_path = out_dir / "adaptive_confusion_matrix.png"
    plot_confusion_matrix(y_test, y_pred, target_names, cm_path)
    print(f"Saved {cm_path}")

    # 2. Feature Importance
    fi_path = out_dir / "adaptive_feature_importance.png"
    plot_feature_importance(clf.feature_importances_, features, fi_path)
    print(f"Saved {fi_path}")

    # 3. Decision Tree
    dt_path = out_dir / "adaptive_decision_tree.png"
    plot_decision_tree(clf, features, target_names, dt_path)
    print(f"Saved {dt_path}")

    print("\nEvaluation Complete.")


if __name__ == "__main__":
    main()
