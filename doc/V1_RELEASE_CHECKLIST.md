# V1 GitHub release — readiness

Target: **tag `v1.0.0`** on GitHub with downloadable binaries and honest feature matrix.

Last updated against tree state: **DRQ Miner 6.25.0** (`src/version.h`).

---

## How close? (summary)

| Track | Readiness | Blocker |
|-------|-----------|---------|
| **Windows x64** (Verus + DERO Rabid) | **Ship-ready** | You already validated shares on desktop |
| **GitHub Release v1.0.0** (process) | **~1–2 days** | Tag, release notes, upload 3–4 zip artifacts, SHA256 |
| **Linux x64 / macOS arm64** | **~90%** | CI builds; soak-test DERO/Verus per OS |
| **Phone Userland — DERO CPU** | **~70%** | Code path exists (divsufsort); **needs real device soak test** |
| **Phone Userland — Verus CPU** | **~75%** | Same |
| **Phone GPU** | **Not v1** | OpenCL often missing; do not promise in v1 notes |

**Bottom line:** You can ship **v1.0.0 for Windows x64 today** with Verus + DERO called out as tested. For **phones + DERO**, treat v1 as **“CPU mining supported, beta on arm64”** after one successful Rabid run on your Userland device—or ship **v1.0.0** desktop now and **v1.1.0** once phone DERO is confirmed.

---

## What v1 should include

### Binaries (minimum)

| Asset | CMake preset / script | v1? |
|-------|----------------------|-----|
| `DRQMiner-6.25.0-win64.zip` | `x64-Release` + `xmrig_astro_spsa.dll` + `verushash.cl` | **Yes** |
| `drqminer-linux-x64.tar.gz` | `linux-x64-Release` | **Yes** |
| `drqminer-macos-arm64.tar.gz` | `macos-arm64-Release` | **Yes** |
| `drqminer-linux-arm64-phone.tar.gz` | `scripts/android/build_userland.sh` or full `linux-arm64-Release` | **Yes (beta)** |
| Termux `android-arm64` | `android-arm64-Release` | Optional v1.0.1 |

### Docs in release notes

- Windows: Verus CPU/CUDA/OpenCL, DERO `--daemon` + `wss://` Rabid example
- Phone (Userland): link `doc/TERMUX_USERLAND_ANDROID.md`, **DERO CPU** command below
- Phone: no AstroSPSA / no desktop kH/s expectations
- K2/Kepler: OpenCL or Kepler CUDA preset (see `doc/LEGACY_GPU_AND_ANDROID.md`)

### CI (recommended before tag)

- [ ] `test_astrobwt` passes on **macOS arm64** (validates DERO hash on ARM without SPSA)
- [ ] `test_verushash` passes on Windows + Linux (if present)
- [ ] Optional: `ubuntu-24.04-arm` job for `linux-arm64-Release`

### Legal / repo hygiene

- [ ] `LICENSE` / GPLv3 notice visible
- [ ] No committed pool wallets or secrets
- [ ] `README.md` install + Rabid DERO one-liner

---

## DERO on phone CPU — technical status

| Component | Phone (arm64) | Desktop Windows |
|-----------|---------------|-----------------|
| AstroBWT v3 algorithm | **Yes** (portable C++ loop) | Yes |
| Wolf SIMD fast path | **No** (AVX2 only) | Yes |
| AstroSPSA fast SA | **No** (x86 DLL / lib) | Yes (if DLL present) |
| Suffix array | **divsufsort** (consensus-correct) | SPSA or divsufsort |
| Rabid `wss://` + `--daemon` | **Yes** (TLS + HTTP in build) | Yes |
| Expected speed | Low (thermal-limited) | High on 14900K etc. |

**Phone build must have:** `-DWITH_ASTROBWT=ON -DWITH_HTTP=ON -DWITH_TLS=ON`  
Use `scripts/android/build_userland.sh` or full `linux-arm64-Release` preset.

**Smoke test on Userland (you):**

```bash
export WALLET=dero1q...
./drqminer --daemon -a astrobwt/v3 -o wss://dero.rabidmining.com:10300 -u "$WALLET" -p x -t 4
```

Success = connects, hashes, **accepted share** within a reasonable window (pool luck applies).

Optional self-test before mining:

```bash
./test_astrobwt    # if built; expect 6/6 vectors
```

---

## Suggested release versioning

| Tag | Contents |
|-----|----------|
| **v1.0.0** | Windows x64 + linux x64 + macOS arm64; release notes: phone DERO “experimental, report issues” |
| **v1.0.1** | Add `linux-arm64` phone zip after your Userland DERO soak test |
| **v1.1.0** | GitHub Actions release workflow + checksum manifest |

---

## Release command sketch (maintainer)

**Private repo setup:** [GITHUB_PRIVATE_SETUP.md](GITHUB_PRIVATE_SETUP.md) — repo name `DRQ-Miner-Testing`, display “DRQ Miner Testing”.

```powershell
# One-time: gh auth login, then:
powershell -ExecutionPolicy Bypass -File scripts\setup_github_private.ps1

# CI builds phone + desktop artifacts:
gh workflow run Release
# Or tagged release:
git tag v1.0.0
git push origin v1.0.0
```

Phone download (private, needs PAT on device):  
`https://github.com/YOU/DRQ-Miner-Testing/releases/download/v1.0.0/drqminer-linux-arm64-phone.tar.gz`

---

## Related

- [PHASE2_BACKLOG.md](PHASE2_BACKLOG.md)
- [TERMUX_USERLAND_ANDROID.md](TERMUX_USERLAND_ANDROID.md)
- Rabid DERO example in [PHASE2_BACKLOG.md](PHASE2_BACKLOG.md#dero-astrobwt-v3--test-now-phase-1)
