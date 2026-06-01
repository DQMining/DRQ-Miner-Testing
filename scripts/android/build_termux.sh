#!/bin/bash
# Native DRQ Miner build inside Termux (aarch64, bionic).
# Usage: cd <repo-root> && bash scripts/android/build_termux.sh

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${ROOT}/out/build/termux-arm64-Release"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

if ! command -v cmake >/dev/null 2>&1; then
  echo "Install deps: pkg install -y git cmake clang make openssl libuv"
  exit 1
fi

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${ROOT}" \
  -DCMAKE_BUILD_TYPE=Release \
  -DARM_V8=ON \
  -DWITH_CUDA=OFF \
  -DWITH_NVML=OFF \
  -DWITH_MSR=OFF \
  -DWITH_DMI=OFF \
  -DWITH_GHOSTRIDER=OFF \
  -DWITH_KAWPOW=OFF \
  -DWITH_RANDOMX=OFF \
  -DWITH_CN_LITE=OFF \
  -DWITH_CN_HEAVY=OFF \
  -DWITH_CN_PICO=OFF \
  -DWITH_CN_FEMTO=OFF \
  -DWITH_ARGON2=OFF \
  -DWITH_ASTROBWT=ON \
  -DWITH_ASTRO_SPSA=OFF \
  -DWITH_VERUSHASH=ON \
  -DWITH_OPENCL=ON \
  -DWITH_HTTP=ON \
  -DWITH_TLS=ON \
  -DWITH_HWLOC=OFF \
  -DXMRIG_BUILD_TESTS=OFF

cmake --build . -j"${JOBS}"

cp -f "${ROOT}/src/backend/opencl/cl/verus/verushash.cl" "${BUILD_DIR}/"
chmod +x "${BUILD_DIR}/drqminer" 2>/dev/null || chmod +x "${BUILD_DIR}/xmrig" 2>/dev/null || true

echo ""
echo "Built: ${BUILD_DIR}/drqminer (or xmrig — check directory listing)"
echo "Copy verushash.cl is already beside the binary."
ls -la "${BUILD_DIR}/"drqminer "${BUILD_DIR}/"xmrig 2>/dev/null || ls -la "${BUILD_DIR}/"
