#!/bin/bash
# Fully static linux-x64 release binary (matches phone prebuilt: wget and run).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${ROOT}/out/build/linux-x64-Release"
HWLOC_PREFIX="${ROOT}/.ci/hwloc-x64"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

bash "${ROOT}/scripts/ci/build_hwloc_x64.sh" "${HWLOC_PREFIX}"

cmake --preset linux-x64-Release \
  -DXMRIG_BUILD_TESTS=OFF \
  -DBUILD_STATIC=ON \
  -DXMRIG_DEPS="${HWLOC_PREFIX}" \
  -DHWLOC_INCLUDE_DIR="${HWLOC_PREFIX}/include" \
  -DHWLOC_LIBRARY="${HWLOC_PREFIX}/lib/libhwloc.a"

cmake --build "${BUILD_DIR}" --target xmrig -j"${JOBS}"

if ldd "${BUILD_DIR}/drqminer" 2>&1 | grep -q "not a dynamic executable"; then
  echo "OK: drqminer is fully static (no host libhwloc required)"
else
  echo "ERROR: drqminer still has dynamic library dependencies:" >&2
  ldd "${BUILD_DIR}/drqminer" >&2 || true
  exit 1
fi

file "${BUILD_DIR}/drqminer"
