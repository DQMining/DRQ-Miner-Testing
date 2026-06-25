# DRQ Miner — Alpha

Pre-release builds for internal testing. **Not for production.**

**Downloads:** [Releases](https://github.com/DQMining/DRQ-Miner-Testing/releases)

| Asset | Platform |
|-------|----------|
| `DRQMiner-win64.zip` | Windows x64 |
| `drqminer-linux-x64.tar.gz` | Linux x64 |
| `drqminer-linux-arm64-phone.tar.gz` | Android Userland (arm64) |

```bash
TAG=v0.0.72   # use latest tag from Releases
wget "https://github.com/DQMining/DRQ-Miner-Testing/releases/download/${TAG}/drqminer-linux-x64.tar.gz"
tar xzf drqminer-linux-x64.tar.gz && chmod +x drqminer
./drqminer -V
```

**Example (nm/1):**

```bash
./drqminer -a nm/1 -o POOL:12427 -u WALLET.WORKER -p x
```

Also supports VerusHash and AstroBWT v3 (DERO). Alpha builds do **not** auto-update.

**Other channels:** [Beta](https://github.com/DQMining/DRQ-Miner-Beta) · [Stable](https://github.com/DQMining/DRQ-Miner)

[www.dqmining.com](https://www.dqmining.com) · [Discord](https://discord.gg/Nku5N3WSBm)
