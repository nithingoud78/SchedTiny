import subprocess
from pathlib import Path


def get_latest_experiment_dir(root: Path) -> Path:
    latest_file = root / "results" / "latest.txt"
    if not latest_file.exists():
        raise FileNotFoundError(
            "No experiments found. Run run_all_experiments.py first."
        )
    with open(latest_file, "r") as f:
        dirname = f.read().strip()
    return root / "results" / dirname


def main():
    root = Path(__file__).parent.parent.absolute()
    exp_dir = get_latest_experiment_dir(root)

    report_dir = root / "reports"
    report_dir.mkdir(exist_ok=True)

    report_file = report_dir / f"Experiment_Report_{exp_dir.name}.md"

    figures_src = exp_dir / "figures"
    tables_src = exp_dir / "tables"

    # Get git hash if available for reproducibility
    git_hash = "Unknown"
    try:
        res = subprocess.run(
            ["git", "rev-parse", "--short", "HEAD"],
            capture_output=True,
            text=True,
            cwd=str(root),
        )
        if res.returncode == 0:
            git_hash = res.stdout.strip()
    except Exception:
        pass

    with open(report_file, "w") as f:
        f.write("# Experimental Evaluation & Hardware Validation Report\n\n")
        f.write(f"**Experiment ID:** `{exp_dir.name}`\n")
        f.write(f"**Git Commit:** `{git_hash}`\n")
        f.write("**Target MCU:** `STM32H743ZI (Arm Cortex-M7 @ 480MHz)`\n")
        f.write("**Compiler:** `arm-none-eabi-gcc 12.3.1 (-O3 -flto)`\n")
        f.write("**Firmware Version:** `1.0.0`\n\n")

        f.write("## 1. Generated Tables (LaTeX)\n\n")

        if tables_src.exists():
            for tex_file in tables_src.glob("*.tex"):
                f.write(f"### {tex_file.name}\n")
                f.write("```latex\n")
                with open(tex_file, "r") as tf:
                    f.write(tf.read())
                f.write("```\n\n")

        f.write("## 2. Visualizations\n\n")

        if figures_src.exists():
            for fig_file in figures_src.glob("*.pdf"):
                f.write(f"### {fig_file.stem}\n\n")
                # Since Markdown doesn't render PDF inline well, we mention it's attached
                f.write(
                    f"**File:** `figures/{fig_file.name}` "
                    "(Available in experiment output directory)\n\n"
                )

        f.write("## 3. TinyML Validation\n\n")

        ml_json = exp_dir / "tinyml_metrics.json"
        if ml_json.exists():
            f.write("### Model Footprint & Cross-Validation Metrics\n")
            f.write("```json\n")
            with open(ml_json, "r") as tf:
                f.write(tf.read())
            f.write("```\n\n")

        f.write("## 4. Hardware vs. Simulation Validation\n\n")
        hw_json = exp_dir / "hardware_validation_metrics.json"
        if hw_json.exists():
            f.write("### Error Characterization (MAPE & RMSE)\n")
            f.write("```json\n")
            with open(hw_json, "r") as hf:
                f.write(hf.read())
            f.write("```\n\n")

        f.write("## 5. Reproducibility\n\n")
        f.write(
            "All raw JSON metrics and CSV workloads used to generate this report are stored in:\n"
        )
        f.write(f"`{exp_dir.relative_to(root)}`\n")

    print(f"Report generated successfully at {report_file}")


if __name__ == "__main__":
    main()
