# DRQ vs TNN-MINER v0.8.2 (DERO)

Source reference: `tnn-miner-v0.8.2/` (from [GitLab v0.8.2 zip](https://gitlab.com/Tritonn204/tnn-miner/-/archive/v0.8.2/tnn-miner-v0.8.2.zip)).

## What TNN does (~40 kH/s on 14900K)

| Piece | TNN v0.8.2 | DRQ (current) |
|--------|------------|----------------|
| PoW input | **48-byte miniblock** (`MINIBLOCK_SIZE`) | Now clamped to 48 B |
| Nonce | BE at offset **43** (`MINIBLOCK_SIZE - 5`) | Fixed in `Job::nonceOffset()` |
| Main loop | **wolf** SIMD (`wolfbranching.spec.cpp`) | **wolf AVX2 in-place** (`src/crypto/astrobwt/wolf/`) — same semantics as `astrobwt_ops.inl` |
| Suffix array | `divsufsort` or **SPSA** (`USE_ASTRO_SPSA`, default ON) | `divsufsort` (consensus-safe) — **likely ~2× SA cost vs TNN** |
| Worker | Persistent `workerData`, OpenSSL SHA reuse | Per-hash scratch in `cryptonight_ctx` |
| Inner loop | 512 hashes before counter flush | **64** hashes per worker iteration |

## CLI (v0.8.2)

Coin flag must be **`--DERO`** (uppercase). `--dero` clashes with `--dero-benchmark`.

## DRQ roadmap to ~40 kH/s

1. **Done / in tree:** 48 B input, nonce offset 43, batch-64 worker loop, AVX2 micro-opts.
2. **Done:** Wolf **CodeLUT** AVX2 for ops 0–252 (in-place, passes 6/6 derohe vectors). TNN’s chunked `WOLF_COMPUTE_BODY` is **not** consensus-equivalent — do not use as-is.
3. **Next for ~40 kH/s:** Link **AstroSPSA** (TNN `USE_ASTRO_SPSA`) or verify fast SA that matches `divsufsort` on all vectors; profile SA vs branched loop on 48 B mining input.
4. **Runtime:** `--huge-pages`, P-core affinity, `-t 24`–`32` on 14900K.

## Validate DRQ

```text
out\build\x64-Release\Release\test_astrobwt.exe
out\build\x64-Release\Release\test_astrobwt.exe --bench-mt 32 200
out\build\x64-Release\DRQMiner-Release.exe --daemon -a astrobwt/v3 -o wss://dero.rabidmining.com:10300 -u WALLET -p x -t 32
```

Consensus: **6/6** test vectors vs derohe (required before pool mining).
