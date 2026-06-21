#!/bin/bash
# Slim phone/desktop-arm64 build for Userland Ubuntu (glibc). Keeps DERO + Verus CPU.
# Usage: cd <repo-root> && bash scripts/android/build_userland.sh
# CI cross-build: CROSS_AARCH64=1 bash scripts/android/build_userland.sh

set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BUILD_DIR="${ROOT}/out/build/linux-arm64-phone-Release"
JOBS="${JOBS:-$(nproc 2>/dev/null || echo 4)}"

mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

CMAKE_ARGS=(
  -DCMAKE_BUILD_TYPE=Release
  -DARM_V8=ON
  -DWITH_CUDA=OFF
  -DWITH_NVML=OFF
  -DWITH_MSR=OFF
  -DWITH_DMI=OFF
  -DWITH_GHOSTRIDER=OFF
  -DWITH_KAWPOW=OFF
  -DWITH_RANDOMX=OFF
  -DWITH_CN_LITE=OFF
  -DWITH_CN_HEAVY=OFF
  -DWITH_CN_PICO=OFF
  -DWITH_CN_FEMTO=OFF
  -DWITH_ARGON2=OFF
  -DWITH_ASTROBWT=ON
  -DWITH_ASTRO_SPSA=ON
  -DWITH_VERUSHASH=ON
  -DWITH_NM=ON
  -DWITH_OPENCL=OFF
  -DWITH_HTTP=ON
  -DWITH_TLS=ON
  -DWITH_HWLOC=OFF
  -DXMRIG_BUILD_TESTS=OFF
  -DBUILD_STATIC=ON
)

if [ -n "${CROSS_AARCH64:-}" ]; then
  CMAKE_ARGS+=(
    -DCMAKE_SYSTEM_NAME=Linux
    -DCMAKE_SYSTEM_PROCESSOR=aarch64
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++
    -DCMAKE_C_FLAGS="-march=armv8-a+crypto -flax-vector-conversions"
    -DCMAKE_CXX_FLAGS="-march=armv8-a+crypto -flax-vector-conversions"
  )
  if [ -n "${OPENSSL_AARCH64_PREFIX:-}" ]; then
    CMAKE_ARGS+=(
      -DOPENSSL_ROOT_DIR="${OPENSSL_AARCH64_PREFIX}"
      -DOPENSSL_INCLUDE_DIR="${OPENSSL_AARCH64_PREFIX}/include"
      -DOPENSSL_CRYPTO_LIBRARY="${OPENSSL_AARCH64_PREFIX}/lib/libcrypto.a"
      -DOPENSSL_SSL_LIBRARY="${OPENSSL_AARCH64_PREFIX}/lib/libssl.a"
    )
  fi
fi

cmake "${ROOT}" "${CMAKE_ARGS[@]}"
cmake --build . -j"${JOBS}"

if command -v ldd >/dev/null 2>&1 && ldd "${BUILD_DIR}/drqminer" 2>&1 | grep -q "not a dynamic executable"; then
  echo "OK: drqminer is fully static (no host glibc required)"
elif command -v ldd >/dev/null 2>&1; then
  echo "WARNING: drqminer is not fully static — Userland may need newer glibc:" >&2
  ldd "${BUILD_DIR}/drqminer" >&2 || true
fi

cp -f "${ROOT}/src/backend/opencl/cl/verus/verushash.cl" "${BUILD_DIR}/" 2>/dev/null || true

echo ""
echo "Userland binary: ${BUILD_DIR}/drqminer"
file "${BUILD_DIR}/drqminer" 2>/dev/null || true
ls -la "${BUILD_DIR}/drqminer" 2>/dev/null || ls -la "${BUILD_DIR}/"
