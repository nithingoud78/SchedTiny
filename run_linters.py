import subprocess

def run_cmd(cmd, outfile):
    print(f"Running {cmd}")
    try:
        res = subprocess.run(cmd, shell=True, capture_output=True, text=True)
        outfile.write(f"=== CMD: {cmd} ===\n")
        outfile.write(f"STDOUT:\n{res.stdout}\n")
        outfile.write(f"STDERR:\n{res.stderr}\n")
        outfile.write(f"RETCODE: {res.returncode}\n\n")
    except Exception as e:
        outfile.write(f"Exception running {cmd}: {e}\n")

with open("lint_report.txt", "w") as f:
    run_cmd("python -m black --check --diff scripts/", f)
    run_cmd("python -m flake8 scripts/ --max-line-length=100 --ignore=E203,W503", f)
    run_cmd("python -m mypy scripts/ --ignore-missing-imports", f)
    run_cmd("python -m yamllint configs/ .github/", f)
