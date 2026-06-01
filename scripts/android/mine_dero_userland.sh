#!/bin/bash
# DERO AstroBWT v3 on Userland / Linux arm64 (Rabid WSS). Edit WALLET or export it.
# Requires: drqminer built with WITH_ASTROBWT=ON (see build_userland.sh or linux-arm64-Release).

set -euo pipefail

POOL="${POOL:-wss://dero.rabidmining.com:10300}"
WALLET="${WALLET:-YOUR_DERO_WALLET}"
THREADS="${THREADS:-4}"

find_miner() {
  for c in \
    "${HOME}/drqminer" \
    "$(dirname "$0")/../../out/build/linux-arm64-phone-Release/drqminer" \
    "$(dirname "$0")/../../out/build/linux-arm64-Release/drqminer"; do
    if [[ -x "$c" ]]; then
      echo "$c"
      return 0
    fi
  done
  return 1
}

MINER="$(find_miner)" || {
  echo "drqminer not found. Build: bash scripts/android/build_userland.sh"
  exit 1
}

exec "$MINER" --daemon -a astrobwt/v3 -o "$POOL" -u "$WALLET" -p x -t "$THREADS" "$@"
