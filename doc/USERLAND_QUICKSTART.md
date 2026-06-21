# Userland — download and mine (public repo)

Repo: **https://github.com/DQMining/DRQ-Miner-Testing** (public)  
Latest phone build: **v1.0.8** (TNN NEON + AstroSPSA — use after CI green)

## Download

```bash
cd ~
wget -O drqminer-linux-arm64-phone.tar.gz \
  https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.8/drqminer-linux-arm64-phone.tar.gz
tar xzf drqminer-linux-arm64-phone.tar.gz
chmod +x drqminer
./drqminer -V
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

## Other releases (v1.0.8)

| Asset | URL |
|-------|-----|
| Windows | `.../releases/download/v1.0.8/DRQMiner-win64.zip` |
| Linux x64 | `.../releases/download/v1.0.8/drqminer-linux-x64.tar.gz` |

Base: `https://github.com/DQMining/DRQ-Miner-Testing/releases/download/v1.0.8/`
