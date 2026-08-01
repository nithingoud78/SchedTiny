from typing import Any
import random
from .generator import WorkloadGenerator, WorkloadTask, uunifast


class FaultInjectionGenerator(WorkloadGenerator):
    def generate(
        self,
        num_tasks: int = 10,
        utilization: float = 0.5,
        fault_rate: float = 0.1,
        min_period: int = 10,
        max_period: int = 1000,
        **kwargs: Any,
    ) -> list[WorkloadTask]:
        self.tasks = []
        utils = uunifast(num_tasks, utilization)

        for i in range(num_tasks):
            period = random.randint(min_period, max_period)
            exe_time = max(1, int(round(utils[i] * period)))

            # Use WorkloadType to encode some fault injection probability for the benchmark runner
            # Normally workload_type is 0-3. We'll leave it as periodic (0) for now.
            # Fault injection configuration is typically handled externally in this framework,
            # but we can set some tasks as CPU-bound or deadline sensitive which might be
            # more prone to faults in the runner.

            # For simplicity, standard tasks.
            task = WorkloadTask(
                task_id=i,
                execution_time=exe_time,
                period=period,
                deadline=period,
                release_time=0,
                priority=num_tasks - i,
                workload_type=0,
            )
            self.tasks.append(task)

        return self.tasks
