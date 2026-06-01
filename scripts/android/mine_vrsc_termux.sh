#!/bin/bash
# CPU Verus on Termux. Edit POOL and WALLET, or pass args through.
# Expects: ~/drqminer or out/build/termux-arm64-Release/drqminer

set -euo pipefail

POOL="${POOL:-stratum+tcp://127.0.0.1:9999}"
WALLET="${WALLET:-YOUR_VRSC_ADDRESS}"
THREADS="${THREADS:-4}"

find_miner() {
  for c in \
    "${HOME}/drqminer" \
    "$(dirname "$0")/../../out/build/termux-arm64-Release/drqminer" \
    "$(dirname "$0")/../../out/build/termux-arm64-Release/xmrig"; do
    if [[ -x "$c" ]]; then
      echo "$c"
      return 0
    fi
  done
  return 1
}

MINER="$(find_miner)" || {
  echo "drqminer not found. Build: bash scripts/android/build_termux.sh"
  exit 1
}

DIR="$(dirname "$MINER")"
cd "$DIR"

if [[ ! -f verushash.cl ]]; then
  echo "warning: verushash.cl not in $DIR (OpenCL will fail; CPU OK)"
fi

if command -v termux-wake-lock >/dev/null 2>&1; then
  termux-wake-lock || true
  trap 'termux-wake-unlock 2>/dev/null || true' EXIT
fi

exec "$MINER" -a verus -o "$POOL" -u "$WALLET" -p x -t "$THREADS" "$@"
