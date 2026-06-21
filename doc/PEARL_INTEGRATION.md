# Pearl (PRL) Integration — DRQ Miner

Reference for adding Pearl to DRQ Miner **after dev-fee go-live**. Pearl is Phase 2; do not block 0.0.1 launch on this.

## Authoritative pool spec

**Use this for implementation:**

- **`doc/PEARL_HASHMONKEYS_INTEGRATION.md`** — full wire protocol from Hashmonkeys pool operator (`eu-pearl-dfppsplus`, port **12437**)

This file is the DRQ roadmap and cross-links only.

---

## What Pearl is

Pearl is an L1 **Proof-of-Useful-Work (PoUW)** chain: mining is a side-effect of verifiable **matrix multiplication** (NoisyGEMM), not a traditional hash loop like RandomX or Verus.

- Whitepaper: [pearlresearch.ai](https://pearlresearch.ai/)
- Paper: [arXiv:2504.09971](https://arxiv.org/abs/2504.09971)
- Official monorepo: [pearl-research-labs/pearl](https://github.com/pearl-research-labs/pearl)

Pearl is **GPU-first**. Official reference stack does not support CPU mining or phone GPUs today.

---

## Two mining paths (do not conflate)

| Path | Use case | Stack | DRQ priority |
|------|----------|-------|--------------|
| **Hashmonkeys pool** | Mine to friend/operator pool | alpha-miner 1.6+ JSON-RPC on `:12437` | **First** |
| **Official Pearl node** | Solo / run your own node | `pearld` + `pearl-gateway` + `vllm-miner` | Later |

DRQ target: **Hashmonkeys pool MVP first** (`eu.hashmonkeys.cloud:12437`).

---

## Operator test defaults

| Field | Value |
|-------|--------|
| Pool id | `eu-pearl-dfppsplus` |
| Connect | `stratum+tcp://eu.hashmonkeys.cloud:12437` |
| Test wallet | `prl1p8p8h0jem6xks82mcxj8sr03sau82tr39rffd4hlmfkrj9003xlrssqyzye` |
| First desktop GPU | RTX 5060 Ti 16GB |
| GPU priority | NVIDIA + AMD + Intel desktop; phones research-only until desktop stable |
| Reference miner | `alphaminetech/pearl-miner:1.6.0`+ |

Pool API: `https://hashmonkeys.cloud/api/pools/eu-pearl-dfppsplus`

---

## DRQ integration plan

### Phase P1 — Pool MVP (desktop NVIDIA)

1. Implement Hashmonkeys JSON-RPC client per `PEARL_HASHMONKEYS_INTEGRATION.md`.
2. Port/wrap PoUW GPU backend (alpha-miner / pearl `miner/` / `py-pearl-mining`).
3. CLI: `-a pearl -o eu.hashmonkeys.cloud:12437 -u prl1p....worker`
4. Validate accepted shares on pool dashboard (5060 Ti first).

### Phase P2 — AMD desktop

Reuse alpha-miner AMD build path (ROCm bundle).

### Phase P3 — Intel desktop (experimental)

No official Pearl support today.

### Phase P4 — Phone GPU (research)

Only after P1–P2 stable.

---

## Dev-fee implications (important)

Two different ports/protocols:

| Endpoint | Protocol | Purpose |
|----------|----------|---------|
| `www.dqmining.com:3333` | xmrig-style donate (RX/Verus/DERO/GRR) | DRQ dev-fee for hash algos |
| `eu.hashmonkeys.cloud:12437` | Pearl JSON-RPC (alpha-miner 1.6+) | Pearl pool mining |

**Pearl dev-fee on `:3333` requires a Pearl-aware proxy** that speaks the full Hashmonkeys protocol on the miner side and forwards to `:12437` upstream (rewriting wallet in `mining.subscribe` / `mining.authorize` for fee shares). Transparent TCP forward of xmrig stratum will **not** work for Pearl.

Current dev-fee status (2026-06): `www.dqmining.com:3333` **CLOSED** from external networks — fix WAN/VPS before any dev-fee go-live.

---

## Official pearl monorepo (solo path)

| Directory | Role |
|-----------|------|
| `node/` | **pearld** — full Pearl node |
| `miner/` | **vLLM miner** — GPU mining (Python/CUDA) |
| `py-pearl-mining/` | Python bindings |
| `zk-pow/` | ZK proof circuit |

See upstream README for `task build`, wallet setup, and gateway flow. Not required for Hashmonkeys pool MVP.

---

## Status

| Item | Status |
|------|--------|
| Hashmonkeys protocol doc | `doc/PEARL_HASHMONKEYS_INTEGRATION.md` |
| Pool JSON-RPC client in DRQ | Not started |
| PoUW GPU backend in DRQ | Not started |
| Pearl dev-fee proxy route | Not started (needs Pearl-aware proxy) |
| Dev-fee WAN `:3333` | CLOSED externally — infra blocker |

**Blocker policy:** Dev-fee go-live (RX/0, Verus, DERO, Ghostrider) completes first.
