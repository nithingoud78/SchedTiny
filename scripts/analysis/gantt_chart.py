#!/usr/bin/env python3
"""
gantt_chart.py — Generate publication-quality Gantt charts for SchedTiny traces.
"""

from __future__ import annotations

import pandas as pd
import matplotlib.pyplot as plt
import matplotlib.patches as mpatches
from pathlib import Path
from typing import Any


def plot_gantt_chart(df: pd.DataFrame, output_path: Path) -> None:
    """Plot an execution timeline Gantt chart."""
    if df.empty:
        print("Empty DataFrame, skipping Gantt chart.")
        return

    fig, ax = plt.subplots(figsize=(12, 6))

    tasks_arr = df["TaskID"].unique()
    valid_tasks = sorted([int(t) for t in tasks_arr if t > 0])  # Exclude idle task (0)

    # Simple state machine to find start and end of task execution
    # A task runs from CONTEXT_SWITCH (where it is selected) or TASK_RESUME
    # until TASK_PREEMPT or IDLE_ENTER or CONTEXT_SWITCH away.

    colors = plt.get_cmap("tab10")
    task_colors = {t: colors(i % 10) for i, t in enumerate(valid_tasks)}

    current_task = None
    start_time = 0

    for _, row in df.iterrows():
        evt = row["Event"]
        t = row["Timestamp"]
        tid = row["TaskID"]

        if evt in ("CONTEXT_SWITCH", "TASK_RESUME", "TASK_START", "IDLE_EXIT"):
            if evt == "IDLE_EXIT" or (evt == "CONTEXT_SWITCH" and tid > 0):
                # Context switch implies previous task preempted
                if current_task is not None and current_task > 0:
                    ax.barh(
                        current_task,
                        t - start_time,
                        left=start_time,
                        height=0.6,
                        color=task_colors[current_task],
                        edgecolor="black",
                    )
                current_task = tid
                start_time = t
            elif evt in ("TASK_RESUME", "TASK_START") and tid > 0:
                current_task = tid
                start_time = t

        elif evt in ("TASK_PREEMPT", "IDLE_ENTER", "TASK_COMPLETE", "TASK_BLOCK"):
            if current_task is not None and current_task > 0:
                ax.barh(
                    current_task,
                    t - start_time,
                    left=start_time,
                    height=0.6,
                    color=task_colors[current_task],
                    edgecolor="black",
                )
            current_task = None

        elif evt == "DEADLINE_MISS":
            if tid > 0:
                ax.scatter(t, tid, color="red", marker="x", s=100, zorder=5)

        elif evt == "FAULT_TRIGGERED":
            if tid > 0:
                ax.scatter(t, tid, color="orange", marker="^", s=100, zorder=5)

    ax.set_yticks(valid_tasks)
    ax.set_yticklabels([f"Task {t}" for t in valid_tasks])
    ax.set_xlabel("Time (Ticks)")
    ax.set_ylabel("Tasks")
    ax.set_title("SchedTiny Execution Timeline")
    ax.grid(True, linestyle="--", alpha=0.7)

    # Legends
    legend_elements: list[Any] = [
        mpatches.Patch(color=task_colors[t], label=f"Task {t}") for t in valid_tasks
    ]
    legend_elements.append(
        plt.Line2D(
            [0],
            [0],
            marker="x",
            color="w",
            markerfacecolor="red",
            markeredgecolor="red",
            markersize=10,
            label="Deadline Miss",
        )
    )
    legend_elements.append(
        plt.Line2D(
            [0],
            [0],
            marker="^",
            color="w",
            markerfacecolor="orange",
            markeredgecolor="orange",
            markersize=10,
            label="Fault Triggered",
        )
    )
    ax.legend(handles=legend_elements, bbox_to_anchor=(1.05, 1), loc="upper left")

    plt.tight_layout()
    output_path.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output_path, dpi=300, bbox_inches="tight")
    plt.close()
    print(f"Gantt chart saved to {output_path}")
