#!/usr/bin/env python3
"""
parse_uart_log.py — Parse SchedTiny trace JSON output.

Reads the JSON array produced by sched_trace_export_json()
and converts it to a pandas DataFrame saved as a CSV file.

Usage:
    python parse_uart_log.py \
        --input  results/EXP-001/raw/trace.json \
        --output results/EXP-001/raw/trace.csv
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import pandas as pd


def parse_log_file(input_path: Path) -> pd.DataFrame:
    """Parse the JSON trace file into a pandas DataFrame."""
    try:
        with open(input_path, "r") as f:
            data = json.load(f)
    except json.JSONDecodeError as e:
        print(f"ERROR: Failed to parse JSON in {input_path}: {e}", file=sys.stderr)
        sys.exit(1)
    except FileNotFoundError:
        print(f"ERROR: Input file not found: {input_path}", file=sys.stderr)
        sys.exit(1)

    if not isinstance(data, list):
        print("ERROR: JSON data must be an array of trace events.", file=sys.stderr)
        sys.exit(1)

    records = []
    for record in data:
        # Validate required fields
        if not all(k in record for k in ("Timestamp", "Event", "TaskID")):
            continue

        records.append(
            {
                "Timestamp": int(record["Timestamp"]),
                "Event": str(record["Event"]),
                "TaskID": int(record["TaskID"]),
                "Policy": int(record.get("Policy", 0)),
                "Core": int(record.get("Core", 0)),
                "Criticality": int(record.get("Criticality", 0)),
                "Priority": int(record.get("Priority", 0)),
                "Deadline": int(record.get("Deadline", 0)),
                "ExecutionTime": int(record.get("ExecutionTime", 0)),
                "Metadata": int(record.get("Metadata", 0)),
            }
        )

    df = pd.DataFrame(records)
    if not df.empty:
        df.sort_values("Timestamp", inplace=True)
        df.reset_index(drop=True, inplace=True)

    return df


def main() -> None:
    parser = argparse.ArgumentParser(description="Parse SchedTiny JSON traces.")
    parser.add_argument(
        "--input", type=Path, required=True, help="Input JSON trace file"
    )
    parser.add_argument("--output", type=Path, required=True, help="Output CSV file")
    args = parser.parse_args()

    print(f"Parsing {args.input}...")
    df = parse_log_file(args.input)

    if df.empty:
        print("WARNING: No valid trace records found.")
    else:
        print(f"Successfully parsed {len(df)} trace events.")

    # Ensure output directory exists
    args.output.parent.mkdir(parents=True, exist_ok=True)
    df.to_csv(args.output, index=False)
    print(f"Trace CSV saved to {args.output}")


if __name__ == "__main__":
    main()
