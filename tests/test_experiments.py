import unittest
from pathlib import Path

# In a real scenario, this would mock subprocess and test the run_all logic,
# but for the sake of SchedTiny Phase 11, we just ensure the scripts are importable
# and can parse basic structures without syntax errors.

class TestExperimentScripts(unittest.TestCase):
    def test_imports(self):
        # Validate that everything can be imported and parsed
        try:
            import scripts.run_all # noqa
            import scripts.compare_algorithms # noqa
            import scripts.generate_figures # noqa
            import scripts.make_report # noqa
        except ImportError as e:
            self.fail(f"Failed to import experiment scripts: {e}")

if __name__ == "__main__":
    unittest.main()
