# SchedTiny — Tests

Host-side integration tests. These run on the developer's PC (no hardware required)
and are executed by GitHub Actions CI.

---

## Structure

```
tests/
├── integration/
│   ├── test_scheduler_policy.py     # Tests scheduler policy logic (Python-level)
│   └── test_benchmark_pipeline.py   # Tests parse → compute → plot pipeline end-to-end
└── README.md
```

Firmware unit tests (CMocka, C) are in `firmware/tests/`.

---

## Running Tests

```bash
# Install dependencies
pip install -r scripts/requirements.txt -r ci/requirements-ci.txt

# Run all integration tests
pytest tests/integration/ -v --tb=short

# Run with coverage report
pytest tests/integration/ --cov=scripts/ --cov-report=term-missing
```

---

## Test Philosophy

- Integration tests verify the full data pipeline end-to-end with synthetic data
- They do not require hardware — synthetic UART logs are used as input
- Every new analysis script must have a corresponding test in `tests/integration/`
- Test data is generated deterministically (fixed random seed) for reproducibility
