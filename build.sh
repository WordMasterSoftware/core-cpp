#!/usr/bin/env bash

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
TOOLCHAIN_FILE="${PROJECT_ROOT}/vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}"
cmake --build "${BUILD_DIR}"
