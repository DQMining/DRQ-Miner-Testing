# Phase 2 backlog

**Phase 1 status (2026-06):** Verus CPU/CUDA/OpenCL + DERO AstroBWT v3 on desktop **done**; **v1.0.8** phone release published (TNN NEON + SPSA). Remaining Phase 1 gate: **Userland soak** on v1.0.8 (DERO kH/s + shares; Verus optional). Then **Phase 2** below.

**Phase 2** adds new PoW algos (BC3 first), auto-updater, dev-fee proxy routing, and OpenCL Verus polish.

---

## Algorithm targets (confirmed)

| Target | Priority | Implementation notes |
|--------|----------|----------------------|
| **BitcoinIII (BC3)** — [bc3.network](https://bc3.network/) | **High (Phase 2 lead hash)** | **SHA3-256t** = three iterations of **SHA3-256 (Keccak)**, **not** triple SHA256. 80-byte Bitcoin-style header; fork of Bitcoin Core v29.1 (via BC2). Post-fork PoW signaled by **version bit 12**; hard fork height **30,240**; nBits reset at fork. Mining = **getblocktemplate / Bitcoin RPC** (pools e.g. [Crypto-Eire](https://bc3.network/)), not CN/RX stratum. CPU + GPU from day one. DRQ already has `src/base/crypto/sha3.cpp` — wire `sha3_256t` hasher + GBT client path (reuse/adapt Bitcoin mining job layout). **Not compatible** with SHA-256d rental hashrate (by design). |
| **Yescrypt** (R8, R16, …) | High | Not in tree today. Likely CPU-only; register in `Algorithm.cpp`, new `cmake/yescrypt.cmake`, stratum/job format per coin variant. Clarify each coin’s parameters (N, r, p or coin-specific R8/R16 profiles). |
| **SHA256D** | High | Double SHA256 (Bitcoin-style). Needs non-standard xmrig stratum or dedicated client path; block/header + nonce layout differs from CN/RX. BC3 plumbing overlaps but **hash function differs** (SHA3-256t vs SHA256d). |
| **SHA256T** (triple SHA256) | Medium (before WART) | `SHA256(SHA256(SHA256(data)))` — **different from BC3 SHA3-256t**. Foundation for **WART Janushash** (GPU leg) + coins like OC Protocol. OpenSSL/`sha256_utils` triple loop; CUDA kernel for GPU leg. Same Bitcoin-style job plumbing as BC3 once GBT client exists. |
| **Warthog (WART) — Janushash** | Medium (after SHA256T + Verus GPU) | **Proof of Balanced Work:** **Sha256t** (GPU) × **Verushash v2.1+** (CPU). “Janus hashrate” = f(both); gaming-PC balance required. [Janushash spec](https://github.com/warthog-network/docs/blob/master/Janushash/Janushash.md), [docs](https://docs.warthog.network/). DRQ: Verus CPU strong; add SHA256T CUDA + combiner. |
| **Pearl (PRL)** — [pearlresearch.ai](https://pearlresearch.ai/) | High (large scope) | **PoUW, not a hash.** Hashmonkeys pool spec: **`doc/PEARL_HASHMONKEYS_INTEGRATION.md`** (`eu.hashmonkeys.cloud:12437`, alpha-miner 1.6+ JSON-RPC). DRQ roadmap: **`doc/PEARL_INTEGRATION.md`**. Official node path: [pearl-research-labs/pearl](https://github.com/pearl-research-labs/pearl). |
| **RandomX v2** | High | **Monero** is moving to RandomX v2 — track official Monero spec / reference implementation when published. Extend `src/crypto/rx/` and `Algorithm.cpp`; may need new dataset/JIT rules vs current `rx/0`. |
| **Verus OpenCL** | Done (v1) | GPU kernel from [monkins1010/AMDVerusCoin](https://github.com/monkins1010/AMDVerusCoin) `input.cl` → `src/backend/opencl/cl/verus/`. Regenerate `verushash_cl.h` after editing `.cl` (see `scripts` / build notes). |

**Removed ambiguity:** earlier “r16” / “yespower” items are folded into **Yescrypt R8/R16 (etc.)** per operator spec — confirm exact coin list before coding (each may differ in params and stratum).

### Hash naming (operator confirmed)

| Name | Function | Primary coin(s) | DRQ status |
|------|----------|-----------------|------------|
| **SHA3-256t** | `SHA3-256` × 3 (Keccak) | **[BitcoinIII (BC3)](https://bc3.network/)** | Phase 2 **first** — `sha3.cpp` in tree |
| **SHA256T** / **Sha256t** | `SHA256` × 3 | Building block for **[Warthog (WART)](https://docs.warthog.network/)** **Janushash** (later); also OC Protocol, etc. | After BC3; reuse Bitcoin-job + OpenSSL SHA256 loop |
| **Janushash** (PoBW) | **Sha256t × Verushash v2.x** (balanced GPU+CPU) | **WART** — [Janushash spec](https://github.com/warthog-network/docs/blob/master/Janushash/Janushash.md) | **Later** — needs SHA256T GPU path **and** Verus CPU (DRQ Verus ~ready); “janus hashrate” = f(GPU Sha256t, CPU Verushash). Reference: [janusminer](https://github.com/CoinFuMasterShifu/janusminer), [mining guide](https://docs.warthog.network/guides/mining/mining-quickstart/) |

**Do not conflate SHA3-256t (BC3) with SHA256T (WART foundation).** Different primitives, different implementations.

---

## DERO (AstroBWT v3) — test now (Phase 1)

AstroBWT v3 is **already in DRQ Miner** (Phase 1 CPU optimization). Use for pool smoke tests before Phase 2 algos land.

| Field | Value |
|-------|--------|
| Pool | `wss://dero.rabidmining.com:10300` (**WebSocket**, not plain TCP) |
| Algorithm | `astrobwt/v3` (or coin `DERO`) |
| Test wallet | `dero1qy5c4sl5p2carlla465gyexlh6vye32v039tsxe4gp9zrha30cv9uqgr66utu` |
| **Pool URL** | Plain **`dero.rabidmining.com:10300`** — miner auto-enables WSS + daemon mode (no `--daemon` / `wss://` needed) |

Example (after Release build):

```text
DRQMiner-Release.exe -a astrobwtv3/dero -o dero.rabidmining.com:10300 -u YOUR_WALLET -p x
```

Legacy form still works: `--daemon -o wss://dero.rabidmining.com:10300`. Classic stratum TCP to Rabid will not work (pool speaks WSS JSON-RPC).

Do **not** commit this wallet in `config.json` to git; use CLI or a local ignored config only.

---

## Auto-updater

- Manifest URL (GitHub Releases or `https://www.dqmining.com/releases/manifest.json`)
- Semver compare vs `src/version.h`
- Per-OS/arch artifacts: win64, linux-x64, linux-arm64, macos-arm64
- SHA256 (or signature) verify before replace
- Windows: safe replace (file lock / helper process)
- Config: `"auto-update": true/false`, check interval

Reusable code: `HttpClient`, TLS, `APP_VERSION`.

MVP scaffolding in repo:
- `scripts/upstream/manifest.template.json`
- `scripts/upstream/check_update_dry_run.ps1`
- `.github/workflows/update-dry-run.yml`

---

## Dev fee → proxy → wallets

**Miner-side:** point `DonateStrategy.cpp` donate host to your proxy; keep `donate-level` behavior.

**Proxy-side (your infra):** stratum proxy maps worker/algo prefix to payout addresses (VRSC, DERO, future Yescrypt/SHA256D/RX-v2 coins). TLS on `:443` if using `donate-over-proxy`.

**Still needed from operator:**

- Proxy hostname/ports (beyond default `pool.dqmining.com:5040`)
- TLS yes/no for donate sessions
- Wallet list **per algo** on the proxy (DERO test wallet above is for **pool testing**, not necessarily dev fee)
- Decide temporary DERO donate endpoint while proxy bootstrap is being completed

Reference setup guide: `doc/DEVFEE_PROXY_BOOTSTRAP.md`

---

## OpenCL Verus (AMD/Intel GPU)

Port ccminer OpenCL Verus into `OclSource.cpp` + runner. Current OpenCL Verus path is CPU-only stub.

---

## Mobile / legacy GPU

**Termux / Userland (phones):** primary guide `doc/TERMUX_USERLAND_ANDROID.md`.

| Milestone | Notes |
|-----------|--------|
| Docs + `scripts/android/*` | Termux native build, Userland push, phone `config.json` |
| Release binaries | `drqminer-linux-arm64` (Userland) + `drqminer-android-arm64` (Termux) in GitHub Releases |
| Slim phone build | `-DWITH_RANDOMX=OFF` etc. (see `build_termux.sh`) |
| Soak test | **DERO** + Verus CPU accepted shares on Userland (blocks v1 phone claim until done) |
| OpenCL on Adreno | Best-effort; document per-device |
| **v1 GitHub Release** | See `doc/V1_RELEASE_CHECKLIST.md` — desktop ~ready, phone DERO needs device test |

**Userland default:** `linux-arm64-Release` (glibc). **Termux only:** `android-arm64-Release` or `scripts/android/build_termux.sh`.

**Experimental:** `android-arm64-Release` preset. **DERO CPU** on arm64: v1.0.8+ uses TNN-class path (AstroSPSA + `branchComputeCPU_aarch64`) when CI build is good; v1.0.6 fallback ~2 kH/s.

**GRID K2 / Kepler:** `x64-Release-verus-kepler` preset (CUDA 11.8 + sm_35) or `--opencl` on the standard Windows build.

Desktop ARM64 (Apple Silicon, Linux aarch64): `linux-arm64-Release`, `macos-arm64-Release`.

---

## Suggested Phase 2 order

1. **DERO / v1.0.8 phone** — finish Rabid soak (TNN path validation)
2. **BitcoinIII (BC3)** — **SHA3-256t** CPU + GBT/pool client ([bc3.network](https://bc3.network/)); CUDA SHA3 kernel follow-on
3. **SHA256T** (triple SHA256) — shared primitive for later WART; OC Protocol etc.
4. **SHA256D** — reuse Bitcoin job plumbing
5. **Warthog (WART) — Janushash** — Sha256t (GPU) + Verushash (CPU) PoBW after (2)+(3)+Verus GPU
6. **Yescrypt R8/R16** — CPU + stratum for first target coin
7. **Pearl (PoUW)** — pool stratum MVP then optional official node stack — see `doc/PEARL_INTEGRATION.md`
8. **RandomX v2** — when Monero publishes stable spec
9. Auto-updater + dev-fee proxy wiring (BC3/WART/Pearl wallets per algo)
10. OpenCL Verus
