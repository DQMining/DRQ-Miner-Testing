# DRQ Miner — Windows Release Audit

Audit date: project tree static analysis + packaging script `scripts/package_windows_release.ps1`.

Target: **clean Windows 10/11 x64** with no Visual Studio, OpenSSL, or CUDA toolkit preinstalled.

---

## Build system summary

| Item | Result |
|------|--------|
| Build | CMake + Visual Studio 2022 (`x64-Release` preset) |
| CRT | **/MD** (MultiThreadedDLL) — requires VC++ 2015–2022 redistributable DLLs |
| vcpkg | **Not used** (no `vcpkg.json`) |
| NuGet | **Not used** (no `PackageReference` / `packages.config`) |
| OpenSSL | **Statically linked** into `DRQMiner-Release.exe` (`OPENSSL_USE_STATIC_LIBS=TRUE`, `cmake/OpenSSL.cmake`) |
| libuv | **Statically linked** (bundled `src/3rdparty/libuv`, `LIBUV_BUILD_SHARED=OFF`) |
| hwloc | **Statically linked** (bundled `src/3rdparty/hwloc` on MSVC) |
| GhostRider / Argon2 / RandomX / Verus CPU | **Statically linked** into executable |
| AstroSPSA (DERO fast path) | **Separate DLL** `xmrig_astro_spsa.dll` (clang++ gnu ABI) — **must ship** when built |

---

## OpenSSL usage

| Component | OpenSSL | Runtime DLL needed? |
|-----------|---------|------------------------|
| `DRQMiner-Release.exe` | TLS/HTTPS (`WITH_TLS=ON`) | **No** — static `.lib` link |
| `xmrig_astro_spsa.dll` | Linked with static `libcrypto.lib` when rebuilt; legacy builds import `libcrypto-3-x64__.dll` | **Yes** — ship `libcrypto-3-x64.dll` **and** alias `libcrypto-3-x64__.dll` |

Packaging script searches `C:\OpenSSL-Win64\bin`, `OPENSSL_ROOT_DIR`, and xmrig-deps paths and copies matching DLLs into `RELEASE/`.

---

## Dynamically loaded modules (`LoadLibrary` / `uv_dlopen`)

These are **not copied** into the release unless noted. They are loaded at runtime from the OS or optional plugins.

| Library | Loader | Required for | Ship? |
|---------|--------|--------------|-------|
| `xmrig-cuda.dll` | `CudaLib.cpp` | RandomX/KawPow CUDA (not embedded Verus) | **Optional** — separate download |
| `OpenCL.dll` | `OclLib.cpp` | OpenCL GPU mining | **No** — GPU driver |
| `nvml.dll` | `NvmlLib.cpp` | NVIDIA telemetry when CUDA enabled | **No** — NVIDIA driver |
| `atiadlxx.dll` | `AdlLib.cpp` | AMD ADL (OpenCL tuning) | **No** — AMD driver |
| `cudart64_*.dll` | Implicit + POST_BUILD copy | Embedded Verus CUDA | **Yes** when CUDA build |
| `xmrig_astro_spsa.dll` | PE import table | DERO AstroSPSA | **Yes** when built |

---

## Files that must ship with DRQ Miner (Windows x64)

### Always (CPU release)

| File | Purpose |
|------|---------|
| `DRQMiner-Release.exe` | Main miner |
| `verushash.cl` | Verus OpenCL kernel (beside exe) |
| `README.txt` | User instructions |
| `DEPENDENCY_REPORT.txt` | DLL audit from build |
| `MANIFEST.txt` | SHA256 checksums |
| `config.json.example` | Starter config (no secrets) |
| `vcruntime140.dll` | VC++ runtime (/MD) |
| `vcruntime140_1.dll` | VC++ runtime (VS2019+) |
| `msvcp140.dll` | C++ standard library runtime |

### When AstroSPSA built (typical desktop Release)

| File | Purpose |
|------|---------|
| `xmrig_astro_spsa.dll` | DERO suffix-array acceleration |
| `libcrypto-3-x64.dll` | OpenSSL for AstroSPSA (canonical name) |
| `libcrypto-3-x64__.dll` | **Required alias** for current AstroSPSA DLL import table (clang/lld quirk) |
| `libssl-3-x64.dll` | If DLL linked against ssl |
| `libstdc++-6.dll` | MinGW/clang runtime for DLL |
| `libwinpthread-1.dll` | pthread for DLL |
| `libgcc_s_seh-1.dll` | GCC unwinder for DLL |

