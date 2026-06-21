#!/bin/bash
# One-shot build deps for DRQ Miner on Ubuntu/Debian (1240Lv3 rigs, dev boxes).
set -euo pipefail

if [ "$(id -u)" -ne 0 ]; then
  echo "Run: sudo bash scripts/linux/install_build_deps.sh"
  exit 1
fi

export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y git curl ca-certificates build-essential cmake libssl-dev

echo "Note: prebuilt linux-x64 tarballs (v1.0.10+) are static — libhwloc-dev is only needed when building from source."

echo "Done. Build with:"
echo "  cmake --preset linux-x64-Release"
echo "  cmake --build out/build/linux-x64-Release -j\$(nproc)"
