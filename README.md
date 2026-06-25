# DRQ Miner — Alpha

Pre-release builds for internal testing. **Not for production.**

**Downloads:** [Releases](https://github.com/DQMining/DRQ-Miner-Testing/releases)

| Asset | Platform |
|-------|----------|
| `DRQMiner-win64.zip` | Windows x64 |
| `drqminer-linux-x64.tar.gz` | Linux x64 |
| `drqminer-linux-arm64-phone.tar.gz` | Android Userland (arm64) |

## Phone (Userland) — copy/paste

```bash
mkdir -p ~/Miners && cd ~/Miners
wget -O drqminer-linux-arm64-phone.tar.gz \
  "https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v0.1.75/drqminer-linux-arm64-phone.tar.gz"
tar xzf drqminer-linux-arm64-phone.tar.gz
chmod +x drqminer mine-nm.sh
nano mine-nm.sh    # set YOUR_WALLET.WORKER
./mine-nm.sh
```

Or one line (replace wallet):

```bash
./drqminer -a nm/1 -o us-east.hashmonkeys.cloud:12427 -u YOUR_WALLET.WORKER -p x -t 8
```

Always use **`-p x`** for the pool password. No extra `export` commands needed.

## Linux x64

```bash
TAG=v0.1.75
wget "https://github.com/DQMining/DRQ-Miner-Testing/releases/download/${TAG}/drqminer-linux-x64.tar.gz"
tar xzf drqminer-linux-x64.tar.gz && chmod +x drqminer
./drqminer -V
```

Alpha builds do **not** auto-update.

**Other channels:** [Beta](https://github.com/DQMining/DRQ-Miner-Beta) · [Stable](https://github.com/DQMining/DRQ-Miner)

[www.dqmining.com](https://www.dqmining.com) · [Discord](https://discord.gg/Nku5N3WSBm)
