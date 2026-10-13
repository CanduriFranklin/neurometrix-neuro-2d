# ============================================================================
# neuro-2d - Developer make targets
# ============================================================================

SHELL := /bin/bash
IMAGE ?= localhost/neurometrix/neuro-2d
TAG   ?= dev

.PHONY: help configure build clean test image-cpu image-rocm compose-up compose-down

help:
@echo "Targets:"
@echo "  configure    Configure CMake build directory"
@echo "  build        Build neuro-2d (Release)"
@echo "  clean        Remove build directory"
@echo "  test         Run unit tests"
@echo "  image-cpu    Build Podman image (CPU target)"
@echo "  image-rocm   Build Podman image (ROCm target, for AMD Cloud)"
@echo "  compose-up   Start local stack with podman compose"
@echo "  compose-down Stop local stack"

configure:
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_TOOLCHAIN_FILE=$(VCPKG_ROOT)/scripts/buildsystems/vcpkg.cmake

build:
cmake --build build --config Release --parallel

clean:
rm -rf build

test: build
ctest --test-dir build -C Release --output-on-failure

image-cpu:
podman build --target runtime-cpu -t $(IMAGE):$(TAG)-cpu .

image-rocm:
podman build --target runtime-rocm -t $(IMAGE):$(TAG)-rocm .

compose-up:
podman compose -f deploy/compose.yaml up -d

compose-down:
podman compose -f deploy/compose.yaml down