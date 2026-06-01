# DRQ Miner Testing — v1.0.0 (private)

**DRQ Miner** 6.25.0 — private testing build. Not for public distribution.

## Downloads

| Platform | File |
|----------|------|
| **Phone Userland (arm64)** | `drqminer-linux-arm64-phone.tar.gz` |
| Linux x64 | `drqminer-linux-x64.tar.gz` |
| Windows x64 | `DRQMiner-win64.zip` |

Each `.tar.gz` / `.zip` has a `.sha256` sidecar.

## Phone (Userland)

```bash
tar xzf drqminer-linux-arm64-phone.tar.gz
chmod +x drqminer
./drqminer -V

# DERO (Rabid)
./drqminer --daemon -a astrobwt/v3 -o wss://dero.rabidmining.com:10300 -u YOUR_WALLET -p x -t 4

# Verus
./drqminer -a verus -o stratum+tcp://POOL:PORT -u WALLET -p x -t 4
```

Download on a private repo requires a GitHub PAT — see `doc/GITHUB_PRIVATE_SETUP.md`.

## Windows

Extract `DRQMiner-win64.zip`, run `DRQMiner-Release.exe` from that folder.

## Known limits

- Phone DERO uses portable CPU path (slower than desktop Wolf/SPSA).
- Phone GPU OpenCL is best-effort only.
- AstroSPSA DLL included in Windows zip when CI built it.
