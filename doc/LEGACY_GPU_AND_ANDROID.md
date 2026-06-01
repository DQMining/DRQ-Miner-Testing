# Legacy NVIDIA (GRID K2 / Kepler) and Android

DRQ Miner targets desktop first. This document covers **experimental** support for older NVIDIA GRID/K2 cards and **Android** phone CPU/GPU mining.

## Support matrix

| Platform | Verus CPU | Verus OpenCL GPU | Verus CUDA GPU |
|----------|-----------|------------------|----------------|
| Windows x64 (modern) | Yes | Yes (AMD/Intel/NVIDIA) | Yes (sm_61+, CUDA 12/13) |
| GRID K2 / Kepler (sm_35) | Yes | **Try `--opencl` first** | Yes only with **CUDA 11.8** + Kepler preset |
| Android arm64-v8a | Yes (ARM) | **Experimental** (Adreno/Mali if driver exposes OpenCL) | No |
| Android arm32 | Not preset | Not recommended | No |

AstroBWT v3 on Android is **not** supported (fast path uses an x64 Windows DLL).

---

## NVIDIA GRID K2 / Kepler (sm_30–sm_52)

CUDA **13.x cannot compile** for Kepler. Use **CUDA Toolkit 11.8** (last release with good sm_35 support).

### Option A — OpenCL (simplest on K2)

Many K2 hosts expose OpenCL on the same driver as CUDA. No special CUDA arch needed.

```bat
cd out\build\x64-Release
DRQMiner-Release.exe --no-cpu --no-cuda --opencl -a verus -o POOL -u WALLET -p x
```

Ensure `verushash.cl` sits next to the executable (Release post-build copy).

Intensity is capped automatically on GPUs with &lt; 2 GiB global memory.

### Option B — Embedded CUDA for Kepler

1. Install [CUDA 11.8](https://developer.nvidia.com/cuda-11-8-0-download-archive) and set `CUDA_PATH` to that install (or use `XMRIG_CUDA_TOOLKIT_ROOT`).
2. Configure with the Kepler preset (sets `CMAKE_CUDA_ARCHITECTURES` to 35 37 50 52 61):

```bat
cmake --preset x64-Release-verus-kepler
cmake --build out\build\x64-Release-verus-kepler --config Release
```

3. Copy `cudart64_110.dll` (or matching 11.x runtime) beside `DRQMiner-Release.exe`.
4. Mine with `--cuda` only (no `xmrig-cuda.dll`).

If configure still picks CUDA 13, set explicitly:

```bat
set XMRIG_CUDA_TOOLKIT_ROOT=C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v11.8
cmake --preset x64-Release-verus-kepler
```

---

## Android (cellphone CPU and GPU)

**Termux / Userland:** see **[TERMUX_USERLAND_ANDROID.md](TERMUX_USERLAND_ANDROID.md)** — Userland Ubuntu uses **`linux-arm64-Release`**; Termux uses **NDK android-arm64** or native Termux build (`scripts/android/`).

### What works today

- **CPU Verus** on **arm64-v8a** (64-bit Android 7+).
- **OpenCL Verus** only if the device ships an OpenCL ICD (some Qualcomm Adreno builds do; many phones do not expose OpenCL to user apps).
- **No CUDA** on phones.
- Thermal and battery limits are your responsibility; use low thread count (`-t 2` or `-t 4`).

### Build (NDK r26+ recommended)

1. Install Android NDK and set `ANDROID_NDK_ROOT` (or `ANDROID_NDK_HOME`).
2. From the repo root:

```bash
cmake --preset android-arm64-Release
cmake --build out/build/android-arm64-Release
```

3. Push the binary to the device (root, Termux, or your own APK wrapper):

```bash
adb push out/build/android-arm64-Release/drqminer /data/local/tmp/
adb shell chmod +x /data/local/tmp/drqminer
```

4. Copy `verushash.cl` next to the binary if using OpenCL.

### Example commands on device

CPU only:

```text
./drqminer -a verus -o stratum+tcp://POOL:PORT -u WALLET -p x -t 4
```

OpenCL (if `clinfo` or the miner lists a GPU platform):

```text
./drqminer --no-cpu --opencl -a verus -o stratum+tcp://POOL:PORT -u WALLET -p x
```

### arm32 (old 32-bit phones)

Not in presets. Use `ANDROID_ABI=armeabi-v7a` manually if required; Verus CPU may work but is slow and untested.

### AstroBWT / RandomX on phone

RandomX and AstroBWT are heavy on mobile RAM and heat. Prefer **Verus only** on Android builds (preset disables GhostRider/KawPow).

---

## CMake options

| Option / preset | Purpose |
|-----------------|--------|
| `XMRIG_VERUS_CUDA_LEGACY_KEPLER=ON` | sm_35–sm_61 for GRID K2 / Kepler |
| `x64-Release-verus-kepler` | Windows x64 + legacy CUDA arches |
| `android-arm64-Release` | NDK arm64, CPU + OpenCL Verus |

---

## Troubleshooting

| Symptom | Likely cause |
|---------|----------------|
| CUDA configure fails on K2 build | CUDA 13 installed; switch to 11.8 and Kepler preset |
| OpenCL `CL_OUT_OF_RESOURCES` on phone | Intensity too high; auto cap should help; lower manual intensity in config |
| No OpenCL devices on Android | OEM disabled OpenCL; use CPU only |
| `verushash.cl` not found | Copy from `src/backend/opencl/cl/verus/verushash.cl` beside binary |