### When CUDA Verus enabled at build time

| File | Purpose |
|------|---------|
| `cudart64_*.dll` | NVIDIA CUDA runtime (version matches toolkit used to build) |

### Optional / situational

| File | Purpose |
|------|---------|
| `WinRing0x64.sys` | MSR access for RandomX (`WITH_MSR`) — admin + driver load |
| `xmrig-cuda.dll` | Legacy CUDA plugin for CN/RX/KawPow — **not** in default package |

### Not shipped (system / driver)

All standard Windows DLLs (`kernel32.dll`, `ws2_32.dll`, `crypt32.dll`, `api-ms-win-*`, etc.).

---

## Typical PE dependency chain (Release)

```
DRQMiner-Release.exe
  ├── (static) libuv, hwloc, openssl, ghostrider, randomx, ...
  ├── xmrig_astro_spsa.dll          [if AstroSPSA built]
  │     ├── libcrypto-3-x64.dll
  │     ├── libstdc++-6.dll
  │     ├── libwinpthread-1.dll
  │     └── vcruntime140*.dll (via mingw/msvc mix)
  ├── cudart64_*.dll                [if CUDA Verus built]
  └── vcruntime140.dll, msvcp140.dll
```

Run `dumpbin /dependents` on the build machine for authoritative output — the packaging script does this automatically.

---

## Packaging workflow

### After every Release build (automatic)

CMake `POST_BUILD` invokes:

```powershell
powershell -File scripts/package_windows_release.ps1 -BuildDir out/build/x64-Release -Config Release -Quiet
```

### Manual full package + zip

```powershell
cmake --preset x64-Release
cmake --build out/build/x64-Release --config Release --target xmrig
cmake --build out/build/x64-Release --config Release --target package-windows-release
```

Or:

```powershell
pwsh -File scripts/package_windows_release.ps1
```

**Output:**

- `out/build/x64-Release/RELEASE/` — distributable folder  
- `out/build/x64-Release/DRQMiner-win64.zip` — zip of RELEASE contents  

### Verify (local or clean VM)

```powershell
pwsh -File scripts/verify_windows_release.ps1
pwsh -File scripts/verify_windows_release.ps1 -ReleaseDir out/build/x64-Release/RELEASE
```

**Clean VM checklist:**

1. Fresh Win10/11 x64, Windows Update only.
2. Copy `RELEASE` folder (or extract zip).
3. Run `verify_windows_release.ps1`.
4. Run `DRQMiner-Release.exe -V` and a short `--dry-run`.
5. Confirm no “missing DLL” dialog.

If VC++ DLLs were not bundled, install [Microsoft VC++ 2015–2022 Redistributable (x64)](https://learn.microsoft.com/en-us/cpp/windows/latest-supported-vc-redist) — packaging script attempts to copy them from the VS installation.

---

## Dependency report format

`RELEASE/DEPENDENCY_REPORT.txt` columns:

| Column | Meaning |
|--------|---------|
| PE | Binary scanned |
| Dependency | DLL name from `dumpbin /dependents` |
| SystemDll | True if Windows-provided |
| Included | `yes` / `MISSING` / `n/a (OS)` |
| Source | Path copied from |

Exit codes from `package_windows_release.ps1`:

- `0` — success  
- `2` — MISSING non-system DLLs  
- `3` — launch test failed  

---

## CI integration

`.github/workflows/release.yml` Windows job runs `package_windows_release.ps1` and uploads `DRQMiner-win64.zip` built from the full `RELEASE/` folder (not exe-only).

---

## Known limitations

1. **OpenCL / NVML / ADL** require end-user GPU drivers — not bundled.
2. **xmrig-cuda.dll** is a separate optional plugin for non-Verus CUDA algos.
3. **AstroSPSA DLL** requires matching OpenSSL + MinGW runtime DLLs on clean systems — packaging script copies them when found on the build machine.
4. **WinRing0** driver may trigger SmartScreen / admin prompts — optional for RandomX MSR tuning only.

---

## Related docs

- `doc/GPU_VERUS_NATIVE.md` — CUDA/OpenCL Verus
- `doc/V1_RELEASE_CHECKLIST.md` — release gates
- `scripts/templates/README.release.txt` — end-user README shipped in zip
