# DRQ Proxy (single endpoint)

`drq_proxy.py` is a config-driven single-listener proxy used to route
dev-fee traffic by algorithm/protocol route.

## Why this exists

- Some algo/pool paths are inconsistent in stock `xmrig-proxy`.
- DRQ launch requires proxy-based dev-fee handling for all launch routes.

## Quick start

```bash
cd tools/drq-proxy
cp routes.template.json routes.json
python3 drq_proxy.py -c routes.json
```

As a service (Ubuntu):

```bash
cd tools/drq-proxy
chmod +x install_systemd.sh
./install_systemd.sh
journalctl -u drq-proxy -f
```

## Route model

The proxy listens on one endpoint (default `0.0.0.0:3333`) and each route has:

- `key` route id
- `match_algos` list for algo matching
- `upstream_host`, `upstream_port` (where traffic is forwarded)
- `enabled`

The proxy reads the first client payload, detects algo/protocol signals, and
chooses the upstream route:

- Detects `algo` from JSON login when available.
- Detects DERO daemon traffic from `getblocktemplate` / `get_info`.
- Uses `fallback_route` if algo cannot be detected.

## Launch defaults (template)

- bind: `0.0.0.0:3333`
- `rx0` -> `eu.hashmonkeys.cloud:12439`
- `ghostrider` -> `eu.hashmonkeys.cloud:12011`
- `verus` -> `usw.vipor.net:5040`
- `dero` -> `dero.rabidmining.com:10300`
- `fallback` -> RX/0 route

## Ops notes

- Open required TCP ports in firewall.
- Keep real wallets/secrets outside public repo.
- Use systemd service for persistence in production.
- For persistent operator values, keep non-secret defaults in `doc/OPERATOR_DEFAULTS.md` and private values in `tools/drq-proxy/.env.operator` (see `.env.operator.example`).
