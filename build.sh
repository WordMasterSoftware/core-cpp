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
BUILD_JOBS="${BUILD_JOBS:-$(nproc)}"
CONAN_VERSION="2.10.1"
CONAN_DEB_URL="https://github.com/conan-io/conan/releases/download/2.10.1/conan-2.10.1-amd64.deb"
CONAN_MANUAL_URL="https://conan.org.cn/downloads"

install_conan_if_missing() {
    if command -v conan >/dev/null 2>&1; then
        return
    fi

    local os_id=""
    local architecture=""
    local download_path="/tmp/conan-${CONAN_VERSION}-amd64.deb"
    local sudo_cmd=""

    if [[ -r /etc/os-release ]]; then
        # shellcheck disable=SC1091
        source /etc/os-release
        os_id="${ID:-}"
    fi

    architecture="$(uname -m)"

    if [[ "${os_id}" != "ubuntu" && "${os_id}" != "debian" ]]; then
        echo "Conan is not installed. Please install it manually from ${CONAN_MANUAL_URL}" >&2
        exit 1
    fi

    if [[ "${architecture}" != "x86_64" && "${architecture}" != "amd64" ]]; then
        echo "Conan is not installed. Automatic installation only supports Ubuntu/Debian amd64. Please install it manually from ${CONAN_MANUAL_URL}" >&2
        exit 1
    fi

    if command -v curl >/dev/null 2>&1; then
        echo "Conan was not found. Downloading Conan ${CONAN_VERSION} package..."
        curl -fL "${CONAN_DEB_URL}" -o "${download_path}"
    elif command -v wget >/dev/null 2>&1; then
        echo "Conan was not found. Downloading Conan ${CONAN_VERSION} package..."
        wget -O "${download_path}" "${CONAN_DEB_URL}"
    else
        echo "Conan is not installed and neither curl nor wget is available. Please install Conan manually from ${CONAN_MANUAL_URL}" >&2
        exit 1
    fi

    if [[ "${EUID}" -ne 0 ]]; then
        if command -v sudo >/dev/null 2>&1; then
            sudo_cmd="sudo"
        else
            echo "Conan package downloaded to ${download_path}, but sudo is not available. Please install it manually from ${CONAN_MANUAL_URL}" >&2
            exit 1
        fi
    fi

    echo "Installing Conan ${CONAN_VERSION}..."
    ${sudo_cmd} dpkg -i "${download_path}"

    if ! command -v conan >/dev/null 2>&1; then
        echo "Conan installation completed, but the 'conan' command is still unavailable. Please verify the installation manually: ${CONAN_MANUAL_URL}" >&2
        exit 1
    fi
}

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
export CMAKE_BUILD_PARALLEL_LEVEL="${BUILD_JOBS}"
export CONAN_CPU_COUNT="${BUILD_JOBS}"
export MAKEFLAGS="${MAKEFLAGS:--j${BUILD_JOBS}}"

install_conan_if_missing

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

cmake --build "${BUILD_DIR}" --config "${BUILD_TYPE}" --parallel "${BUILD_JOBS}"
