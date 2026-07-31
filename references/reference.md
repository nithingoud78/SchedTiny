# SchedTiny

An Open Benchmark and Interrupt-Aware Scheduling Framework for Co-Running Real-Time Control Tasks and TinyML Inference on STM32 Microcontrollers

## Project Structure

SchedTiny
│
├── docs
├── firmware
├── experiments
├── models
├── datasets
├── papers
├── references/
|       |
|       ├── papers/
|       │
|       ├── research.md
|       │
|       ├── literature_matrix.md
|       │
|       ├── glossary.md
|       │
|       ├── ideas.md
|       │
|       └── open_questions.md
|       
└── citations.bib
├── scripts
├── results
├── hardware
├── meeting_notes
└── README.md


## Install Software

Git
GitHub Desktop (optional)
VS Code
STM32CubeIDE
STM32CubeMX (included with CubeIDE)
Python 3.12+
CMake
ARM GNU Toolchain
STM32CubeProgrammer
Doxygen
LaTeX (TeX Live or MiKTeX)
Zotero (reference manager)

## GitHub Repository

https://github.com/nithingoud78/SchedTiny.git

## Hardware

STM32 NUCLEO-H743ZI2 (preferred) or STM32F746G-Discovery
MPU6050
INA219
Breadboard
Jumper wires
Logic Analyzer (24 MHz clone)

## Architecture

Before coding, understand what we are building.

          Sensors
             │
             ▼
      Sensor Driver
             │
             ▼
      FreeRTOS Scheduler
        ┌───────────────┐
        │               │
        ▼               ▼
 Real-Time Tasks    TinyML Task
        │               │
        └──────┬────────┘
               ▼
      Measurement Layer
               ▼
      UART / GPIO Timing
               ▼
        Python Analysis
               ▼
      Research Figures
               ▼
        IEEE Journal


## Research Reading

Before implementation, read approximately 15–20 papers on:

FreeRTOS scheduling
Mixed-criticality systems
TinyML
CMSIS-NN
TensorFlow Lite Micro
Real-time scheduling
Embedded benchmarking

# Rules for This Project

These are non-negotiable if the goal is a journal paper:

Never copy code without understanding it.
Every graph must come from experimental data.
Every experiment must be reproducible.
Commit frequently with meaningful messages.
Document hardware connections and software versions.
Keep a research diary after each session.
Read papers before claiming novelty.