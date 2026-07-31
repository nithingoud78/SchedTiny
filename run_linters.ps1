# Run all linters and capture output
$ErrorActionPreference = "Continue"

echo "=== PYTHON BLACK ===" > lint_output.txt
python -m black --check --diff scripts/ 2>&1 >> lint_output.txt

echo "=== PYTHON FLAKE8 ===" >> lint_output.txt
python -m flake8 scripts/ --max-line-length=100 --ignore=E203,W503 2>&1 >> lint_output.txt

echo "=== PYTHON MYPY ===" >> lint_output.txt
python -m mypy scripts/ --ignore-missing-imports 2>&1 >> lint_output.txt

echo "=== CLANG-FORMAT ===" >> lint_output.txt
# Using a simpler loop since find | xargs is not native to powershell
Get-ChildItem -Path firmware/core, firmware/research, firmware/include -Include *.c, *.h -Recurse | ForEach-Object {
    clang-format --dry-run --Werror --style=file:tools/clang-format/.clang-format $_.FullName 2>&1 >> lint_output.txt
}

echo "=== YAMLLINT ===" >> lint_output.txt
python -m yamllint configs/ .github/ 2>&1 >> lint_output.txt

echo "DONE" >> lint_output.txt
