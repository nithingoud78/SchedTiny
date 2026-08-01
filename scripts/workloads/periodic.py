from typing import Any
from .generator import WorkloadGenerator, WorkloadTask, uunifast


class PeriodicGenerator(WorkloadGenerator):
    def generate(
        self,
        num_tasks: int = 10,
        utilization: float = 0.5,
        min_period: int = 10,
        max_period: int = 1000,
        harmonic: bool = False,
        **kwargs: Any,
    ) -> list[WorkloadTask]:
        self.tasks = []
        utils = uunifast(num_tasks, utilization)

        current_period = min_period
        for i in range(num_tasks):
            if harmonic:
                period = current_period
                current_period *= 2
                if current_period > max_period:
                    current_period = min_period
            else:
                import random

                period = random.randint(min_period, max_period)

            exe_time = max(1, int(round(utils[i] * period)))

            task = WorkloadTask(
                task_id=i,
                execution_time=exe_time,
                period=period,
                deadline=period,  # Implicit deadline
                release_time=0,
                priority=num_tasks
                - i,  # Rate monotonic roughly (needs sorting for exact RM)
                workload_type=0,  # Periodic
            )
            self.tasks.append(task)

        # Sort by period for Rate Monotonic Priority assignment
        self.tasks.sort(key=lambda t: t.period)
        for idx, t in enumerate(self.tasks):
            t.priority = num_tasks - idx  # Higher priority is higher number

        return self.tasks
