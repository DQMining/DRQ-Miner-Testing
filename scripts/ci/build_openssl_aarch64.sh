#!/bin/bash
# Cross-build static OpenSSL for linux/arm64 (Userland phone CI).
# Usage: bash scripts/ci/build_openssl_aarch64.sh <install-prefix>

set -euo pipefail

PREFIX="${1:?install prefix required}"
OPENSSL_VER="openssl-3.0.15"
SRC_DIR="${TMPDIR:-/tmp}/${OPENSSL_VER}"

if [ ! -d "${SRC_DIR}" ]; then
  curl -fsSL "https://www.openssl.org/source/${OPENSSL_VER}.tar.gz" | tar xz -C "${TMPDIR:-/tmp}"
fi

cd "${SRC_DIR}"
./Configure linux-aarch64 \
  --prefix="${PREFIX}" \
  --openssldir="${PREFIX}" \
  --cross-compile-prefix=aarch64-linux-gnu- \
  no-shared no-tests
make -j"$(nproc)"
make install_sw

echo "OpenSSL aarch64 installed to ${PREFIX}"
