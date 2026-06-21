# V1 GitHub release — readiness

Target: **tag `v1.0.8`** on GitHub (current) with downloadable binaries and honest feature matrix.

Last updated against tree state: **DRQ Miner 6.25.0** (`src/version.h`), release **v1.0.8** published.

---

## How close? (summary)

| Track | Readiness | Blocker |
|-------|-----------|---------|
| **Windows x64** (Verus + DERO Rabid) | **Ship-ready** | Desktop shares validated; CUDA/OpenCL Verus optional |
| **GitHub Release v1.0.8** | **Done** | CI green; win64 + linux-x64 + phone arm64 published |
| **Linux x64** | **Ship-ready** | CI builds; soak-test optional |
| **Phone Userland — DERO CPU** | **~95%** | v1.0.8 TNN path (SPSA + NEON); **device soak on S20** to confirm ~4 kH/s + shares |
| **Phone Userland — Verus CPU** | **~90%** | Built in phone slim preset; optional Rabid/VIPOR soak |
| **macOS arm64** | **~80%** | Main Build CI libuv issue; not blocking Release artifacts |
| **Phone GPU** | **Not v1** | OpenCL off in phone build |

**Bottom line:** Phase 1 **Verus + DERO desktop** is done. Phone DERO needs **v1.0.8 soak** on Userland (not v1.0.6/7). Then Phase 2 (**BC3 SHA3-256t**) starts — see `doc/PHASE2_BACKLOG.md`.

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

| Component | Phone (arm64) v1.0.8+ | Desktop Windows |
|-----------|------------------------|-----------------|
| AstroBWT v3 algorithm | **Yes** (TNN `branchComputeCPU_aarch64`) | Yes (Wolf AVX2 + SPSA) |
| Wolf SIMD fast path | **No** (AVX2 only) | Yes |
| AstroSPSA fast SA | **Yes** (static `.a`) | Yes (DLL if present) |
| Suffix array | **SPSA** (fallback divsufsort if SPSA off) | SPSA or divsufsort |
| Rabid `wss://` + `--daemon` | **Yes** (TLS + HTTP in build) | Yes |
| Expected speed | **~3.5–4.2 kH/s** (S20-class, `-t 8`) | High on 14900K etc. |

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
