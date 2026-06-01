# Phase 2 backlog

Phase 1 (Verus CUDA stability, AstroBWT v3 CPU opts, cross-platform branding, GitHub docs) is in the tree. **Phase 2** adds new PoW algos, auto-updater, dev-fee proxy routing, and OpenCL Verus.

---

## Algorithm targets (confirmed)

| Target | Priority | Implementation notes |
|--------|----------|----------------------|
| **Yescrypt** (R8, R16, …) | High | Not in tree today. Likely CPU-only; register in `Algorithm.cpp`, new `cmake/yescrypt.cmake`, stratum/job format per coin variant. Clarify each coin’s parameters (N, r, p or coin-specific R8/R16 profiles). |
| **SHA256D** | High | Double SHA256 (Bitcoin-style). Needs non-standard xmrig stratum or dedicated client path; block/header + nonce layout differs from CN/RX. |
| **RandomX v2** | High | **Monero** is moving to RandomX v2 — track official Monero spec / reference implementation when published. Extend `src/crypto/rx/` and `Algorithm.cpp`; may need new dataset/JIT rules vs current `rx/0`. |
| **Verus OpenCL** | Done (v1) | GPU kernel from [monkins1010/AMDVerusCoin](https://github.com/monkins1010/AMDVerusCoin) `input.cl` → `src/backend/opencl/cl/verus/`. Regenerate `verushash_cl.h` after editing `.cl` (see `scripts` / build notes). |

**Removed ambiguity:** earlier “r16” / “yespower” items are folded into **Yescrypt R8/R16 (etc.)** per operator spec — confirm exact coin list before coding (each may differ in params and stratum).

---

## DERO (AstroBWT v3) — test now (Phase 1)

AstroBWT v3 is **already in DRQ Miner** (Phase 1 CPU optimization). Use for pool smoke tests before Phase 2 algos land.

| Field | Value |
|-------|--------|
| Pool | `wss://dero.rabidmining.com:10300` (**WebSocket**, not plain TCP) |
| Algorithm | `astrobwt/v3` (or coin `DERO`) |
| Test wallet | `dero1qy5c4sl5p2carlla465gyexlh6vye32v039tsxe4gp9zrha30cv9uqgr66utu` |
| **Required flag** | **`--daemon`** (Rabid pool uses WSS via `DaemonClient`, not classic stratum) |

Example (after Release build):

```text
DRQMiner-Release.exe --daemon -a astrobwt/v3 -o wss://dero.rabidmining.com:10300 -u dero1qy5c4sl5p2carlla465gyexlh6vye32v039tsxe4gp9zrha30cv9uqgr66utu -p x
```

Plain `dero.rabidmining.com:10300` without `wss://` and without `--daemon` will fail with `read error: "end of file"`.

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

---

## Dev fee → proxy → wallets

**Miner-side:** point `DonateStrategy.cpp` donate host to your proxy; keep `donate-level` behavior.

**Proxy-side (your infra):** stratum proxy maps worker/algo prefix to payout addresses (VRSC, DERO, future Yescrypt/SHA256D/RX-v2 coins). TLS on `:443` if using `donate-over-proxy`.

**Still needed from operator:**

- Proxy hostname/ports (beyond default `pool.dqmining.com:5040`)
- TLS yes/no for donate sessions
- Wallet list **per algo** on the proxy (DERO test wallet above is for **pool testing**, not necessarily dev fee)

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

**Experimental:** `android-arm64-Release` preset. **DERO CPU works** on arm64 (divsufsort); AstroSPSA/Wolf fast paths are desktop-only.

**GRID K2 / Kepler:** `x64-Release-verus-kepler` preset (CUDA 11.8 + sm_35) or `--opencl` on the standard Windows build.

Desktop ARM64 (Apple Silicon, Linux aarch64): `linux-arm64-Release`, `macos-arm64-Release`.

---

## Suggested Phase 2 order

1. **DERO pool validation** on Rabid (Phase 1 binary) — baseline before new algos
2. **Yescrypt R8/R16** — CPU + stratum for first target coin
3. **SHA256D** — stratum/client design
4. **RandomX v2** — when Monero publishes stable spec
5. Auto-updater + dev-fee proxy wiring
6. OpenCL Verus
