#!/usr/bin/env python3
"""
parse_uart_log.py — Parse SchedTiny structured UART JSON log output.

Reads the newline-delimited JSON records produced by firmware/src/bench/bench_log.c
and converts them to a pandas DataFrame saved as a CSV file.

Expected input format (one JSON object per line):
    {"ts":123456,"ev":"TASK_START","tid":1,"val":0}

Where:
    ts  — timestamp in microseconds (from DWT cycle counter)
    ev  — event type string (TASK_START, TASK_END, INFER_START, etc.)
    tid — task or ISR source ID (uint8)
    val — optional value (latency, ISR source, etc.)

Usage:
    python parse_uart_log.py \\
        --input  results/EXP-001/raw/run_001.txt \\
        --output results/EXP-001/raw/run_001.csv

Output CSV columns:
    timestamp_us, event_type, task_id, value
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

import pandas as pd

# ---------------------------------------------------------------------------
# Constants
# ---------------------------------------------------------------------------

# Map event type strings to integer codes for downstream analysis
EVENT_TYPE_MAP: dict[str, int] = {
    "TASK_START": 0,
    "TASK_END": 1,
    "INFER_START": 2,
    "INFER_END": 3,
    "ISR_ENTRY": 4,
    "ISR_EXIT": 5,
    "DEADLINE_MISS": 6,
    "WINDOW_START": 7,
    "WINDOW_END": 8,
}


# ---------------------------------------------------------------------------
# Core functions
# ---------------------------------------------------------------------------


def parse_log_line(line: str, line_number: int) -> dict | None:
    """Parse a single JSON log line from the UART output.

    Args:
        line:        Raw line string from the log file.
        line_number: Line number for error reporting.

    Returns:
        A dict with keys {timestamp_us, event_type, task_id, value},
        or None if the line is malformed or should be skipped.
    """
    line = line.strip()
    if not line or line.startswith("#"):
        # Skip blank lines and comment lines (used for session metadata)
        return None

    try:
        record = json.loads(line)
    except json.JSONDecodeError as e:
        print(
            f"WARNING: Line {line_number}: JSON parse error: {e}",
            file=sys.stderr,
        )
        return None

    # Validate required fields
    for field in ("ts", "ev", "tid"):
        if field not in record:
            print(
                f"WARNING: Line {line_number}: missing field '{field}', skipping.",
                file=sys.stderr,
            )
            return None

    return {
        "timestamp_us": int(record["ts"]),
        "event_type": str(record["ev"]),
        "task_id": int(record["tid"]),
        "value": int(record.get("val", 0)),
    }


def parse_log_file(input_path: Path) -> pd.DataFrame:
    """Parse all lines in a UART log file.

    Args:
        input_path: Path to the raw UART log file.

    Returns:
        DataFrame with columns: timestamp_us, event_type, task_id, value.

    Raises:
        FileNotFoundError: If input_path does not exist.
    """
    if not input_path.exists():
        raise FileNotFoundError(f"Input file not found: {input_path}")

    records: list[dict] = []
    with input_path.open("r", encoding="utf-8") as fh:
        for line_number, line in enumerate(fh, start=1):
            record = parse_log_line(line, line_number)
            if record is not None:
                records.append(record)

    if not records:
        raise ValueError(f"No valid records found in {input_path}")

    df = pd.DataFrame(records)

    # Validate timestamp monotonicity (DWT overflow should have been handled in firmware)
    if not df["timestamp_us"].is_monotonic_increasing:
        n_violations = (df["timestamp_us"].diff() < 0).sum()
        print(
            f"WARNING: {n_violations} non-monotonic timestamp(s) detected. "
            "This may indicate DWT overflow in firmware. "
            "See docs/CODING_STANDARD.md for the overflow-safe API.",
            file=sys.stderr,
        )

    return df


def parse_args() -> argparse.Namespace:
    """Parse command-line arguments."""
    parser = argparse.ArgumentParser(
        description=__doc__,
        formatter_class=argparse.RawDescriptionHelpFormatter,
    )
    parser.add_argument(
        "--input",
        type=Path,
        required=True,
        help="Path to raw UART log file (.txt or .log)",
    )
    parser.add_argument(
        "--output",
        type=Path,
        required=True,
        help="Path to output CSV file",
    )
    parser.add_argument(
        "--verbose",
        action="store_true",
        help="Print parsing statistics to stdout",
    )
    return parser.parse_args()


def main() -> int:
    """Entry point. Returns 0 on success, non-zero on error."""
    args = parse_args()

    try:
        df = parse_log_file(args.input)
    except (FileNotFoundError, ValueError) as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return 1

    # Ensure output directory exists
    args.output.parent.mkdir(parents=True, exist_ok=True)

    df.to_csv(args.output, index=False)

    if args.verbose:
        print(f"Parsed {len(df)} records from {args.input}")
        print(f"Event type distribution:\n{df['event_type'].value_counts()}")
        print(f"Time range: {df['timestamp_us'].min()} – {df['timestamp_us'].max()} µs")
        print(f"Saved to {args.output}")

    return 0


if __name__ == "__main__":
    sys.exit(main())
