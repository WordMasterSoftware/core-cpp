#!/usr/bin/env bash

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${BUILD_DIR:-${PROJECT_ROOT}/build}"
BUILD_TYPE="${BUILD_TYPE:-Release}"
CONAN_HOME_DIR="${CONAN_HOME_DIR:-${PROJECT_ROOT}/.conan}"
GCC_ROOT="${GCC_ROOT:-/usr/local/gcc-15.2.0}"
GCC_BIN_DIR="${GCC_ROOT}/bin"
DEFAULT_GCC="${GCC_BIN_DIR}/gcc"
DEFAULT_GXX="${GCC_BIN_DIR}/g++"

# 优先切换到 GCC 15.2.0，确保 Conan、CMake 与实际编译阶段使用同一套工具链。
if [[ -x "${DEFAULT_GCC}" && -x "${DEFAULT_GXX}" ]]; then
    export CC="${CC:-${DEFAULT_GCC}}"
    export CXX="${CXX:-${DEFAULT_GXX}}"
    export PATH="${GCC_BIN_DIR}:${PATH}"
else
    echo "GCC 15.2.0 was not found under ${GCC_ROOT}. Set GCC_ROOT, CC and CXX explicitly before building." >&2
    exit 1
fi

export CONAN_HOME="${CONAN_HOME_DIR}"

mkdir -p "${BUILD_DIR}"

conan profile detect --force

conan install "${PROJECT_ROOT}" \
    --output-folder "${BUILD_DIR}" \
    --build=missing \
    --settings build_type="${BUILD_TYPE}" \
    --settings compiler=gcc \
    --settings compiler.version=15 \
    --settings compiler.cppstd=20 \
    --settings compiler.libcxx=libstdc++11

TOOLCHAIN_FILE="$(find "${BUILD_DIR}" -name conan_toolchain.cmake -print -quit)"

if [[ -z "${TOOLCHAIN_FILE}" ]]; then
    echo "Unable to locate conan_toolchain.cmake under ${BUILD_DIR}" >&2
    exit 1
fi

cmake -S "${PROJECT_ROOT}" -B "${BUILD_DIR}" \
    --fresh \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_CXX_COMPILER="${CXX}"

cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"
