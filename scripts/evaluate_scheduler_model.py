#!/usr/bin/env python3
"""
evaluate_scheduler_model.py
Evaluates the trained decision tree model and generates visualizations.
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
    unique_labels = np.unique(np.concatenate((y_true, y_pred)))
    target_names_subset = [target_names[i] for i in unique_labels]

    disp = ConfusionMatrixDisplay(
        confusion_matrix=cm, display_labels=target_names_subset
    )
    fig, ax = plt.subplots(figsize=(6, 5))
    disp.plot(cmap=plt.cm.Blues, ax=ax)
    plt.title("Adaptive Scheduler Confusion Matrix")
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def plot_feature_importance(importances, feature_names, out_path):
    indices = np.argsort(importances)[::-1]
    non_zero = sum(importances > 0)
    indices = indices[:non_zero]

    plt.figure(figsize=(8, 5))
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
    plt.figure(figsize=(16, 8))
    plot_tree(
        clf,
        feature_names=feature_names,
        class_names=target_names,
        filled=True,
        rounded=True,
        fontsize=8,
    )
    plt.title("Decision Tree for Adaptive Scheduling")
    plt.tight_layout()
    plt.savefig(out_path)
    plt.close()


def calculate_footprint(clf):
    # A decision tree node in C typically requires:
    # 4 bytes for threshold (int32)
    # 4 bytes for feature index (int32)
    # 4 bytes for left child / right child offset or return value
    # Roughly 12 bytes per node.
    num_nodes = clf.tree_.node_count
    flash_bytes = num_nodes * 12
    ram_bytes = 0  # Inference is in-place, static memory
    return flash_bytes, ram_bytes


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
    metrics = model_data.get("metrics", {})

    set_style()

    out_dir = results_dir / "figures"
    out_dir.mkdir(parents=True, exist_ok=True)

    from train_scheduler_model import load_dataset, prepare_training_data
    from sklearn.model_selection import train_test_split

    df = load_dataset(results_dir)
    X, y, _ = prepare_training_data(df)
    if X is None:
        return

    _, X_test, _, y_test = train_test_split(X, y, test_size=0.2, random_state=42)
    y_pred = clf.predict(X_test)

    print("Generating Visualizations...")

    cm_path = out_dir / "adaptive_confusion_matrix.pdf"
    plot_confusion_matrix(y_test, y_pred, target_names, cm_path)

    fi_path = out_dir / "adaptive_feature_importance.pdf"
    plot_feature_importance(clf.feature_importances_, features, fi_path)

    dt_path = out_dir / "adaptive_decision_tree.pdf"
    plot_decision_tree(clf, features, target_names, dt_path)

    # Calculate footprint
    flash_usage, ram_usage = calculate_footprint(clf)

    print("\nEvaluation Metrics:")
    if metrics:
        print(f"  Accuracy:  {metrics.get('accuracy', 0) * 100:.2f}%")
        print(f"  Precision: {metrics.get('precision', 0):.4f}")
        print(f"  Recall:    {metrics.get('recall', 0):.4f}")
        print(f"  F1 Score:  {metrics.get('f1', 0):.4f}")
    print(f"  Model Size (Flash): ~{flash_usage} bytes")
    print(f"  Model RAM:          ~{ram_usage} bytes")

    # Export metrics to JSON for the report generator
    import json

    metrics_out = {
        "accuracy": metrics.get("accuracy", 0),
        "precision": metrics.get("precision", 0),
        "recall": metrics.get("recall", 0),
        "f1": metrics.get("f1", 0),
        "flash_bytes": flash_usage,
        "ram_bytes": ram_usage,
        "node_count": clf.tree_.node_count,
    }
    with open(results_dir / "tinyml_metrics.json", "w") as f:
        json.dump(metrics_out, f, indent=4)


if __name__ == "__main__":
    main()
