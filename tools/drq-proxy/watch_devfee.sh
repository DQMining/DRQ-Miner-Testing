#!/bin/bash
# Watch drq-proxy dev-fee connections and append to a log file.
# Run on the proxy rig before you step away:
#   chmod +x watch_devfee.sh
#   ./watch_devfee.sh
# Review later:
#   grep connect ~/drq-proxy/devfee-watch.log
#   tail -f ~/drq-proxy/devfee-watch.log

set -euo pipefail

LOG="${DRQ_DEVFEE_LOG:-$HOME/drq-proxy/devfee-watch.log}"
UNIT="${DRQ_PROXY_UNIT:-drq-proxy}"
INTERVAL="${DRQ_SNAPSHOT_SEC:-30}"

mkdir -p "$(dirname "$LOG")"

stamp() { date '+%Y-%m-%d %H:%M:%S'; }

echo "[$(stamp)] dev-fee watch started (unit=$UNIT log=$LOG)" | tee -a "$LOG"
echo "[$(stamp)] listeners:" | tee -a "$LOG"
ss -tlnp 2>/dev/null | grep -E ':3333|:3334|:3335|:3336|:3337' | tee -a "$LOG" || true
echo "---" | tee -a "$LOG"

snapshot_connections() {
    local snap
    snap="$(ss -tnp 2>/dev/null | grep -E ':3333|:3334|:3335|:3336|:3337' || true)"
    if [[ -n "$snap" ]]; then
        echo "[$(stamp)] ACTIVE TCP:" | tee -a "$LOG"
        echo "$snap" | tee -a "$LOG"
    fi
}

(
    while true; do
        snapshot_connections
        sleep "$INTERVAL"
    done
) &
SNAP_PID=$!
trap 'kill "$SNAP_PID" 2>/dev/null || true' EXIT INT TERM

journalctl -u "$UNIT" -f --since "1 min ago" 2>/dev/null | while IFS= read -r line; do
    ts="$(stamp)"
    if echo "$line" | grep -qE 'connect from|disconnect|upstream connect failed|failed reading|no enabled routes'; then
        echo "[$ts] $line" | tee -a "$LOG"
    elif echo "$line" | grep -qE 'ConnectionResetError|Traceback'; then
        echo "[$ts] RESET $line" | tee -a "$LOG"
    fi
done
