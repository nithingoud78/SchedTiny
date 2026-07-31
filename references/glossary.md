# SchedTiny Glossary

Domain terminology definitions for SchedTiny. Every term used in `research.md`,
`docs/SPEC.md`, and the paper must be defined here on first introduction.

---

## Scheduling Theory

**Fixed-Priority Preemptive (FPP):** A scheduling policy where each task has a
static priority assigned before runtime. The highest-priority ready task always
runs. Lower-priority tasks are preempted when a higher-priority task becomes ready.
This is the default FreeRTOS scheduling mode.

**Rate-Monotonic (RM):** A static-priority scheduling policy where shorter-period
tasks are assigned higher priorities. Proven to be optimal among fixed-priority
policies for periodic tasks. Schedulability test: ∑(Cᵢ/Tᵢ) ≤ n(2^(1/n) − 1).

**Earliest Deadline First (EDF):** A dynamic-priority scheduling policy where the
task with the earliest absolute deadline always runs next. Optimal for preemptive
systems; achieves 100% CPU utilization. Computationally more expensive than FPP.

**WCET (Worst-Case Execution Time):** The maximum time a task can take to complete
one execution in the worst-case input and hardware state. For TinyML tasks, WCET
is probabilistic due to variable input-dependent execution paths in TFLM operators.

**Response Time:** The time from a task's release (arrival) to its completion.
For a task to meet its deadline: Rᵢ ≤ Dᵢ. Computed by response-time analysis (RTA).

**Response-Time Analysis (RTA):** An iterative fixed-point method for computing
worst-case response times of fixed-priority tasks under preemption.

**Jitter:** Variation in task response time or ISR execution time between successive
activations. In SchedTiny: interrupt-induced jitter = latency delta caused by an ISR
preempting a task at a critical measurement point.

**Deadline Miss:** When a task's response time exceeds its deadline (Rᵢ > Dᵢ).
Hard deadline misses are system failures; soft deadline misses are measured as a
performance metric.

**Mixed-Criticality System (MCS):** A system where tasks have different criticality
levels (hard/soft deadlines, different WCET estimates). SchedTiny models PID as
high-criticality (hard deadline) and TinyML inference as low-criticality (soft).

**Schedulability:** A task set is schedulable if all tasks can meet their deadlines
under the given scheduling policy and hardware constraints.

---

## TinyML / Embedded ML

**TinyML:** Machine learning inference deployed on microcontrollers with tight
constraints on memory (< 1 MB SRAM), compute (< 500 MHz), and energy (< 100 mW).

**TFLM (TensorFlow Lite Micro):** Google's TinyML inference runtime designed for
bare-metal MCUs with < 256 KB SRAM. Uses flatbuffer model format (.tflite).

**CMSIS-NN:** ARM's neural network kernel library for Cortex-M (M4/M7/M33).
Provides optimized INT8/INT16 implementations of common DNN operators (Conv2D,
DepthwiseConv2D, GEMM, etc.). TFLM delegates to CMSIS-NN kernels on ARM.

**KWS (Keyword Spotting):** The task of detecting a specific spoken keyword in
an audio stream. A standard TinyML benchmark task in MLPerf Tiny.

**DS-CNN (Depthwise Separable CNN):** A CNN architecture using depthwise separable
convolutions that achieves high accuracy with significantly lower compute than
standard CNNs. Standard model for KWS in embedded ML.

**INT8 Quantization:** Post-training quantization that converts model weights and
activations from float32 to 8-bit integers. Reduces model size by ~4× and enables
integer-only inference on MCUs without FPU support.

**Tensor Arena:** The static memory buffer allocated for TFLM at initialization,
used to hold all intermediate tensor activations during inference.

---

## STM32 / Hardware

**DWT (Data Watchpoint and Trace):** ARM Cortex-M debug unit that includes a
free-running cycle counter (DWT->CYCCNT). Used in SchedTiny for zero-overhead
latency measurement at < 10 ns resolution.

**ISR (Interrupt Service Routine):** A function called by the processor in
response to a hardware interrupt. ISR execution preempts any running task,
including FreeRTOS tasks. ISR latency directly contributes to scheduling jitter.

**INA219:** A Texas Instruments current/power monitor IC. Interfaces via I²C.
Measures voltage (bus and shunt), current (via shunt resistor), and power.
Used in SchedTiny for energy-per-inference measurement.

**MPU6050:** InvenSense 6-axis IMU (3-axis accelerometer + 3-axis gyroscope).
Used as the sensor source for the benchmark sensor acquisition task.

**NUCLEO:** STMicroelectronics development boards featuring a built-in ST-LINK
debugger and Arduino/Morpho connector headers. SchedTiny targets the Nucleo-144
form factor boards.

---

## Benchmarking

**MLPerf Tiny:** Industry-standard TinyML benchmarking suite from MLCommons.
Covers KWS, visual wake words, anomaly detection, and image classification.
SchedTiny extends MLPerf Tiny with interrupt-aware, co-running scenarios.

**p95/p99 Latency:** The 95th/99th percentile of the latency distribution.
Preferred over mean for real-time systems, where tail latency determines
schedulability and deadline miss rate.

**CDF (Cumulative Distribution Function):** A plot of latency percentile vs.
latency value. Standard visualization for inference latency in SchedTiny paper figures.
