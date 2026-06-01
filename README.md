# DRQ Miner

Fork of [XMRig](https://github.com/xmrig/xmrig) focused on **VerusHash (VRSC)** and **AstroBWT v3 (DERO)**, with upstream CPU/OpenCL/CUDA backends for other algorithms. Product site: [www.dqmining.com](https://www.dqmining.com).

## Mining backends

| Backend | VerusHash (VRSC) | AstroBWT v3 (DERO) | Other algos |
|---------|------------------|--------------------|-------------|
| **CPU** | Full v2 hash, optimized | Full v3 hash, optimized SA path | Upstream XMRig set |
| **CUDA (NVIDIA)** | **Embedded** ccminer kernel when built with **nvcc** (no `xmrig-cuda.dll` required for Verus) | — | RandomX/KawPow/CN via `xmrig-cuda` plugin |
| **OpenCL** | **GPU** kernel ([AMDVerusCoin](https://github.com/monkins1010/AMDVerusCoin) `input.cl`) on AMD/Intel | — | Upstream where enabled |

### Verus CUDA (embedded)

When CMake finds the CUDA toolkit (`nvcc`), the build defines `XMRIG_FEATURE_CUDA_VERUS` and links the embedded kernel from [monkins1010/ccminer](https://github.com/monkins1010/ccminer) `verus2.2gpu`. Run with:

```text
drqminer -a verushash -o POOL:5040 -u WALLET.WORKER -p x --no-cpu --cuda
```

Keep **`cudart64_*.dll`** (Windows) next to the exe. Remove or rename mismatched **`xmrig-cuda.dll`** if you only want embedded Verus. See [doc/GPU_VERUS_NATIVE.md](doc/GPU_VERUS_NATIVE.md).

Smoke / launch (Windows, after Release build):

- `scripts\mine_vrsc_cpu.bat` — CPU on VIPOR
- `scripts\mine_vrsc_cuda.bat` — embedded NVIDIA CUDA
- `scripts\mine_vrsc_opencl.bat` — OpenCL GPU (AMD/Intel)
- `scripts\vipor_cuda_smoke.bat` — quick CUDA smoke
- `out\build\x64-Release\Release\test_verushash.exe` — self-test + `--bench-mt 32`

## Build

See [doc/build/CMAKE_OPTIONS.md](doc/build/CMAKE_OPTIONS.md).

| Platform | Preset | Output binary |
|----------|--------|---------------|
| Windows x64 | `x64-Release` | `out/build/x64-Release/DRQMiner-Release.exe` |
| Linux x64 | `linux-x64-Release` | `out/build/linux-x64-Release/drqminer` |
| Linux ARM64 | `linux-arm64-Release` | `out/build/linux-arm64-Release/drqminer` |
| macOS ARM64 | `macos-arm64-Release` | `out/build/macos-arm64-Release/drqminer` |
| Windows + GRID K2 CUDA | `x64-Release-verus-kepler` | `out/build/x64-Release-verus-kepler/DRQMiner-Release.exe` |
| Android arm64 (Termux, experimental) | `android-arm64-Release` | `out/build/android-arm64-Release/drqminer` |
| Phone via **Userland** Ubuntu | `linux-arm64-Release` | [TERMUX_USERLAND](doc/TERMUX_USERLAND_ANDROID.md) · DERO CPU · [v1 checklist](doc/V1_RELEASE_CHECKLIST.md) |

Use **Release** for mining; Debug is much slower.

```bash
cmake --preset x64-Release
cmake --build out/build/x64-Release --config Release
```

## Configure

Same JSON and CLI model as XMRig. Example: `-a verushash` or coin `VRSC` with a Verus stratum URL.

Default dev donation is **1%** to `pool.dqmining.com:5040` (override with `donate-level` or `src/donate.h`).

## Publishing to GitHub

See [doc/PUBLISHING.md](doc/PUBLISHING.md). Do not commit `config.json` with wallets or API keys.

## Upstream

DRQ Miner inherits XMRig’s license and architecture. VerusHash notes: [doc/verushash-porting-summary.md](doc/verushash-porting-summary.md). Phase 2 roadmap: [doc/PHASE2_BACKLOG.md](doc/PHASE2_BACKLOG.md).
