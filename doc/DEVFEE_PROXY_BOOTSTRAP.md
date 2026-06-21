# Dev-Fee Proxy Bootstrap

This guide bootstraps DRQ Miner dev-fee routing with your own proxy.

## Goals

- Keep wallets out of miner source code.
- Route dev-fee shares by algo/coin in proxy config.
- Support decimal donate levels (for example `0.5`, `0.1`, `0.01`).

## Miner-side defaults

- `donate-level` supports decimals down to `0.01`.
- `donate-over-proxy` modes:
  - `0`: never use proxy donate
  - `1`: auto (fallback behavior)
  - `2`: always use proxy donate

Recommended starter:

```json
{
  "donate-level": 0.5,
  "donate-over-proxy": 1
}
```

## Proxy-side routing

Use `scripts/proxy/devfee-routing.template.json` as a starter template.
For a launch-ready custom proxy runtime, use `tools/drq-proxy/`.

Fill:

- `proxy.bind.host`
- `proxy.bind.port`
- `proxy.bind.tls_port` (optional)
- `routes.*.wallet`

## DERO temporary path

If proxy DERO endpoint is not ready yet, set a temporary upstream endpoint for DERO in your proxy routing config and move to your own endpoint later.

## Operator checklist

1. Configure proxy host/ports and TLS.
2. Fill wallets per algorithm:
   - `verushash`
   - `astrobwt/v3` (DERO)
   - `rx/0`
   - `ghostrider`
   - future: `bc3-sha3-256t`, `sha256t`, `janushash`, `pearl`
3. Run miner with a low donate level first (for example `0.1`) and verify shares.
4. Raise to production target.

## Notes

- Do not commit real wallets/secrets in public repo files.
- Keep production proxy config in private infra repo or secret manager.
- Miner donate endpoint defaults to `www.dqmining.com:3333`.
- Miner donate endpoint can be overridden with env vars:
  - `DRQ_DONATE_PROXY_HOST`
  - `DRQ_DONATE_PROXY_PORT`
  - `DRQ_DONATE_PROXY_TLS_PORT` (optional TLS endpoint)
