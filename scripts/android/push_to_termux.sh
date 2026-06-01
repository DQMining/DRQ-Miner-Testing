#!/bin/bash
# Push android-arm64 NDK build into Termux home (run on PC with adb).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
BIN="${ROOT}/out/build/android-arm64-Release/drqminer"
CL="${ROOT}/src/backend/opencl/cl/verus/verushash.cl"
TERMUX_HOME="/data/data/com.termux/files/home"

if [[ ! -f "$BIN" ]]; then
  echo "Missing $BIN — build: cmake --preset android-arm64-Release && cmake --build out/build/android-arm64-Release"
  exit 1
fi

adb push "$BIN" "${TERMUX_HOME}/drqminer"
adb push "$CL" "${TERMUX_HOME}/verushash.cl"
adb shell chmod 755 "${TERMUX_HOME}/drqminer"

echo "Done. In Termux: ~/drqminer -a verus -o POOL -u WALLET -p x -t 4"
