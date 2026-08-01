from typing import Any
import random
from .generator import WorkloadGenerator, WorkloadTask, uunifast


class MixedCriticalityGenerator(WorkloadGenerator):
    def generate(
        self,
        num_tasks: int = 10,
        utilization_lo: float = 0.5,
        utilization_hi: float = 0.8,
        hi_crit_ratio: float = 0.3,
        min_period: int = 10,
        max_period: int = 1000,
        **kwargs: Any,
    ) -> list[WorkloadTask]:
        self.tasks = []

        # Determine number of HI criticality tasks
        num_hi = max(1, int(round(num_tasks * hi_crit_ratio)))

        utils_lo = uunifast(num_tasks, utilization_lo)

        # HI criticality tasks need more utilization in HI mode
        utils_hi = uunifast(
            num_hi, utilization_hi - (utilization_lo * (num_tasks - num_hi) / num_tasks)
        )

        hi_task_indices = set(random.sample(range(num_tasks), num_hi))
        hi_idx = 0

        for i in range(num_tasks):
            period = random.randint(min_period, max_period)
            is_hi = i in hi_task_indices

            lo_wcet = max(1, int(round(utils_lo[i] * period)))
            hi_wcet = lo_wcet

            if is_hi:
                # HI mode WCET is typically larger
                hi_wcet = max(lo_wcet + 1, int(round(utils_hi[hi_idx] * period)))
                hi_idx += 1

            task = WorkloadTask(
                task_id=i,
                execution_time=lo_wcet,  # Starts in LO mode
                period=period,
                deadline=period,
                release_time=random.randint(0, period),
                priority=num_tasks - i,  # Rate monotonic roughly
                workload_type=0,  # Periodic
                criticality=1 if is_hi else 0,
                lo_wcet=lo_wcet,
                hi_wcet=hi_wcet,
            )
            self.tasks.append(task)

        return self.tasks
