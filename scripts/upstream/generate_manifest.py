#!/usr/bin/env python3
"""Build manifest.json for DRQ Miner auto-updater from release artifact SHA256 sidecars."""

from __future__ import annotations

import argparse
import json
import re
import sys
from datetime import datetime, timezone
from pathlib import Path


REPO = "DQMining/DRQ-Miner-Testing"
DEFAULT_MANIFEST_URL = "https://www.dqmining.com/releases/manifest.json"


def read_sha256(path: Path) -> str:
    text = path.read_text(encoding="utf-8").strip()
    m = re.match(r"^([0-9a-fA-F]{64})", text)
    if not m:
        raise ValueError(f"invalid sha256 file: {path}")
    return m.group(1).lower()


def read_app_version(version_h: Path) -> str:
    for line in version_h.read_text(encoding="utf-8").splitlines():
        m = re.search(r'#define\s+APP_VERSION\s+"(.+)"', line)
        if m:
            return m.group(1)
    raise ValueError(f"APP_VERSION not found in {version_h}")


def channel_for_tag(tag: str) -> str:
    if tag.startswith("beta-") or "-beta" in tag.lower():
        return "beta"
    return "stable"


def main() -> int:
    parser = argparse.ArgumentParser(description="Generate DRQ Miner update manifest.json")
    parser.add_argument("--tag", required=True, help="Release tag, e.g. v0.0.1")
    parser.add_argument("--dist", required=True, type=Path, help="Directory with release artifacts")
    parser.add_argument("--version-h", type=Path, default=Path("src/version.h"))
    parser.add_argument("--out", type=Path, default=Path("manifest.json"))
    parser.add_argument("--repo", default=REPO)
    args = parser.parse_args()

    tag = args.tag.lstrip("v") if args.tag.startswith("v") else args.tag
    release_tag = args.tag if args.tag.startswith(("v", "beta-")) else f"v{args.tag}"
    base_url = f"https://github.com/{args.repo}/releases/download/{release_tag}"

    artifacts = {
        "win64": {
            "file": "DRQMiner-win64.zip",
            "sha_file": "DRQMiner-win64.zip.sha256",
        },
        "linux-x64": {
            "file": "drqminer-linux-x64.tar.gz",
            "sha_file": "drqminer-linux-x64.tar.gz.sha256",
        },
        "linux-arm64-phone": {
            "file": "drqminer-linux-arm64-phone.tar.gz",
            "sha_file": "drqminer-linux-arm64-phone.tar.gz.sha256",
        },
    }

    manifest_artifacts = {}
    for key, spec in artifacts.items():
        sha_path = args.dist / spec["sha_file"]
        if not sha_path.is_file():
            print(f"warning: missing {sha_path}, skipping {key}", file=sys.stderr)
            continue
        manifest_artifacts[key] = {
            "url": f"{base_url}/{spec['file']}",
            "sha256": read_sha256(sha_path),
        }

    if not manifest_artifacts:
        print("error: no artifacts found", file=sys.stderr)
        return 1

    version = read_app_version(args.version_h)
    manifest = {
        "channel": channel_for_tag(release_tag),
        "version": version,
        "tag": release_tag,
        "published_at": datetime.now(timezone.utc).strftime("%Y-%m-%dT%H:%M:%SZ"),
        "manifest_url": DEFAULT_MANIFEST_URL,
        "notes_url": f"https://github.com/{args.repo}/releases/tag/{release_tag}",
        "artifacts": manifest_artifacts,
    }

    args.out.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"wrote {args.out} version={version} channel={manifest['channel']}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
