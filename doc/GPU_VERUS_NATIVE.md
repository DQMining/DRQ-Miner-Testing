# Native GPU VerusHash

## Status in DRQ Miner (2026)

**NVIDIA CUDA (embedded):** When CMake finds `nvcc`, DRQ Miner builds `src/crypto/verushash/cuda/ccminer_verus_gpu.cu` and enables `XMRIG_FEATURE_CUDA_VERUS`. The host still runs CPU `VerusHashHalf` + `GenNewCLKey` per job; the GPU runs the inner nonce scan. Use `--cuda` without `xmrig-cuda.dll` for Verus-only mining.

Requirements:

- Release build with CUDA toolkit detected at configure time
- `cudart64_*.dll` beside the exe on Windows (CMake copies when possible)
- One CUDA thread per GPU (shared `__constant__` job state)
- No merged-mining jobs on GPU (falls back to CPU)

**OpenCL:** **GPU kernel** from [monkins1010/AMDVerusCoin](https://github.com/monkins1010/AMDVerusCoin) `input.cl` (vendored as `src/backend/opencl/cl/verus/verushash.cl`). Build copies `verushash.cl` next to `DRQMiner-Release.exe`. Use `--no-cpu --no-cuda --opencl`. Work size **128**; host still runs `VerusHashHalf` + `GenNewCLKey` per job (same as CUDA).

Smoke test: `scripts/vipor_cuda_smoke.bat` (Windows).

---

## Historical / porting notes (OpenCL not shipped yet)

**“GPU mines by itself”** means the hash loop must run in **OpenCL or CUDA device code** (kernels), not on the host CPU inside a “GPU” worker thread.

This repository’s current OpenCL/CUDA Verus path **still executes `VerusHash_xmrig` on the CPU** for correctness and stratum compatibility. That is intentional scaffolding until real kernels exist.

## Reference implementation (NVIDIA CUDA)

The practical open-source baseline for **NVIDIA** is:

- Repository: [monkins1010/ccminer](https://github.com/monkins1010/ccminer)  
- Branch: `verus2.2gpu`  
- Core GPU code: `verus/verus.cu` (`verus_gpu_hash`, `verus_extra_gpu_prepare`, host helpers `verus_init`, `verus_setBlock`, `verus_hash`)  
- Host scan / work packing: `verus/verusscan.cpp` (`scanhash_verus`, `VerusHashHalf`, `GenNewCLKey`, `Verus2hash`, …)  
- Supporting Verus/Crypto: `verus/verus_clhash*.cpp`, `verus/haraka*.c`, `verus/verus_hash.*`, `verus/primitives/*`, etc.

`scanhash_verus` builds the **1487-byte** preimage (`full_data`: 140-byte header + compact size + 1344-byte solution), runs **CPU** `VerusHashHalf` + `GenNewCLKey`, then pushes block + key to the GPU and loops `verus_hash` → `verus_gpu_hash`.

## Your local ccminer 3.8.3 trees (this PC)

These folders sit next to the xmrig-style project under `xmrig-Verus-Fast` (paths contain spaces — quote them in shells).

### Official GPU source — monkins1010 `verus2.2gpu` (cloned here)

`C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\cc miner sources\monkins1010-ccminer-verus2.2gpu`

This is a **shallow git clone** of [monkins1010/ccminer](https://github.com/monkins1010/ccminer) on branch **`verus2.2gpu`** (remote branch names are case-sensitive; the GPU branch is **lowercase** `verus2.2gpu`, not `Verus2.2gpu`). It includes the full open-source **CUDA Verus** stack, including **`verus/verus.cu`**, for building or porting into DRQ Miner.

Re-clone or refresh (PowerShell):

```powershell
cd "C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\cc miner sources"
Remove-Item -Recurse -Force monkins1010-ccminer-verus2.2gpu -ErrorAction SilentlyContinue
git clone -b verus2.2gpu --depth 1 https://github.com/monkins1010/ccminer.git monkins1010-ccminer-verus2.2gpu
```

For a full history or other branches (`Verus2.2`, `ARM`, …), omit `--depth 1` or add `git fetch` as needed.

The **`ccminer-3.8.3`** tree below is material you identify as **[monkins1010](https://github.com/monkins1010/ccminer)** GPU-oriented ccminer **3.8.3**. The walk-through is from the **actual files on disk** (March 2026): that **source** tree is a **partial** snapshot — **no** `verus.cu`, and `verus/verusscan.cpp` there runs the hash loop on the **CPU**. Your **3.8.3** / prebuilt layout can still be the **correct** runtime setup; the **clone above** is the matching **public** tree when you need **`verus.cu`** and the GPU scan path in source form.

### Open-source / partial source — `ccminer-3.8.3`

`C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\cc miner sources\ccminer-3.8.3\ccminer-3.8.3`

- **Verus-related files** live under **`verus\`** (enumerated in the **Inventory** section below).
- **`algos.h`**: the string **`"verus"`** occupies the same **slot** in `algo_names[]` where upstream ccminer had **`"equihash"`** (enum still has `ALGO_EQUIHASH` — common fork pattern). **`ccminer.cpp`** calls **`scanhash_verus`** for that algo.
- **`verus/verusscan.cpp`**: **`#include <cuda_helper.h>`** is **commented out**; **`scanhash_verus`** sets **`throughput = 1`** and loops **`Verus2hash`** on the host — there is **no** `verus_hash` / `verus_gpu_hash` device path in this file. So **this source drop is CPU Verus scanning**, not the GPU scan loop described for branch **`verus2.2gpu`** on GitHub.
- **CUDA in tree**: recursive search finds **one** `.cu` file: **`equi/cuda_equi.cu`** (Equihash), **not** Verus.
- **`Makefile.am`** lists only **`verus/verusscan.cpp`**, **`verus/haraka.c`**, **`verus/verus_clhash.cpp`** (plus **`equi/equi-stratum.cpp`**) — **no `verus.cu`**, no Equihash `.cu` in autotools sources.
- **`ccminer.vcxproj`**: CUDA include/lib paths exist, but the project **does not include `verus.cu`** (and in this tree **there is no `verus.cu` file**). GPU Verus in a **different** build would come from **sources not present here** or from the **prebuilt** folder.
- **`Output-SourceControl-Git.txt`** only references another path (`D:\15 raven\ccminer-8.21\...`) — IDE metadata, not proof of origin.
- **Implication:** To **compile** GPU Verus yourself you need a tree that contains **`verus/verus.cu`** and the matching **`verusscan.cpp`** host glue (e.g. clone [monkins1010/ccminer](https://github.com/monkins1010/ccminer) branch **`verus2.2gpu`** with the correct spelling). This **3.8.3** folder alone does **not** provide that kernel source.

### Prebuilt / closed GPU build — `ccminer_GPU_3_8_3`

`C:\Users\DREADQUILL11168\Documents\Mining Software\Astrobwt Comparing\xmrig-Verus-Fast\cc miner sources\ccminer_GPU_3_8_3\ccminer_GPU_3_8_3`

- On this machine the folder currently contains only **`run.bat`** (example `ccminer.exe -a verus ...`). **`ccminer.exe`** may live next to it on your disk but is **not** in the workspace listing used here — use your local binary for **actual NVIDIA GPU Verus** today.
- **Do not** copy `ccminer.exe` (or other blobs) into DRQ Miner’s git tree as “integration” without license clearance; use it **side-by-side** with xmrig, or keep mining Verus with that exe until **open** CUDA sources are ported into this project.

### Inventory: ccminer-3.8.3 source tree (159 files)

Counts by top-level area:

| Area | Files | Role |
|------|------:|------|
| `compat/` | 86 | Windows/Linux portability, **curl-for-windows** (31), **jansson** (23), **nvapi** (15), **pthreads** (5), **getopt** (2), headers, `winansi`, etc. |
| Repository root | 42 | Main miner: `ccminer.cpp`, `algos.h`, `pools.cpp`, build (`Makefile.am`, `configure.ac`, `*.sln`, `*.vcxproj*`), `README.txt` / `README.md`, configs, `bench.cpp`, `api.cpp`, … |
| `equi/` | 14 | Equihash + **only** CUDA file in repo: `cuda_equi.cu`; Blake2 helpers under `equi/blake2/` |
| `verus/` | 10 | VerusHash **host** implementation (see list below) |
| `api/` | 4 | `index.php`, `local-sample.php`, `summary.pl`, `websocket.htm` |
| `res/` | 3 | `ccminer.ico`, `ccminer.rc`, `resource.h` |

**`verus/` (10 files):** `haraka.c`, `haraka.h`, `haraka_portable.c`, `haraka_portable.h`, `verus_clhash.cpp`, `verus_clhash.h`, `verus_clhash_portable.cpp`, `verus_hash.cpp`, `verus_hash.h`, `verusscan.cpp`.

**`equi/` (14 files):** `blake2/blake2bx.cpp`, `blake2/blake2b-load-sse2.h`, `blake2/blake2b-load-sse41.h`, `blake2/blake2b-round.h`, `blake2/blake2-config.h`, `blake2/blake2-impl.h`, `blake2/blake2-round.h`, `blake2/blake2.h`, `cuda_equi.cu`, `eqcuda.hpp`, `equi-stratum.cpp`, `equi.cpp`, `equihash.cpp`, `equihash.h`.

**Repository root (42 files):** `.gitignore`, `algos.h`, `api.cpp`, `autogen.sh`, `avxintrin-emu.h`, `bench.cpp`, `bignum.cpp`, `bignum.hpp`, `build-freebsd.sh`, `build.cmd`, `build.sh`, `ccminer.conf`, `ccminer.cpp`, `ccminer.sln`, `ccminer.vcxproj`, `ccminer.vcxproj.filters`, `ccminer.vcxproj.user`, `compat.h`, `compile`, `config.guess`, `configure.ac`, `configure.sh`, `crc32.c`, `elist.h`, `hashlog.cpp`, `INSTALL`, `install-sh`, `Makefile.am`, `miner.h`, `nvapi.cpp`, `Output-SourceControl-Git.txt`, `pools.conf`, `pools.cpp`, `README.md`, `README.txt`, `run`, `serialize.hpp`, `stats.cpp`, `sysinfos.cpp`, `uint256.h`, `util.cpp`, `vc140.pdb`.

**`api/`:** `index.php`, `local-sample.php`, `summary.pl`, `websocket.htm`.

**`compat/`:** Too many small files to list inline; largest subtrees are **`compat/curl-for-windows/`**, **`compat/jansson/`**, **`compat/nvapi/`**, **`compat/pthreads/`**, plus single-file shims (`gettimeofday.c`, `winansi.c`, `inttypes.h`, `unistd.h`, `ccminer-config.h`, `bignum_ssl10.hpp`, etc.).

### Inventory: `ccminer_GPU_3_8_3` (this workspace)

- **`run.bat`** only (regenerate or expand if you add `ccminer.exe`, DLLs, or docs beside it).

## What a full xmrig integration requires

1. **CMake / toolchain**  
   - `enable_language(CUDA)` (or nvcc as a custom command), CUDA toolkit, architecture flags (`sm_60`+ typical for modern Verus builds).  
   - Optional: keep xmrig’s existing **xmrig-cuda plugin** model and add Verus there instead of the main binary (larger change across two repos).

2. **Vendored or submodule**  
   - Either submodule `ccminer` at `verus2.2gpu` or copy the `verus/` tree under e.g. `src/3rdparty/ccminer-verus/` with **LICENSE** attribution (GPL lineage from ccminer / tpruvot / nheqminer — verify compatibility with your distribution model).

3. **Adapter layer** (new C++ in xmrig)  
   - Map xmrig `Job` blob + Verus fields to ccminer’s `full_data` / `work` layout (same 140 + `fd4005` + 1344 pattern you already use on CPU).  
   - Replace ccminer globals (`device_map`, `work_restart`, `opt_*`) with xmrig’s device index, `Nonce::CUDA`, and job stop signals.  
   - Implement `CudaVerusNativeRunner` (or extend xmrig-cuda): `init` → `verus_init`, each job → `verus_setBlock` + `verus_hash`, read back nonces, feed `JobResults` (GPU bundle path already re-validates on CPU today; you may keep or relax that once the kernel is trusted).

4. **OpenCL**  
   - There is no drop-in OpenCL file in that ccminer tree comparable to `verus.cu`; a separate OpenCL port (or another miner’s CL) would be another large project.

5. **Testing**  
   - Same pool + share acceptance tests you use for CPU; compare hashrate to stock ccminer on the same GPU.

## Practical recommendation today

For **NVIDIA GPU hashrate on Verus**, run **[monkins1010/ccminer](https://github.com/monkins1010/ccminer)** (branch `verus2.2gpu`) against your pool until the native port above is finished in DRQ Miner / xmrig.

## Status

- **In this tree:** CPU VerusHash v2 is production-oriented; OpenCL/CUDA entries are **host-hash placeholders**.  
- **Not in this tree:** Compiled CUDA kernels for Verus (`verus_gpu_hash` path) or OpenCL equivalents.

When a native port lands, update this file and the main `README.md` “GPU” section accordingly.
