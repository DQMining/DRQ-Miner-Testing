#!/bin/bash
# Push linux-arm64 build for Userland Ubuntu (glibc). Run on PC with adb or copy manually.
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/out/build/linux-arm64-Release/drqminer"
CL="${ROOT}/src/backend/opencl/cl/verus/verushash.cl"

if [[ ! -f "$BIN" ]]; then
  echo "Missing $BIN — build: cmake --preset linux-arm64-Release && cmake --build out/build/linux-arm64-Release"
  exit 1
fi

echo "Userland paths vary. Common options:"
echo "  1) adb push to /sdcard/Download/ then in Userland: cp to ~/drqminer"
echo "  2) scp from PC to Userland SSH (if enabled)"
echo "  3) Shared folder in Userland settings"
echo ""

if command -v adb >/dev/null 2>&1; then
  adb push "$BIN" /sdcard/Download/drqminer
  adb push "$CL" /sdcard/Download/verushash.cl
  echo "Pushed to /sdcard/Download/drqminer and verushash.cl"
  echo "In Userland Ubuntu:"
  echo "  cp /mnt/shared/Download/drqminer ~/drqminer   # path may differ"
  echo "  cp /mnt/shared/Download/verushash.cl ~/"
  echo "  chmod +x ~/drqminer && ~/drqminer -V"
else
  echo "adb not found — copy these files manually:"
  echo "  $BIN"
  echo "  $CL"
fi
