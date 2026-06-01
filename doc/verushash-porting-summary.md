## VerusHash Porting Summary

This document tracks all changes made to integrate VerusHash into this xmrig-based miner.

- **Baseline repository**: `Slashx124/xmrig-test` (`master` branch) as of the time of porting.
- **Scope**: CPU VerusHash mining first, GPU support to be added later.

### Change log (high level)

- **Core hashing**
  - Wire VerusHash (VerusHash v2) into the CPU hashing backend following the existing CN/GhostRider/AstroBWT patterns.
  - Added `VerusHash_xmrig.h` / `VerusHash_xmrig.cpp` wrappers and hooked them into `CnHash.cpp` so `Algorithm::VERUSHASH` uses `verus_hash_v2` via the standard `cn_hash_fun` interface.
- **Build system**
  - Ensure all VerusHash sources are compiled and guarded behind `WITH_VERUSHASH` / `XMRIG_ALGO_VERUSHASH`.
  - Updated `cmake/verushash.cmake` to include the new wrapper sources in `HEADERS_CRYPTO` / `SOURCES_CRYPTO`.
  - Added MSVC compatibility in `src/crypto/verushash/verus_clhash.h` by replacing the GCC-only `x86intrin.h` include path with `immintrin.h` on MSVC and providing GCC-style CPUID helper compatibility (`__get_cpuid`, `bit_AVX`, `bit_AES`, `bit_PCLMUL`).
  - Removed non-essential Equihash compilation units from `cmake/verushash.cmake` to avoid unrelated Boost-heavy VerusCore dependencies in the xmrig PoW path.
  - Added `src/crypto/verushash/uint256.cpp` to VerusHash source compilation and patched it to remove `utilstrencodings.h` dependency by using a local hex parser helper.

- **Visual Studio portability hardening**
  - Replaced `tinyformat` dependency in `verus_clhash.h` with local `snprintf` formatting.
  - Added Windows/MSVC endian fallback macros in `common.h` and stubbed `init_and_check_sodium()` to remove hard dependency on libsodium for the xmrig PoW integration path.
  - Updated `verus_clhash.cpp` and `verus_clhash_portable.cpp` include strategy to compile standalone in xmrig (explicit Verus headers instead of missing VerusCore aggregate headers), disabled unused VerusCore mining helper entrypoints that depend on missing chain primitives, and fixed C/C++ linkage for `haraka_portable` symbols.
  - Replaced C99 VLA usage in `haraka_portable.c` with a fixed-size local buffer compatible with MSVC.
  - Verified a full VS2022 developer-prompt build now succeeds (`xmrig.exe` links successfully).
- **Stratum / networking**
  - Added a Verus-specific Stratum dialect path in `EthStratumClient` and wired `Pool::createClient()` to use that path for `Algorithm::VERUSHASH_FAMILY` / `Coin::VERUS`.
  - Verus path now uses `mining.subscribe` + `mining.authorize` flow (instead of `login`), handles `mining.notify` (Verus param layout) and `mining.set_target`, and formats `mining.submit` in pool-style tuple form.
  - Added duplicate-job tolerance for Verus jobs to avoid reconnect loops on repeated notifications from this pool.
  - Added per-share Verus submit payload wiring (`ntime`, `extraNonce2`, `solution`) via `Job` -> `JobResult` -> `EthStratumClient::submit()`, plus Verus-specific solution/nonce layout handling and larger job blob capacity for serialized Verus solution payloads.
  - Verus solution submission now uses CompactSize-prefixed solution bytes (`fd4005` + 1344-byte solution), fixed-width `extraNonce2`, and SRBMiner-compatible shifted solution-tail packing so `extraNonce1` and worker nonce land at the exact pool-expected offsets.
  - Added robust write fallback for socket backpressure (`uv_try_write` -> queued `uv_write`) in the shared Stratum client path to avoid disconnect loops under transient `UV_EAGAIN`.
- **Hash/preimage alignment (ccminer cross-check)**
  - Cross-checked with `ccminer-verus-Verus2.2` and aligned hash preimage construction to `140-byte header + CompactSize + 1344-byte solution` (explicit 32-byte `nNonce` field before solution in the blob).
  - Updated Verus nonce offsets and `Finalize2b` version-parsing offsets to the 140-byte header layout.
  - Added merged-mining canonicalization before hashing for v2.2+/v7+ solution payloads (zeroing non-canonical header/solution fields) in `VerusHash_xmrig.cpp`, matching ccminer behavior.
- **Current live status**
  - Live pool validation on `stratum+tcp://usw.vipor.net:5040` now shows accepted shares.
  - 30-minute soak run result: **20 accepted / 0 rejected** in miner output, with no `invalid solution, pool nonce missing` errors.
