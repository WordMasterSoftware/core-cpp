#!/usr/bin/env bash

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
BUILD_TYPE="${BUILD_TYPE:-Release}"
CONAN_HOME_DIR="${PROJECT_ROOT}/.conan"

export CC="${CC:-gcc}"
export CXX="${CXX:-g++}"
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
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_TOOLCHAIN_FILE="${TOOLCHAIN_FILE}" \
    -DCMAKE_CXX_COMPILER="${CXX}"

cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}"
