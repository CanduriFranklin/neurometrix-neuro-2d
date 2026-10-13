# syntax=docker/dockerfile:1.7
# ============================================================================
# neuro-2d - Multi-stage container build
# ============================================================================
# Targets:
#   runtime-cpu   : local development, Eigen on CPU
#   runtime-rocm  : AMD Developer Cloud, ROCm 6.2 + hipBLAS acceleration
#
# Build:
#   podman build --target runtime-cpu  -t localhost/neurometrix/neuro-2d:cpu  .
#   podman build --target runtime-rocm -t localhost/neurometrix/neuro-2d:rocm .
# ============================================================================

ARG BASE_BUILD=ubuntu:24.04
ARG BASE_RUNTIME=ubuntu:24.04
ARG BASE_ROCM=rocm/dev-ubuntu-24.04:6.2

# ---------------------------------------------------------------------------
# Stage: build
# ---------------------------------------------------------------------------
FROM ${BASE_BUILD} AS build

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential cmake ninja-build git ca-certificates \
    curl zip unzip tar pkg-config python3 \
 && rm -rf /var/lib/apt/lists/*

ENV VCPKG_ROOT=/vcpkg
RUN git clone --depth 1 https://github.com/microsoft/vcpkg.git $VCPKG_ROOT \
 && $VCPKG_ROOT/bootstrap-vcpkg.sh -disableMetrics

WORKDIR /src
COPY vcpkg.json .
RUN $VCPKG_ROOT/vcpkg install --triplet x64-linux

COPY . .
RUN cmake -S . -B build -G Ninja \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake \
 && cmake --build build --parallel

# ---------------------------------------------------------------------------
# Stage: runtime-cpu (local development)
# ---------------------------------------------------------------------------
FROM ${BASE_RUNTIME} AS runtime-cpu

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 libpq5 libsasl2-2 zlib1g libgomp1 ca-certificates \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/neuro-2d /usr/local/bin/neuro-2d

USER 65534:65534
ENTRYPOINT ["/usr/local/bin/neuro-2d"]

# ---------------------------------------------------------------------------
# Stage: runtime-rocm (AMD Developer Cloud)
# ---------------------------------------------------------------------------
# This stage targets AMD Instinct accelerators via ROCm 6.2.
# Eigen with EIGEN_USE_BLAS + hipBLAS accelerates the linear algebra.
# ---------------------------------------------------------------------------
FROM ${BASE_ROCM} AS runtime-rocm

RUN apt-get update && apt-get install -y --no-install-recommends \
    libssl3 libpq5 libsasl2-2 zlib1g libgomp1 ca-certificates \
 && rm -rf /var/lib/apt/lists/*

COPY --from=build /src/build/neuro-2d /usr/local/bin/neuro-2d

# ROCm runtime environment (overridable by K8s env)
ENV HSA_OVERRIDE_GFX_VERSION=9.4.2
ENV ROCR_VISIBLE_DEVICES=0
ENV AMD_SERIALIZE_KERNEL=0

USER 65534:65534
ENTRYPOINT ["/usr/local/bin/neuro-2d"]