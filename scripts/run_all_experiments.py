import subprocess
import time
import json
import pandas as pd
from pathlib import Path
from workloads import (
    PeriodicGenerator,
    RandomGenerator,
    MixedCriticalityGenerator,
)


def build_runner(root_dir: Path) -> Path:
    print("Building benchmark runner...")
    tests_dir = root_dir / "firmware" / "tests"
    build_dir = tests_dir / "build"

    try:
        subprocess.run(
            ["cmake", "-S", str(tests_dir), "-B", str(build_dir)], check=True
        )
        subprocess.run(
            ["cmake", "--build", str(build_dir), "--target", "benchmark_runner"],
            check=True,
        )
    except subprocess.CalledProcessError as e:
        print(f"Error building runner: {e}")
        exit(1)

    runner = build_dir / "benchmark_runner.exe"
    if not runner.exists():
        runner = build_dir / "benchmark_runner"
    if not runner.exists():
        runner = build_dir / "Debug" / "benchmark_runner.exe"

    return runner


def run_experiment(
    runner: Path, workload_csv: Path, seed: int, out_format: str = "json"
) -> str:
    cmd = [
        str(runner),
        "--workload-csv",
        str(workload_csv),
        "--seed",
        str(seed),
        "--ticks",
        "100000",
        "--out",
        out_format,
    ]
    res = subprocess.run(cmd, capture_output=True, text=True, check=True)
    return res.stdout


def main():
    root = Path(__file__).parent.parent.absolute()
    runner = build_runner(root)

    timestamp = time.strftime("%Y%m%d_%H%M%S")
    out_dir = root / "results" / f"experiment_{timestamp}"
    out_dir.mkdir(parents=True, exist_ok=True)

    # Save latest
    with open(root / "results" / "latest.txt", "w") as f:
        f.write(f"experiment_{timestamp}")

    workload_dir = out_dir / "workloads"
    workload_dir.mkdir(exist_ok=True)

    raw_dir = out_dir / "raw"
    raw_dir.mkdir(exist_ok=True)

    generators = {
        "periodic": PeriodicGenerator(),
        "random": RandomGenerator(),
        "mixed_criticality": MixedCriticalityGenerator(),
    }

    # Policies mapped to their enum values or we can let runner iterate if it runs all
    # Wait, benchmark_runner.c runs ALL policies automatically in sched_benchmark_run_all
    # and outputs a JSON array!

    all_results = []

    utilizations = [0.2, 0.4, 0.6, 0.8, 0.95]
    seeds = [1, 2, 3]

    print(f"Running experiments... Saving to {out_dir}")

    for gen_name, gen in generators.items():
        for util in utilizations:
            for seed in seeds:
                print(f"  -> {gen_name} util={util} seed={seed}")

                # Generate workload
                if gen_name == "mixed_criticality":
                    gen.generate(
                        num_tasks=20, utilization_lo=util * 0.7, utilization_hi=util
                    )
                else:
                    gen.generate(num_tasks=20, utilization=util)

                wl_file = workload_dir / f"{gen_name}_u{util:.2f}_s{seed}.csv"
                gen.export_csv(str(wl_file))

                # Run benchmark
                json_out = run_experiment(runner, wl_file, seed, "json")

                # Save raw json
                raw_file = raw_dir / f"{gen_name}_u{util:.2f}_s{seed}.json"
                with open(raw_file, "w") as f:
                    f.write(json_out)

                # Parse and append to aggregate
                try:
                    data = json.loads(json_out)
                    for item in data:
                        item["workload_type"] = gen_name
                        item["utilization"] = util
                        item["seed"] = seed
                        all_results.append(item)
                except Exception as e:
                    print(f"Failed to parse JSON for {wl_file}: {e}")

    # Save aggregated DataFrame
    df = pd.DataFrame(all_results)
    df.to_csv(out_dir / "aggregated_results.csv", index=False)

    print("Experiments complete. Generating figures and tables...")

    # Ensure analysis scripts are executable
    subprocess.run(
        [
            "python",
            str(root / "scripts" / "analysis" / "plot_figures.py"),
            "--metrics",
            str(out_dir),
            "--output",
            str(out_dir / "figures"),
        ]
    )

    subprocess.run(
        [
            "python",
            str(root / "scripts" / "analysis" / "generate_tables.py"),
            "--metrics",
            str(out_dir),
            "--output",
            str(out_dir / "tables"),
        ]
    )

    print("Running TinyML Pipeline...")
    subprocess.run(
        [
            "python",
            str(root / "scripts" / "train_scheduler_model.py"),
            "--results-dir",
            str(out_dir),
        ]
    )
    subprocess.run(
        [
            "python",
            str(root / "scripts" / "evaluate_scheduler_model.py"),
            "--results-dir",
            str(out_dir),
        ]
    )
    subprocess.run(
        [
            "python",
            str(root / "scripts" / "export_decision_tree.py"),
            "--results-dir",
            str(out_dir),
            "--out",
            str(root / "firmware" / "include" / "sched_adaptive_model.h"),
        ]
    )

    print("Generating Report...")
    subprocess.run(["python", str(root / "scripts" / "make_report.py")])
    print("Done!")


if __name__ == "__main__":
    main()
