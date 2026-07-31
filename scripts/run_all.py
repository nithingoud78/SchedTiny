import subprocess
import time
from pathlib import Path
import sys


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
        print(f"Error building runner: {e}", file=sys.stderr)
        sys.exit(1)

    runner = build_dir / "benchmark_runner.exe"
    if not runner.exists():
        runner = build_dir / "benchmark_runner"  # Linux/macOS
    if not runner.exists():
        # Sometimes cmake puts it in Debug/Release subfolders on Windows
        runner = build_dir / "Debug" / "benchmark_runner.exe"

    return runner


def main():
    root = Path(__file__).parent.parent.absolute()

    runner_path = build_runner(root)
    if not runner_path.exists():
        print(
            f"Could not find compiled benchmark_runner at {runner_path.parent}",
            file=sys.stderr,
        )
        sys.exit(1)

    timestamp = time.strftime("%Y%m%d_%H%M%S")
    out_dir = root / "results" / f"experiment_{timestamp}"
    out_dir.mkdir(parents=True, exist_ok=True)

    profiles = ["small", "medium", "large", "stress"]
    seeds = [1, 2, 3, 4, 5]

    print(f"Running benchmarks. Outputs saving to {out_dir}")

    for profile in profiles:
        for seed in seeds:
            print(f"  -> profile={profile}, seed={seed}")

            # JSON
            cmd_json = [
                str(runner_path),
                "--profile",
                profile,
                "--seed",
                str(seed),
                "--ticks",
                "10000",
                "--out",
                "json",
            ]
            res_json = subprocess.run(
                cmd_json, capture_output=True, text=True, check=True
            )
            with open(out_dir / f"metrics_{profile}_{seed}.json", "w") as f:
                f.write(res_json.stdout)

            # CSV
            cmd_csv = [
                str(runner_path),
                "--profile",
                profile,
                "--seed",
                str(seed),
                "--ticks",
                "10000",
                "--out",
                "csv",
            ]
            res_csv = subprocess.run(
                cmd_csv, capture_output=True, text=True, check=True
            )
            with open(out_dir / f"metrics_{profile}_{seed}.csv", "w") as f:
                f.write(res_csv.stdout)

    print(f"\nBenchmarking complete. Data saved to {out_dir}")

    # Store the latest experiment dir for other scripts
    with open(root / "results" / "latest.txt", "w") as f:
        f.write(out_dir.name)


if __name__ == "__main__":
    main()