- **Testing**
  - Recommended validation: run the miner with `--algo=verushash` (or coin `VRSC`) against a Verus pool and verify shares are accepted and hashrate is in line with the Verus reference miner.
  - For deeper verification, compare hashes of a fixed header blob using this miner and an external VerusHash implementation (e.g. `verushash-node`'s `hash2` function as shown in its `test.js` in [`VerusCoin/verushash-node`](https://github.com/VerusCoin/verushash-node)).
  - Added a simple CLI self-test flag `--verushash-self-test` that computes a VerusHash v2 of a fixed test string on startup, prints the hash in hex, and exits; this can be compared against external implementations.
  - Fixed self-test startup ordering by calling `CVerusHashV2::init()` before `verus_hash_v2()` to avoid an initialization-time crash on Windows builds.
  - For cross-checking `xmrig --verushash-self-test`, clone [`VerusCoin/verushashpy`](https://github.com/VerusCoin/verushashpy) elsewhere and compare the printed hash to `verus_hash2()` in `tests/verus_hash.py` for the same fixed input string.

Each subsequent code change for the VerusHash port should be reflected here with a short description under the sections above, referencing the affected file names.

**Note:** `verushash-porting.patch` is an archival diff and may still mention helper scripts or paths that were later removed to keep the repository layout close to upstream xmrig source-only trees; treat this summary as authoritative for current practice.

### DRQ Miner product / rebrand

- **`src/version.h`**: `APP_NAME` / `APP_ID` / `APP_SITE` / `APP_DOMAIN` set for DRQ Miner and dqmining.com.
- **`README.md`**: Describes DRQ Miner scope (VerusHash CPU-first) and build/output names.
- **`CMakeLists.txt`**: MSVC `RUNTIME_OUTPUT_NAME_*` uses `DRQMiner-*` so binaries are easy to distinguish from stock XMRig.

### Dev donation (dqmining.com)

- **`src/donate.h`**: Default `kDefaultDonateLevel` is **1%** (still overridable; minimum remains 0).
- **`src/net/strategies/DonateStrategy.cpp`**: Donation pools point to **`pool.dqmining.com`** (TCP **5040**; TLS host same, port **443** when TLS builds are used). Verus builds use **`Pool::MODE_AUTO_ETH`** for the donate session (same stratum style as KawPow/GhostRider).
- **`src/core/config/Config_default.h`**: Example `donate-level` **1** and example pool URL **`pool.dqmining.com:5040`**.
- **Operational note:** If that host/port is not a live Verus stratum, update it to the operator’s real endpoint or users should set `donate-level` to `0`.

### CPU performance (target: accepted shares + high hashrate)

- Always run **`Release`** or **`RelWithDebInfo`** with optimizations; **`Debug`** disables or neuters inlining and is not representative of mining speed.
- **`cmake/flags.cmake`**: RelWithDebInfo is aligned with Release-style optimization on MSVC so “debuggable Release” builds are not accidentally slow.
- **`CMakeLists.txt`**: Extra MSVC compile options on Verus/Haraka hot files where enabled.
- Thread count, affinity, and huge pages behave like upstream XMRig; tune `--threads`, `cpu` JSON, and OS huge-page settings for your hardware.
- For algorithmic SIMD ideas, compare with reference miners (e.g. ccminer-verus / Verus reference); further micro-opts belong in `src/crypto/verushash` and `VerusHash_xmrig.cpp`.

### GPU (OpenCL / CUDA) and VerusHash

- **Wired backends:** `OclWorker` / `CudaWorker` dispatch **`Algorithm::VERUSHASH_FAMILY`** to `OclVerusHashRunner` / `CudaVerusHostRunner`; autoconfig via `ocl_verus_generator.cpp`, `generate<Algorithm::VERUSHASH_FAMILY>()`, and `CudaDevice::generate()` for Verus.
- **CUDA (NVIDIA):** Embedded `ccminer_verus_gpu.cu` when built with nvcc (`XMRIG_FEATURE_CUDA_VERUS`). See **`doc/GPU_VERUS_NATIVE.md`**.
- **OpenCL (AMD/Intel):** GPU `verus_gpu_hash` from [monkins1010/AMDVerusCoin](https://github.com/monkins1010/AMDVerusCoin) `input.cl` → `src/backend/opencl/cl/verus/verushash.cl`. Loaded at runtime from `verushash.cl` beside the exe (post-build copy). `OclVerusHashRunner` mirrors the CUDA host setup (`VerusHashHalf`, `GenNewCLKey`, nonce scan). Fallback to CPU hash if the kernel file is missing or the job is merged-mining.

### GitHub / release hygiene

- Initialize a **new** git repository for DRQ Miner; do not push wallets or local `config.json` with secrets.
- Short publish commands are in **`README.md`** under “Publishing to GitHub”.

