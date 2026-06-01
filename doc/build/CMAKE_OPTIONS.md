# CMake options
**Recent version of this document: https://xmrig.com/docs/miner/cmake-options**

On MSVC, the main executable is named **`DRQMiner-<Config>.exe`** (for example `DRQMiner-Release.exe`) in the build directory.

On Linux, macOS, and other Unix targets the binary is **`drqminer`** in the preset build directory (for example `out/build/linux-x64-Release/drqminer`).

## CMake presets (DRQ Miner)

| Preset | Platform |
|--------|----------|
| `x64-Release` | Windows x64 (Visual Studio 2022) |
| `linux-x64-Release` | Linux x64 |
| `linux-arm64-Release` | Linux ARM64 (`-DARM_V8=ON`) |
| `macos-arm64-Release` | macOS Apple Silicon |
| `x64-Release-verus-kepler` | Windows x64 + Verus CUDA for GRID K2 / Kepler (**CUDA 11.8**, not 13+) |
| `android-arm64-Release` | Android NDK arm64 (experimental Verus CPU + OpenCL) |

See also `doc/LEGACY_GPU_AND_ANDROID.md` and `doc/TERMUX_USERLAND_ANDROID.md` (Termux vs Userland).

Example:

```bash
cmake --preset linux-arm64-Release
cmake --build out/build/linux-arm64-Release
./out/build/linux-arm64-Release/drqminer -V
```

ARM64 builds use `-march=armv8-a+crypto` when supported (`XMRIG_ARM_CRYPTO`), including accelerated SHA256 for AstroBWT v3.

## Algorithms

* **`-DWITH_CN_LITE=OFF`** disable all CryptoNight-Lite algorithms (`cn-lite/0`, `cn-lite/1`).
* **`-DWITH_CN_HEAVY=OFF`** disable all CryptoNight-Heavy algorithms (`cn-heavy/0`, `cn-heavy/xhv`, `cn-heavy/tube`).
* **`-DWITH_CN_PICO=OFF`** disable CryptoNight-Pico algorithm (`cn-pico`).
* **`-DWITH_RANDOMX=OFF`** disable RandomX algorithms (`rx/loki`, `rx/wow`).
* **`-DWITH_ARGON2=OFF`** disable Argon2 algorithms (`argon2/chukwa`, `argon2/wrkz`).

## Features

* **`-DWITH_HWLOC=OFF`**
disable [hwloc](https://github.com/xmrig/xmrig/issues/1077) support.
Disabling this feature is not recommended in most cases.
This feature add external dependency to libhwloc (1.10.0+) (except MSVC builds).
* **`-DWITH_LIBCPUID=OFF`** disable built in libcpuid support, this feature always disabled if hwloc enabled, if both hwloc and libcpuid disabled auto configuration for CPU will very limited.
* **`-DWITH_HTTP=OFF`** disable built in HTTP support, this feature used for HTTP API and daemon (solo mining) support.
* **`-DWITH_TLS=OFF`** disable SSL/TLS support (secure connections to pool). This feature add external dependency to OpenSSL.
* **`-DWITH_ASM=OFF`** disable assembly optimizations for modern CryptoNight algorithms.
* **`-DWITH_EMBEDDED_CONFIG=ON`** Enable [embedded](https://github.com/xmrig/xmrig/issues/957) config support.
* **`-DWITH_OPENCL=OFF`** Disable OpenCL backend.
* **`-DWITH_CUDA=OFF`** Disable CUDA backend.
* **`-DWITH_SSE4_1=OFF`** Disable SSE 4.1 for Blake2 (useful for arm builds).

## Debug options

* **`-DWITH_DEBUG_LOG=ON`** enable debug log (mostly network requests).
* **`-DHWLOC_DEBUG=ON`** enable some debug log for hwloc.
* **`-DCMAKE_BUILD_TYPE=Debug`** enable debug build, only useful for investigate crashes, this option slow down miner.

## Special build options

* **`-DXMRIG_USE_BUNDLED_LIBUV=OFF`** use external libuv from `XMRIG_DEPS` / `FindUV.cmake` instead of the default bundled copy under `src/3rdparty/libuv` (downloaded automatically on first configure when this option is ON).
* **`-DXMRIG_DEPS=<path>`** path to precompiled dependencies https://github.com/xmrig/xmrig-deps (folder that contains `include/` and `lib/`, e.g. `.../msvc2022/x64`). If unset, CMake also checks the `XMRIG_DEPS` environment variable and, on Windows, `${CMAKE_SOURCE_DIR}/xmrig-deps/msvc2022/x64` and `.../msvc2019/x64` after you clone or extract [xmrig-deps](https://github.com/xmrig/xmrig-deps) into `./xmrig-deps` next to this project. Still required for OpenSSL/hwloc on typical Windows MSVC builds when not using other package managers.
* **`-DARM_TARGET=<number>`** override ARM target, possible values `7` (ARMv7) and `8` (ARMv8).
* **`-DUV_INCLUDE_DIR=<path>`** custom path to libuv headers.
* **`-DUV_LIBRARY=<path>`** custom path to libuv library.
* **`-DHWLOC_INCLUDE_DIR=<path>`** custom path to hwloc headers.
* **`-DHWLOC_LIBRARY=<path>`** custom path to hwloc library.
* **`-DOPENSSL_ROOT_DIR=<path>`** custom path to OpenSSL.
