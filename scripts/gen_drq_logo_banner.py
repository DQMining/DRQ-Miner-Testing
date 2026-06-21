#!/usr/bin/env python3
"""Parse asciiart.club BBCode or plain ASCII -> DrqBannerLogo.inc.h."""
from __future__ import annotations

import argparse
import html
import json
import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
OUT_H = ROOT / "src" / "branding" / "DrqBannerLogo.inc.h"
SRC_BB = ROOT / "res" / "drq_logo_forum.txt"
SRC_ASCII = ROOT / "res" / "drq_logo_ascii.txt"
MAX_WIDTH = 140
TRANSCRIPT = (
    Path.home()
    / ".cursor/projects/c-Users-DREADQUILL11168-Documents-Mining-Software-Astrobwt-Comparing-xmrig-Verus-Fast-xmrig-test-master/agent-transcripts/15f47c7b-8430-4e36-9ec3-40ae747e05bb/15f47c7b-8430-4e36-9ec3-40ae747e05bb.jsonl"
)

Pixel = tuple[str, int, int, int]

TAG_RE = re.compile(
    r"\[color=#([0-9a-fA-F]{3,6})\](.*?)\[/color\]|"
    r'<span style="color:#([0-9a-fA-F]{3,6})">(.*?)</span>',
    re.DOTALL,
)

# Plain "Basic Text" from asciiart.club -> approximate truecolor
ASCII_PALETTE: dict[str, tuple[int, int, int]] = {
    " ": (0, 0, 0),
    ".": (55, 35, 30),
    ":": (85, 50, 42),
    "-": (70, 45, 38),
    "=": (100, 55, 45),
    "+": (160, 70, 55),
    "*": (190, 90, 60),
    "#": (45, 12, 12),
    "%": (130, 25, 22),
}


def hex_rgb(h: str) -> tuple[int, int, int]:
    h = h.lower()
    if len(h) == 3:
        h = "".join(c * 2 for c in h)
    return int(h[0:2], 16), int(h[2:4], 16), int(h[4:6], 16)


def is_background(r: int, g: int, b: int) -> bool:
    spread = max(r, g, b) - min(r, g, b)
    avg = (r + g + b) // 3
    if avg > 100 and spread < 28:
        return True
    if r < 10 and g < 10 and b < 8:
        return True
    return False


def clean_markup(line: str) -> str:
    line = html.unescape(line)
    line = re.sub(r"\[/?(?:size|font)[^\]]*\]", "", line)
    line = re.sub(r"<span[^>]*>", "", line)
    return line.replace("</span>", "")


def parse_bbcode_line(line: str) -> list[Pixel]:
    line = clean_markup(line)
    out: list[Pixel] = []
    pos = 0
    for m in TAG_RE.finditer(line):
        if m.start() > pos:
            for ch in line[pos : m.start()]:
                if ch not in "\r\n":
                    out.append((" ", 0, 0, 0))
        h = m.group(1) or m.group(3)
        text = m.group(2) or m.group(4) or ""
        r, g, b = hex_rgb(h)
        for ch in text:
            if ch in "\r\n":
                continue
            if is_background(r, g, b) or ch == " ":
                out.append((" ", 0, 0, 0))
            else:
                out.append((ch, r, g, b))
        pos = m.end()
    if pos < len(line):
        for ch in line[pos:]:
            if ch not in "\r\n":
                out.append((" ", 0, 0, 0))
    return out


def parse_ascii_char(ch: str) -> Pixel:
    if ch in "\r\n":
        return (" ", 0, 0, 0)
    rgb = ASCII_PALETTE.get(ch, (90, 60, 50))
    if ch == " ":
        return (" ", 0, 0, 0)
    return (ch, rgb[0], rgb[1], rgb[2])


def parse_ascii_text(text: str, wrap: int) -> list[list[Pixel]]:
    text = text.strip()
    if not text:
        return []
    lines_raw = text.splitlines()
    if len(lines_raw) == 1 and wrap > 0:
        s = lines_raw[0]
        lines_raw = [s[i : i + wrap] for i in range(0, len(s), wrap)]
    rows: list[list[Pixel]] = []
    for ln in lines_raw:
        if not ln.strip():
            continue
        rows.append([parse_ascii_char(c) for c in ln])
    return rows


def downsample_row(row: list[Pixel], scale: int) -> list[Pixel]:
    if scale <= 1:
        return row
    return row[::scale]


def trim_row(row: list[Pixel]) -> list[Pixel]:
    start = 0
    end = len(row)
    while start < end and row[start][0] == " ":
        start += 1
    while end > start and row[end - 1][0] == " ":
        end -= 1
    return row[start:end]


def trim_rows(rows: list[list[Pixel]]) -> list[list[Pixel]]:
    rows = [trim_row(r) for r in rows if any(ch != " " for ch, *_ in r)]
    while rows and not any(ch != " " for ch, *_ in rows[0]):
        rows.pop(0)
    while rows and not any(ch != " " for ch, *_ in rows[-1]):
        rows.pop()
    return rows


def stretch_for_console(row: list[Pixel]) -> list[Pixel]:
    out: list[Pixel] = []
    for px in row:
        out.append(px)
        out.append(px)
    return out


