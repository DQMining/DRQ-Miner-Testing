#!/usr/bin/env python3
"""Extract colored ASCII from asciiart.club saved HTML -> res/drq_logo_forum.txt lines."""
from __future__ import annotations

import html
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT = ROOT / "res" / "drq_logo_forum.txt"

SPAN_RE = re.compile(
    r'<span\s+style="color:#([0-9a-fA-F]{3,6})">(.*?)</span>',
    re.IGNORECASE | re.DOTALL,
)


def spans_to_bbcode_line(line_html: str) -> str:
    parts: list[str] = []
    for m in SPAN_RE.finditer(line_html):
        color = m.group(1)
        if len(color) == 3:
            color = "".join(c * 2 for c in color)
        ch = html.unescape(m.group(2))
        if ch in ("\r", "\n"):
            continue
        parts.append(f"[color=#{color}]{ch}[/color]")
    return "".join(parts)


def import_html(src: Path, out: Path = OUT) -> int:
    if not src.is_file():
        raise FileNotFoundError(src)

    text = src.read_text(encoding="utf-8", errors="replace")
    # Drop HTML wrapper; keep line breaks from <br> or newlines in <pre>
    text = re.sub(r"<br\s*/?>", "\n", text, flags=re.IGNORECASE)
    text = re.sub(r"</?pre[^>]*>", "\n", text, flags=re.IGNORECASE)

    lines_out: list[str] = []
    for raw_line in text.splitlines():
        if "style=\"color:" not in raw_line and "style='color:" not in raw_line:
            continue
        bb = spans_to_bbcode_line(raw_line)
        if bb:
            lines_out.append(bb)

    if not lines_out:
        # Single-line HTML blob
        bb = spans_to_bbcode_line(text)
        if bb:
            lines_out = [bb]

    if not lines_out:
        raise ValueError("No colored spans found in HTML")

    out.write_text("\n".join(lines_out) + "\n", encoding="utf-8")
    print(f"Wrote {len(lines_out)} lines to {out}")
    print(f"Sample width (tags): {lines_out[0].count('[color=')}")
    return len(lines_out)


def main() -> None:
    src = Path(sys.argv[1]) if len(sys.argv) > 1 else Path.home() / "Downloads/ascii-art.html"
    import_html(src)


if __name__ == "__main__":
    main()
