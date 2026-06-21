#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "$0")" && pwd)"
SERVICE_FILE="/etc/systemd/system/drq-proxy.service"
ROUTES_FILE="${ROOT_DIR}/routes.json"
PYTHON_BIN="${PYTHON_BIN:-/usr/bin/python3}"

if [ ! -f "${ROUTES_FILE}" ]; then
  cp "${ROOT_DIR}/routes.template.json" "${ROUTES_FILE}"
  echo "Created ${ROUTES_FILE} from template."
fi

sudo tee "${SERVICE_FILE}" >/dev/null <<EOF
[Unit]
Description=DRQ Proxy (multi-route devfee proxy)
After=network-online.target
Wants=network-online.target

[Service]
Type=simple
WorkingDirectory=${ROOT_DIR}
ExecStart=${PYTHON_BIN} ${ROOT_DIR}/drq_proxy.py -c ${ROUTES_FILE}
Restart=always
RestartSec=3
LimitNOFILE=65535

[Install]
WantedBy=multi-user.target
EOF

sudo systemctl daemon-reload
sudo systemctl enable drq-proxy
sudo systemctl restart drq-proxy
sudo systemctl status drq-proxy --no-pager || true
