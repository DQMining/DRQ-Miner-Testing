#!/bin/bash
# Download aarch64 AstroSPSA prebuilt for phone CI (headers live in lib/astrospsa/).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
LIBDIR="${ROOT}/lib/astrospsa"
LIB="libastroSPSA_linux_aarch64_clang_18_armv8-a+crypto.a"
TAG="8938667bfa3253b52c622daf575fc11674d7067b"

mkdir -p "${LIBDIR}"

if [ -f "${LIBDIR}/${LIB}" ]; then
  echo "OK: ${LIBDIR}/${LIB} already present"
  exit 0
fi

URL="https://gitlab.com/Tritonn204/astro-spsa/-/raw/${TAG}/${LIB}"
echo "Fetching ${URL} ..."
curl -fsSL -o "${LIBDIR}/${LIB}" "${URL}"
ls -la "${LIBDIR}/${LIB}"
