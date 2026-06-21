# Dev-Fee Validation Runbook

Use this before `0.0.1` launch to prove payouts are working.

## 1) Temporary high donate setting

Set `donate-level` to `5.0` for short validation only.

## 2) Route matrix (single proxy endpoint)

- Miner dev-fee endpoint -> `www.dqmining.com:3333` (or env override)
- Proxy route `rx0` -> upstream RX/0 dev pool
- Proxy route `ghostrider` -> upstream GRR dev pool
- Proxy route `verus` -> upstream VRSC dev pool
- Proxy route `dero` -> upstream DERO dev pool
- Proxy route `fallback` -> safe default upstream

## 3) Proof checklist (per route)

- Run miner for 10–15 minutes on each route target algo.
- Capture:
  - Miner console showing `DONATE 5.00%`
  - Proxy log showing detected algo and selected route
  - Pool dashboard screenshot with accepted shares to dev wallet

## 4) Restore release defaults

- Set `donate-level` back to `0.5`.
- Keep decimal support enabled.

## 5) Launch gate

Do not ship until each launch algo has at least one accepted-share proof.
