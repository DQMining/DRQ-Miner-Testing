#!/usr/bin/env python3
"""DRQ Proxy single-endpoint dev-fee router.

This proxy listens on one public endpoint (for example www.dqmining.com:3333),
detects the miner algorithm/protocol from initial traffic, and forwards the
stream to the configured upstream dev-fee route.
"""

from __future__ import annotations

import argparse
import asyncio
import json
import re
import signal
from dataclasses import dataclass
from pathlib import Path
from typing import Dict, List, Optional


@dataclass
class UpstreamRoute:
    key: str
    match_algos: List[str]
    upstream_host: str
    upstream_port: int
    enabled: bool = True


@dataclass
class ProxyConfig:
    listen_host: str
    listen_port: int
    routes: List[UpstreamRoute]
    fallback_route: str


def load_routes(path: Path) -> ProxyConfig:
    data = json.loads(path.read_text(encoding="utf-8"))
    bind = data.get("bind", {})
    routes: List[UpstreamRoute] = []
    for item in data.get("routes", []):
        routes.append(
            UpstreamRoute(
                key=item["key"],
                match_algos=[a.lower() for a in item.get("match_algos", [])],
                upstream_host=item["upstream_host"],
                upstream_port=int(item["upstream_port"]),
                enabled=bool(item.get("enabled", True)),
            )
        )
    enabled_routes = [r for r in routes if r.enabled]
    return ProxyConfig(
        listen_host=bind.get("host", "0.0.0.0"),
        listen_port=int(bind.get("port", 3333)),
        routes=enabled_routes,
        fallback_route=str(data.get("fallback_route", "fallback")),
    )


def extract_algo(payload: bytes) -> Optional[str]:
    text = payload.decode("utf-8", errors="ignore").lower()

    if "getblocktemplate" in text or '"method":"get_info"' in text:
        return "astrobwt/v3"

    for pattern in (
        r'"algo"\s*:\s*"([^"]+)"',
        r'"algorithm"\s*:\s*"([^"]+)"',
        r'"mining\.subscribe".*?"([^"]+)"',
        r'"method"\s*:\s*"mining\.subscribe"[^}]*"([^"/]+/[^"]+)"',
    ):
        match = re.search(pattern, text, re.DOTALL)
        if match:
            return match.group(1).strip()

    # Verus / eth-style login often names algo in params without "algo" key
    if "verushash" in text or "verus" in text:
        return "verushash"
    if "ghostrider" in text:
        return "ghostrider"
    if "nm/1" in text or "neuromorph" in text:
        return "nm/1"

    return None


def pick_route(algo: Optional[str], routes: List[UpstreamRoute], fallback_key: str) -> Optional[UpstreamRoute]:
    if algo:
        normalized = algo.lower()
        for route in routes:
            if normalized in route.match_algos:
                return route

    for route in routes:
        if route.key == fallback_key:
            return route

    return routes[0] if routes else None


async def pipe(reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
    try:
        while True:
            data = await reader.read(65536)
            if not data:
                break
            writer.write(data)
            await writer.drain()
    except (asyncio.CancelledError, ConnectionResetError, BrokenPipeError, OSError):
        pass
    finally:
        try:
            writer.close()
            await writer.wait_closed()
        except Exception:
            pass


async def handle_client(config: ProxyConfig, reader: asyncio.StreamReader, writer: asyncio.StreamWriter) -> None:
    peer = writer.get_extra_info("peername")
    try:
        first_payload = await reader.read(65536)
    except Exception as exc:
        print(f"[session] failed reading initial payload from {peer}: {exc}")
        writer.close()
        await writer.wait_closed()
        return

    if not first_payload:
        writer.close()
        await writer.wait_closed()
        return

    algo = extract_algo(first_payload)
    route = pick_route(algo, config.routes, config.fallback_route)
    if route is None:
        print(f"[session] no enabled routes for {peer}")
        writer.close()
        await writer.wait_closed()
        return

    algo_label = algo or "unknown"
    used_fallback = algo is None or all(algo_label.lower() not in r.match_algos for r in config.routes if r.key != config.fallback_route)
    if used_fallback:
        print(
            f"[{route.key}] FALLBACK connect from {peer} algo={algo_label} "
            f"-> {route.upstream_host}:{route.upstream_port}"
        )
    else:
        print(
            f"[{route.key}] connect from {peer} algo={algo_label} "
            f"-> {route.upstream_host}:{route.upstream_port}"
        )
    try:
        up_reader, up_writer = await asyncio.open_connection(route.upstream_host, route.upstream_port)
    except Exception as exc:
        print(f"[{route.key}] upstream connect failed: {exc}")
        writer.close()
        await writer.wait_closed()
        return

    up_writer.write(first_payload)
    await up_writer.drain()

    await asyncio.gather(
        pipe(reader, up_writer),
        pipe(up_reader, writer),
        return_exceptions=True,
    )
    print(f"[{route.key}] disconnect {peer}")


async def run(config: ProxyConfig) -> None:
    server = await asyncio.start_server(
        lambda r, w: handle_client(config, r, w),
        host=config.listen_host,
        port=config.listen_port,
    )
    print(f"[drq-proxy] listening on {config.listen_host}:{config.listen_port}")
    for route in config.routes:
        print(f"  - route {route.key}: {route.upstream_host}:{route.upstream_port} match={route.match_algos}")
    print(f"  - fallback: {config.fallback_route}")

    stop_event = asyncio.Event()

    def stop_handler() -> None:
        stop_event.set()

    loop = asyncio.get_running_loop()
    for sig in (signal.SIGINT, signal.SIGTERM):
        try:
            loop.add_signal_handler(sig, stop_handler)
        except NotImplementedError:
            pass

    await stop_event.wait()

    server.close()
    await server.wait_closed()


def main() -> int:
    parser = argparse.ArgumentParser(description="DRQ single-endpoint algo-aware proxy")
    parser.add_argument(
        "-c",
        "--config",
        default=str(Path(__file__).with_name("routes.template.json")),
        help="Path to JSON route config",
    )
    args = parser.parse_args()

    config_path = Path(args.config)
    if not config_path.is_file():
        print(f"Config not found: {config_path}")
        return 2

    config = load_routes(config_path)
    if not config.routes:
        print("No enabled routes in config.")
        return 3

    asyncio.run(run(config))
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
