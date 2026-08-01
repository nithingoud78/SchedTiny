import csv
from typing import List, Dict, Any


class WorkloadTask:
    def __init__(
        self,
        task_id: int,
        execution_time: int,
        period: int,
        deadline: int,
        release_time: int,
        priority: int,
        workload_type: int = 0,
        criticality: int = 0,
        lo_wcet: int = 0,
        hi_wcet: int = 0,
    ):
        self.task_id = task_id
        self.execution_time = execution_time
        self.period = period
        self.deadline = deadline
        self.release_time = release_time
        self.priority = priority
        self.workload_type = workload_type
        self.criticality = criticality
        self.lo_wcet = lo_wcet if lo_wcet > 0 else execution_time
        self.hi_wcet = hi_wcet if hi_wcet > 0 else execution_time

    def to_dict(self) -> Dict[str, Any]:
        return {
            "TaskID": self.task_id,
            "ExecutionTime": self.execution_time,
            "Period": self.period,
            "Deadline": self.deadline,
            "ReleaseTime": self.release_time,
            "Priority": self.priority,
            "WorkloadType": self.workload_type,
            "Criticality": self.criticality,
            "LoWCET": self.lo_wcet,
            "HiWCET": self.hi_wcet,
        }


class WorkloadGenerator:
    def __init__(self):
        self.tasks: List[WorkloadTask] = []

    def generate(self, **kwargs) -> List[WorkloadTask]:
        """Override to generate a set of tasks."""
        raise NotImplementedError

    def export_csv(self, file_path: str):
        """Export tasks to CSV format compatible with C benchmark runner."""
        if not self.tasks:
            raise ValueError("No tasks generated to export.")

        fieldnames = [
            "TaskID",
            "ExecutionTime",
            "Period",
            "Deadline",
            "ReleaseTime",
            "Priority",
            "WorkloadType",
            "Criticality",
            "LoWCET",
            "HiWCET",
        ]

        with open(file_path, "w", newline="") as f:
            writer = csv.DictWriter(f, fieldnames=fieldnames)
            writer.writeheader()
            for task in self.tasks:
                writer.writerow(task.to_dict())


def uunifast(n: int, u: float) -> List[float]:
    """UUniFast algorithm for unbiased utilization generation."""
    import random
    import math

    utilizations = []
    sum_u = u
    for i in range(1, n):
        next_sum_u = sum_u * math.pow(random.random(), 1.0 / (n - i))
        utilizations.append(sum_u - next_sum_u)
        sum_u = next_sum_u
    utilizations.append(sum_u)
    return utilizations