def center_rows(rows: list[list[Pixel]]) -> list[list[Pixel]]:
    if not rows:
        return rows
    width = max(len(r) for r in rows)
    centered: list[list[Pixel]] = []
    for row in rows:
        pad = (width - len(row)) // 2
        centered.append([(" ", 0, 0, 0)] * pad + row + [(" ", 0, 0, 0)] * (width - len(row) - pad))
    return centered


def row_to_plain(row: list[Pixel]) -> str:
    return "".join(ch for ch, *_ in row)


def row_to_ansi(row: list[Pixel]) -> str:
    parts: list[str] = []
    cur_rgb: tuple[int, int, int] | None = None
    buf: list[str] = []

    def flush() -> None:
        nonlocal cur_rgb, buf
        if not buf:
            return
        if cur_rgb is None:
            parts.append("".join(buf))
        else:
            r, g, b = cur_rgb
            parts.append(f"\033[38;2;{r};{g};{b}m{''.join(buf)}")
        buf = []

    for ch, r, g, b in row:
        if ch == " ":
            flush()
            cur_rgb = None
            parts.append(" ")
            continue
        rgb = (r, g, b)
        if rgb != cur_rgb:
            flush()
            cur_rgb = rgb
        buf.append(ch)
    flush()
    return "".join(parts)


def c_escape(s: str) -> str:
    out: list[str] = []
    for ch in s:
        o = ord(ch)
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\n":
            out.append("\\n")
        elif ch == "\033":
            out.append("\\033")
        elif o < 0x20 or o == 0x7F:
            out.append(f"\\x{o:02x}")
        elif o < 0x80:
            out.append(ch)
        else:
            out.append(ch)
    return "".join(out)


def emit_header(rows: list[list[Pixel]]) -> str:
    plain = [row_to_plain(r) for r in rows]
    ansi = [row_to_ansi(r) for r in rows]
    lines = [
        "/* Auto-generated by scripts/gen_drq_logo_banner.py — do not edit. */",
        "#ifndef XMRIG_DRQBANNERLOGO_INC_H",
        "#define XMRIG_DRQBANNERLOGO_INC_H",
        "",
        "#include <cstddef>",
        "",
        "namespace xmrig {",
        "",
        f"static constexpr size_t kDrqLogoRows = {len(rows)};",
        "",
        "static const char *kDrqLogoPlain[kDrqLogoRows] = {",
    ]
    for p in plain:
        lines.append(f'    "{c_escape(p)}",')
    lines.append("};")
    lines.append("")
    lines.append("static const char *kDrqLogoColor[kDrqLogoRows] = {")
    for a in ansi:
        lines.append(f'    "{c_escape(a)}",')
    lines.append("};")
    lines += [
        "",
        "} // namespace xmrig",
        "",
        "#endif",
        "",
    ]
    return "\n".join(lines)


def load_bbcode(path: Path) -> list[list[Pixel]]:
    raw_lines = [
        ln
        for ln in path.read_text(encoding="utf-8").splitlines()
        if "[color=" in ln or "<span" in ln
    ]
    return [parse_bbcode_line(ln) for ln in raw_lines]


def auto_scale(width: int, target: int) -> int:
    if width <= target:
        return 1
    return max(1, (width + target - 1) // target)


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--source", type=Path, default=None, help="Input file")
    parser.add_argument(
        "--html",
        type=Path,
        default=None,
        help="asciiart.club saved HTML (imports to res/drq_logo_forum.txt first)",
    )
    parser.add_argument(
        "--scale",
        type=int,
        default=0,
        help="Keep every Nth column/row pixel (0 = auto for wide BBCode)",
    )
    parser.add_argument(
        "--ascii-width",
        type=int,
        default=100,
        help="Wrap plain ASCII to this width if one long line",
    )
    parser.add_argument(
        "--no-stretch",
        action="store_true",
        help="Disable 2x horizontal stretch for console aspect",
    )
    args = parser.parse_args()

    if args.html:
        from import_ascii_html import import_html

        import_html(args.html)

    src = args.source
    if src is None:
        if SRC_ASCII.is_file() and SRC_ASCII.stat().st_size > 100:
            src = SRC_ASCII
        else:
            src = SRC_BB

    if not src.is_file():
        sys.exit(f"Missing source file: {src}")

    text_head = src.read_text(encoding="utf-8", errors="replace")[:4096]
    use_ascii = "[color=" not in text_head and "%" in text_head

    if use_ascii:
        rows = parse_ascii_text(src.read_text(encoding="utf-8", errors="replace"), args.ascii_width)
        print(f"Mode: plain ASCII from {src}")
    else:
        rows = load_bbcode(src)
        print(f"Mode: BBCode from {src}")

    if not rows:
        sys.exit("No logo pixels parsed")

    raw_width = max(len(r) for r in rows)
    scale = args.scale if args.scale > 0 else auto_scale(raw_width, MAX_WIDTH)
    if scale > 1:
        rows = [downsample_row(r, scale) for r in rows]
        rows = rows[::scale] if len(rows) > 80 else rows
        print(f"Downsampled {scale}x (source width {raw_width})")

    rows = trim_rows(rows)
    if not args.no_stretch:
        rows = [stretch_for_console(r) for r in rows]
    rows = center_rows(rows)

    OUT_H.write_text(emit_header(rows), encoding="utf-8")
    w = max(len(r) for r in rows)
    print(f"Wrote {OUT_H} ({len(rows)} rows x {w} cols)")


if __name__ == "__main__":
    main()
