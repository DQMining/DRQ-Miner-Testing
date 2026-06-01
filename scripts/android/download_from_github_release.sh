#!/bin/bash
# Download DRQ Miner release assets from a PRIVATE GitHub repo (Userland / Termux).
#
# Required env:
#   GITHUB_TOKEN   fine-grained PAT with Contents:Read on the repo
#   GITHUB_USER    your GitHub username
#   GITHUB_REPO    default: DRQ-Miner-Testing
#   RELEASE_TAG    default: v1.0.0
#   ASSET_NAME     default: drqminer-linux-arm64-phone.tar.gz
#
# Usage:
#   export GITHUB_TOKEN=github_pat_...
#   export GITHUB_USER=YourName
#   ./download_from_github_release.sh

set -euo pipefail

GITHUB_REPO="${GITHUB_REPO:-DRQ-Miner-Testing}"
RELEASE_TAG="${RELEASE_TAG:-v1.0.0}"
ASSET_NAME="${ASSET_NAME:-drqminer-linux-arm64-phone.tar.gz}"

if [[ -z "${GITHUB_TOKEN:-}" ]]; then
  echo "Set GITHUB_TOKEN (fine-grained PAT, Contents read on ${GITHUB_REPO})"
  exit 1
fi
if [[ -z "${GITHUB_USER:-}" ]]; then
  echo "Set GITHUB_USER to your GitHub login"
  exit 1
fi

REPO_SLUG="${GITHUB_USER}/${GITHUB_REPO}"
API_ROOT="https://api.github.com/repos/${REPO_SLUG}/releases/tags/${RELEASE_TAG}"

echo "Fetching release ${RELEASE_TAG} from ${REPO_SLUG} ..."

JSON="$(curl -fsSL \
  -H "Authorization: Bearer ${GITHUB_TOKEN}" \
  -H "Accept: application/vnd.github+json" \
  -H "X-GitHub-Api-Version: 2022-11-28" \
  "${API_ROOT}")"

# Parse asset API URL without jq (python3 fallback)
ASSET_API_URL=""
if command -v jq >/dev/null 2>&1; then
  ASSET_API_URL="$(echo "$JSON" | jq -r --arg n "$ASSET_NAME" '.assets[] | select(.name==$n) | .url')"
else
  ASSET_API_URL="$(python3 -c "
import json, os, sys
data = json.load(sys.stdin)
name = os.environ['ASSET_NAME']
for a in data.get('assets', []):
    if a.get('name') == name:
        print(a['url'])
        sys.exit(0)
sys.exit(1)
" <<< "$JSON")"
fi

if [[ -z "$ASSET_API_URL" || "$ASSET_API_URL" == "null" ]]; then
  echo "Asset not found: ${ASSET_NAME}"
  echo "Available assets:"
  if command -v jq >/dev/null 2>&1; then
    echo "$JSON" | jq -r '.assets[].name'
  fi
  exit 1
fi

echo "Downloading ${ASSET_NAME} ..."
curl -fsSL \
  -H "Authorization: Bearer ${GITHUB_TOKEN}" \
  -H "Accept: application/octet-stream" \
  -H "X-GitHub-Api-Version: 2022-11-28" \
  -o "${ASSET_NAME}" \
  "${ASSET_API_URL}"

SHA_FILE="${ASSET_NAME}.sha256"
SHA_API_URL=""
if command -v jq >/dev/null 2>&1; then
  SHA_API_URL="$(echo "$JSON" | jq -r --arg n "$SHA_FILE" '.assets[] | select(.name==$n) | .url')"
fi
if [[ -n "$SHA_API_URL" && "$SHA_API_URL" != "null" ]]; then
  curl -fsSL \
    -H "Authorization: Bearer ${GITHUB_TOKEN}" \
    -H "Accept: application/octet-stream" \
    -o "${SHA_FILE}" \
    "${SHA_API_URL}"
  if command -v sha256sum >/dev/null 2>&1; then
    sha256sum -c "${SHA_FILE}" && echo "Checksum OK"
  fi
fi

echo ""
echo "Done: $(pwd)/${ASSET_NAME}"
echo "Extract: tar xzf ${ASSET_NAME}"
echo "Run:     chmod +x drqminer && ./drqminer -V"
echo ""
echo "DERO:    ./drqminer --daemon -a astrobwt/v3 -o wss://dero.rabidmining.com:10300 -u WALLET -p x -t 4"
