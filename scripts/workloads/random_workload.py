from typing import Any
import random
from .generator import WorkloadGenerator, WorkloadTask, uunifast


class RandomGenerator(WorkloadGenerator):
    def generate(
        self,
        num_tasks: int = 10,
        utilization: float = 0.5,
        min_period: int = 10,
        max_period: int = 1000,
        **kwargs: Any,
    ) -> list[WorkloadTask]:
        self.tasks = []
        utils = uunifast(num_tasks, utilization)

        for i in range(num_tasks):
            period = random.randint(min_period, max_period)
            exe_time = max(1, int(round(utils[i] * period)))

            # Constrained deadline (between exe_time and period)
            deadline = random.randint(exe_time, period)

            task = WorkloadTask(
                task_id=i,
                execution_time=exe_time,
                period=period,
                deadline=deadline,
                release_time=random.randint(0, period),
                priority=random.randint(1, 255),
                workload_type=random.randint(
                    0, 3
                ),  # Mix of periodic, aperiodic, cpu, deadline sensitive
            )
            self.tasks.append(task)

        return self.tasks
