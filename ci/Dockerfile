# SchedTiny CI Docker Image
# Provides a reproducible build environment for all firmware and Python targets.
#
# Build: docker build -t schedtiny-ci ci/
# Run:   docker run --rm -v $(pwd):/workspace schedtiny-ci <command>

FROM ubuntu:24.04

LABEL maintainer="nithingoud78"
LABEL description="SchedTiny CI build environment — ARM GNU + Python 3.12"
LABEL version="0.1.0"

# Prevent interactive prompts
ENV DEBIAN_FRONTEND=noninteractive

# ---------------------------------------------------------------------------
# System packages
# ---------------------------------------------------------------------------
RUN apt-get update -qq && \
    apt-get install -y --no-install-recommends \
        # ARM cross-compiler
        gcc-arm-none-eabi \
        binutils-arm-none-eabi \
        # Build system
        cmake \
        ninja-build \
        make \
        # Unit testing
        libcmocka-dev \
        # Python
        python3.12 \
        python3.12-venv \
        python3-pip \
        # Utilities
        git \
        wget \
        curl \
        doxygen \
        graphviz \
        clang-format \
        yamllint \
        # LaTeX (for paper builds — optional, ~500 MB; comment out if not needed)
        # texlive-full \
    && rm -rf /var/lib/apt/lists/*

# ---------------------------------------------------------------------------
# Verify toolchain
# ---------------------------------------------------------------------------
RUN arm-none-eabi-gcc --version && \
    cmake --version && \
    python3.12 --version

# ---------------------------------------------------------------------------
# Python environment
# ---------------------------------------------------------------------------
WORKDIR /workspace
COPY ../scripts/requirements.txt /tmp/requirements.txt
COPY requirements-ci.txt /tmp/requirements-ci.txt

RUN python3.12 -m pip install --no-cache-dir \
    -r /tmp/requirements.txt \
    -r /tmp/requirements-ci.txt

# ---------------------------------------------------------------------------
# Default working directory and command
# ---------------------------------------------------------------------------
WORKDIR /workspace
CMD ["bash"]
