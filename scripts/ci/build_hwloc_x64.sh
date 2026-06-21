#!/bin/bash
# Build static hwloc for linux/x64 portable prebuilts (no libhwloc.so on target rigs).
# Usage: bash scripts/ci/build_hwloc_x64.sh <install-prefix>

set -euo pipefail

PREFIX="${1:?install prefix required}"
HWLOC_VERSION="2.12.1"
SRC_DIR="${TMPDIR:-/tmp}/hwloc-${HWLOC_VERSION}"
TARBALL="${TMPDIR:-/tmp}/hwloc-${HWLOC_VERSION}.tar.gz"

if [ ! -d "${SRC_DIR}" ]; then
  curl -fsSL \
    "https://download.open-mpi.org/release/hwloc/v2.12/hwloc-${HWLOC_VERSION}.tar.gz" \
    -o "${TARBALL}"
  tar xzf "${TARBALL}" -C "${TMPDIR:-/tmp}"
fi

cd "${SRC_DIR}"
./configure \
  --prefix="${PREFIX}" \
  --disable-shared \
  --enable-static \
  --disable-io \
  --disable-libudev \
  --disable-libxml2 \
  --disable-netloc \
  --disable-cudart \
  --disable-opencl \
  --disable-nvml \
  --disable-rsmi
make -j"$(nproc)"
make install

echo "hwloc x64 static installed to ${PREFIX}"
