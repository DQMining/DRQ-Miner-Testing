# Userland — download and mine (public repo)

Repo: **https://github.com/DQMining/DRQ-Miner-Testing** (public)  
Latest release: **v1.0.10** — nm/1 (Neuromorph), VAES-256 on x64, ARM-AES on phones  
Linux x64 prebuilt is **fully static** (no `apt install libhwloc` needed).

## Download (phone — arm64)

```bash
cd ~
wget -O drqminer-linux-arm64-phone.tar.gz \
  https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.10/drqminer-linux-arm64-phone.tar.gz
tar xzf drqminer-linux-arm64-phone.tar.gz
chmod +x drqminer
./drqminer -V
```

## Download (Linux x64 rig — prebuilt, no compiler needed)

```bash
cd ~/Miners
wget -O drqminer-linux-x64.tar.gz \
  https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.10/drqminer-linux-x64.tar.gz
tar xzf drqminer-linux-x64.tar.gz
chmod +x drqminer
./drqminer -V
```

Or use `bash scripts/linux/fetch_prebuilt_x64.sh v1.0.10 YOUR_WALLET`.

## Mine Neuromorph (nm/1, Cereblix)

```bash
./drqminer -a nm/1 \
  -o eu.hashmonkeys.cloud:12427 \
  -u YOUR_CEREBLIX_WALLET \
  -p x
```

Log should show `nm scratch fill: VAES-256` (Intel/AMD x64) or `ARM-AES` (phone). x64 rigs use physical-core thread count automatically when hwloc is available.

## Build from source (x64 rig without prebuilt)

```bash
sudo bash scripts/linux/install_build_deps.sh   # git, cmake, libssl-dev, libhwloc-dev
git clone https://github.com/DQMining/DRQ-Miner-Testing.git
cd DRQ-Miner-Testing
cmake --preset linux-x64-Release
cmake --build out/build/linux-x64-Release -j$(nproc)
./out/build/linux-x64-Release/drqminer -a nm/1 -o POOL -u WALLET
```

## Mine DERO (Rabid, CPU)

Plain pool address — no `wss://` or `--daemon` required (miner upgrades to WSS daemon mode automatically):

```bash
./drqminer -a astrobwtv3/dero \
  -o dero.rabidmining.com:10300 \
  -u YOUR_DERO_WALLET \
  -p x -t 8
```

Aliases: `astrobwt/v3`, `astrobwt/dero`, `astrobwtv3`. Replace `YOUR_DERO_WALLET` with your address. On big.LITTLE phones (e.g. Galaxy S20), **`-t 8`** usually beats `-t 4`. Target **~3.5–4.2 kH/s** on v1.0.8 (TNN path). Fallback: **v1.0.6** (~1.8–2.2 kH/s) if shares fail.

## Mine Verus (VRSC, CPU)

```bash
./drqminer -a verushash \
  -o usw.vipor.net:5040 \
  -u YOUR_WALLET.worker \
  -p x -t 4
```

Use any Verus stratum pool. Phone hashrate is modest (thermal-limited); desktop Release builds are much faster.

## Dev-fee proxy behavior (launch)

- `donate-level` supports decimals (for example `0.5`, `0.1`, `0.01`).
- Dev-fee routing is proxy-oriented by algorithm route; see `doc/DEVFEE_PROXY_BOOTSTRAP.md`.
- Keep production wallets out of repo files; use private proxy configs.

## Other releases (v1.0.10)

| Asset | URL |
|-------|-----|
| Windows | `.../releases/download/v1.0.10/DRQMiner-win64.zip` |
| Linux x64 | `.../releases/download/v1.0.10/drqminer-linux-x64.tar.gz` |
| Linux arm64 (phone) | `.../releases/download/v1.0.10/drqminer-linux-arm64-phone.tar.gz` |

Base: `https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.10/`
