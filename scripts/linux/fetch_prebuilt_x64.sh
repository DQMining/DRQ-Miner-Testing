#!/bin/bash
# Download and run DRQ Miner linux-x64 prebuilt (nm/1 + Verus + DERO).
set -euo pipefail

TAG="${1:-v1.0.9}"
WALLET="${2:-YOUR_WALLET}"
POOL="${3:-eu.hashmonkeys.cloud:12427}"

URL="https://github.com/DQMining/DRQ-Miner-Testing/releases/download/${TAG}/drqminer-linux-x64.tar.gz"
DIR="${HOME}/Miners/drqminer-${TAG}"

mkdir -p "${DIR}"
cd "${DIR}"
curl -fsSL -o drqminer-linux-x64.tar.gz "${URL}"
tar xzf drqminer-linux-x64.tar.gz
chmod +x drqminer
./drqminer -V
echo
echo "Mining nm/1 (example):"
echo "  ./drqminer -o ${POOL} -u ${WALLET} -a nm/1"
