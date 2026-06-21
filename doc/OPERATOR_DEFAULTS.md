# DRQ Operator Defaults

Use this file as the canonical source for stable operator values so setup does not need to be repeated in chat.

## Canonical Network Defaults

- Dev-fee proxy endpoint: `www.dqmining.com:3333`
- Proxy model: single endpoint, algo-aware routing inside DRQ Proxy
- Donate default: `0.5`
- Donate minimum: `0.01`

## Infrastructure Defaults

- Primary proxy OS: Ubuntu 22.04
- Initial deployment location: home-hosted machine (migrate to VPS later)

## GitHub Defaults

Fill once and keep updated.

- Private repo URL: `https://github.com/DQMining/drq-miner-private`
- Default branch: `main`

## Proxy Access Defaults

Fill once and keep updated.

- Proxy SSH user: `dq111`
- Proxy host/IP: `192.168.0.129` (LAN); public `www.dqmining.com:3333`
- SSH port: `22`

## Dev-Fee Route Policy

- All dev-fee traffic goes to the single endpoint above.
- DRQ Proxy performs route selection by detected algorithm/protocol.
- Fallback route must always be enabled.

## Secret Handling Policy

- Do not put wallets, tokens, passwords, or private keys in this file.
- Put secrets in local-only files:
  - repo root `.env.operator`
  - `tools/drq-proxy/.env.operator`

## Current Launch Scope

- Required launch routes: `rx/0`, `verushash`, `astrobwt/v3` (DERO), `ghostrider`
- Planned additions: `bc3-sha3-256t`, `sha256t`, `janushash`, `pearl`

## Pearl (PRL) test defaults

- Pool integration spec (authoritative): `doc/PEARL_HASHMONKEYS_INTEGRATION.md`
- DRQ roadmap: `doc/PEARL_INTEGRATION.md`
- Pool id: `eu-pearl-dfppsplus`
- Friend pool: `eu.hashmonkeys.cloud:12437`
- Official reference repo: https://github.com/pearl-research-labs/pearl
- Reference miner: `alphaminetech/pearl-miner:1.6.0`+
