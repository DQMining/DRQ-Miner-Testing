# Userland — download and mine (public repo)

Repo: **https://github.com/DQMining/DRQ-Miner-Testing** (public)  
Latest phone build: **v1.0.7** (TNN-class ARM path: AstroSPSA + NEON branch compute)

## Download

```bash
cd ~
wget -O drqminer-linux-arm64-phone.tar.gz \
  https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.7/drqminer-linux-arm64-phone.tar.gz
tar xzf drqminer-linux-arm64-phone.tar.gz
chmod +x drqminer
./drqminer -V
```

## Mine DERO (Rabid, CPU)

```bash
./drqminer --daemon -a astrobwt/v3 \
  -o wss://dero.rabidmining.com:10300 \
  -u YOUR_DERO_WALLET \
  -p x -t 4
```

Replace `YOUR_DERO_WALLET` with your address. Adjust `-t` to thread count.

## Other releases

| Asset | URL |
|-------|-----|
| Windows | `.../releases/download/v1.0.4/DRQMiner-win64.zip` |
| Linux x64 | `.../releases/download/v1.0.4/drqminer-linux-x64.tar.gz` |

Base: `https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.4/`
