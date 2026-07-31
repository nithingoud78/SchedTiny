import shutil
from pathlib import Path


def get_latest_experiment_dir(root: Path) -> Path:
    latest_file = root / "results" / "latest.txt"
    if not latest_file.exists():
        raise FileNotFoundError("No experiments found. Run run_all.py first.")
    with open(latest_file, "r") as f:
        dirname = f.read().strip()
    return root / "results" / dirname


def main():
    root = Path(__file__).parent.parent.absolute()
    exp_dir = get_latest_experiment_dir(root)

    paper_dir = root / "paper"
    paper_dir.mkdir(exist_ok=True)

    report_file = paper_dir / "Results.md"

    comparison_md = exp_dir / "comparison.md"
    if not comparison_md.exists():
        print("comparison.md not found. Run compare_algorithms.py first.")
        return

    with open(comparison_md, "r") as f:
        comp_text = f.read()

    # Copy figures to a stable paper/figures dir so the markdown works seamlessly
    figures_src = exp_dir / "figures"
    figures_dst = paper_dir / "figures"
    if figures_src.exists():
        if figures_dst.exists():
            shutil.rmtree(figures_dst)
        shutil.copytree(figures_src, figures_dst)

    with open(report_file, "w") as f:
        f.write("# Experimental Results\n\n")
        f.write(f"**Experiment ID:** `{exp_dir.name}`\n\n")

        f.write("## 1. Summary Statistics\n\n")
        f.write(comp_text)
        f.write("\n")

        f.write("## 2. Visualizations\n\n")

        figs = [
            ("Figure 2: CPU Utilization", "figure2_cpu_utilization.png"),
            ("Figure 3: Deadline Misses", "figure3_deadline_misses.png"),
            ("Figure 4: Context Switches", "figure4_context_switches.png"),
            ("Figure 5: Scheduler Latency", "figure5_scheduler_latency.png"),
            ("Figure 6: Waiting Time", "figure6_waiting_time.png"),
            ("Figure 7: Response Time", "figure7_response_time.png"),
            ("Figure 8: Throughput", "figure8_throughput.png"),
            ("Figure 9: Estimated Energy", "figure9_estimated_energy.png"),
            ("Figure 10: Energy per Task", "figure10_energy_per_task.png"),
            ("Figure 11: Mode Switches", "figure11_mode_switches.png"),
            ("Figure 12: Dropped LO Tasks", "figure12_dropped_lo_tasks.png"),
            ("Figure 13: Recovery Success", "figure13_recovery_success.png"),
            (
                "Figure 14: System Availability",
                "figure14_system_availability.png",
            ),
            ("Figure 15: Fault Coverage", "figure15_fault_coverage.png"),
        ]

        for title, filename in figs:
            f.write(f"### {title}\n\n")
            f.write(f"![{title}](figures/{filename})\n\n")

        f.write("## 3. Algorithm Comparison (Conclusion)\n\n")
        f.write(
            "As shown in the figures and tables above, the various "
            "scheduling policies exhibit distinct behaviors:\n"
        )
        f.write(
            "- **HPF** excels in simplicity but can " "starve lower-priority tasks.\n"
        )
        f.write(
            "- **EDF** achieves zero or near-zero deadline misses "
            "under high utilization up to 100% theoretical bound.\n"
        )
        f.write(
            "- **RMS** provides optimal static-priority scheduling "
            "for periodic tasks but suffers if CPU utilization exceeds ~69%.\n"
        )

    print(f"Report generated successfully at {report_file}")


if __name__ == "__main__":
    main()
