#!/usr/bin/env python3
"""Render res/drq_logo_source.png -> DrqBannerLogo.inc.h (half-block truecolor)."""
from __future__ import annotations

import argparse
from pathlib import Path

from PIL import Image

ROOT = Path(__file__).resolve().parents[1]
OUT_H = ROOT / "src" / "branding" / "DrqBannerLogo.inc.h"
DEFAULT_PNG = ROOT / "res" / "drq_logo_source.png"

Pixel = tuple[str, int, int, int]
HALF = "\u2580"  # ▀ upper half block (fg=top, bg=bottom)


def lum(r: int, g: int, b: int) -> int:
    return (54 * r + 183 * g + 19 * b) >> 8


def is_background(r: int, g: int, b: int, a: int = 255) -> bool:
    if a < 20:
        return True
    if lum(r, g, b) < 22:
        return True
    return False


def composite_on_black(img: Image.Image) -> Image.Image:
    img = img.convert("RGBA")
    bg = Image.new("RGBA", img.size, (0, 0, 0, 255))
    return Image.alpha_composite(bg, img)


def crop_content(img: Image.Image, pad: int = 8) -> Image.Image:
    rgba = img.convert("RGBA")
    w, h = rgba.size
    mask = []
    for y in range(h):
        for x in range(w):
            r, g, b, a = rgba.getpixel((x, y))
            if not is_background(r, g, b, a):
                mask.append((x, y))
    if not mask:
        return img
    xs = [p[0] for p in mask]
    ys = [p[1] for p in mask]
    x0 = max(0, min(xs) - pad)
    y0 = max(0, min(ys) - pad)
    x1 = min(w, max(xs) + pad + 1)
    y1 = min(h, max(ys) + pad + 1)
    return rgba.crop((x0, y0, x1, y1))


def resize_logo(img: Image.Image, px_w: int, px_h: int) -> Image.Image:
    """Resize to pixel buffer (width x height). Height should be even for half-blocks."""
    px_h = max(8, px_h)
    if px_h % 2:
        px_h += 1
    px_w = max(8, px_w)
    return img.resize((px_w, px_h), Image.Resampling.LANCZOS)


def image_to_halfblock_rows(img: Image.Image, stretch_x: int = 1) -> list[list[Pixel]]:
    """Each terminal cell: one HALF char, we store as special tuple or encode in ansi directly."""
    img = img.convert("RGBA")
    w, h = img.size
    if h % 2:
        img = img.crop((0, 0, w, h - 1))
        h -= 1
    term_rows = h // 2
    rows: list[list[tuple[tuple[int, int, int], tuple[int, int, int], bool]]] = []
    for ty in range(term_rows):
        row = []
        y0, y1 = ty * 2, ty * 2 + 1
        for x in range(w):
            t = img.getpixel((x, y0))
            b = img.getpixel((x, y1))
            tr, tg, tb, ta = t
            br, bg, bb, ba = b
            t_bg = is_background(tr, tg, tb, ta)
            b_bg = is_background(br, bg, bb, ba)
            cell = ((tr, tg, tb), (br, bg, bb), t_bg and b_bg)
            for _ in range(stretch_x):
                row.append(cell)
        rows.append(row)
    return rows


def trim_half_rows(
    rows: list[list[tuple[tuple[int, int, int], tuple[int, int, int], bool]]],
) -> list[list[tuple[tuple[int, int, int], tuple[int, int, int], bool]]]:
    def any_vis(r):
        return any(not c[2] for c in r)

    while rows and not any_vis(rows[0]):
        rows.pop(0)
    while rows and not any_vis(rows[-1]):
        rows.pop()
    if not rows:
        return rows
    w = max(len(r) for r in rows)
    left, right = w, 0
    for r in rows:
        for i, c in enumerate(r):
            if not c[2]:
                left = min(left, i)
                right = max(right, i)
    return [r[left : right + 1] for r in rows]


def cell_to_ansi(top: tuple[int, int, int], bot: tuple[int, int, int], empty: bool) -> str:
    if empty:
        return " "
    tr, tg, tb = top
    br, bg, bb = bot
    return f"\033[38;2;{tr};{tg};{tb}m\033[48;2;{br};{bg};{bb}m{HALF}"


def cell_to_plain(top: tuple[int, int, int], bot: tuple[int, int, int], empty: bool) -> str:
    if empty:
        return " "
    # ASCII only — MSVC may compile DrqBannerLogo.inc.h without /utf-8.
    return "#" if lum(*top) > lum(*bot) else ":"


def row_to_ansi_half(row) -> str:
    parts = [cell_to_ansi(t, b, e) for t, b, e in row]
    return "".join(parts) + "\033[0m"


def row_to_plain_half(row) -> str:
    return "".join(cell_to_plain(t, b, e) for t, b, e in row)


def c_escape(s: str) -> str:
    out: list[str] = []
    for ch in s:
        if ch == "\\":
            out.append("\\\\")
        elif ch == '"':
            out.append('\\"')
        elif ch == "\033":
            out.append("\\033")
        elif ord(ch) < 0x20:
            out.append(f"\\x{ord(ch):02x}")
        elif ord(ch) > 127:
            out.append(f"\\u{ord(ch):04x}")
        else:
            out.append(ch)
    return "".join(out)


# MSVC rejects single string literals longer than ~2048 escaped bytes.
_MSVC_ESCAPED_MAX = 1900


def emit_c_concat_literal(parts: list[str]) -> str:
    return " ".join(f'"{c_escape(p)}"' for p in parts)


def split_ansi_row(row) -> list[str]:
    """Split one terminal row into C string chunks on cell boundaries."""
    chunks: list[str] = []
    buf: list[str] = []
    esc_len = 0
    for top, bot, empty in row:
        piece = cell_to_ansi(top, bot, empty)
        piece_esc = len(c_escape(piece))
        if buf and esc_len + piece_esc > _MSVC_ESCAPED_MAX:
            chunks.append("".join(buf))
            buf = [piece]
            esc_len = piece_esc
        else:
            buf.append(piece)
            esc_len += piece_esc
    if buf:
        chunks.append("".join(buf))
    chunks[-1] = chunks[-1] + "\033[0m" if chunks else ""
    return chunks


def emit_header(rows_half) -> str:
    plain = [row_to_plain_half(r) for r in rows_half]
    lines = [
        "/* Auto-generated by scripts/gen_drq_logo_from_png.py (half-block) */",
        "#ifndef XMRIG_DRQBANNERLOGO_INC_H",
        "#define XMRIG_DRQBANNERLOGO_INC_H",
        "",
        "#include <cstddef>",
        "",
        "namespace xmrig {",
        "",
        f"static constexpr size_t kDrqLogoRows = {len(rows_half)};",
        "",
        "static const char *const kDrqLogoPlain[] = {",
    ]
    for p in plain:
        lines.append(f'    "{c_escape(p)}",')
    lines.append("};")
    lines.append("")
    lines.append("static const char *const kDrqLogoColor[] = {")
    for row in rows_half:
        parts = split_ansi_row(row)
        lines.append(f"    {emit_c_concat_literal(parts)},")
    lines.append("};")
    lines += [
        "",
        "static_assert(sizeof(kDrqLogoPlain) / sizeof(kDrqLogoPlain[0]) == kDrqLogoRows, \"logo plain rows\");",
        "static_assert(sizeof(kDrqLogoColor) / sizeof(kDrqLogoColor[0]) == kDrqLogoRows, \"logo color rows\");",
        "",
        "} // namespace xmrig",
        "",
        "#endif",
        "",
    ]
    return "\n".join(lines)


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--png", type=Path, default=DEFAULT_PNG)
    ap.add_argument(
        "--width",
        type=int,
        default=110,
        help="Image pixel width before --stretch (horizontal unchanged when tuning height)",
    )
    ap.add_argument(
        "--height",
        type=int,
        default=38,
        help="Terminal rows (half-block); increase only this to stretch vertically",
    )
    ap.add_argument(
        "--v-stretch",
        type=float,
        default=1.0,
        help="Extra vertical pixel scale only (>1 = taller, does not change width)",
    )
    ap.add_argument(
        "--stretch",
        type=int,
        default=2,
        help="Horizontal cells per image column (keep at 2 for cmd aspect)",
    )
    args = ap.parse_args()

    if not args.png.is_file():
        raise SystemExit(f"PNG not found: {args.png}")

    img = composite_on_black(Image.open(args.png))
    img = crop_content(img)
    px_h = int(args.height * 2 * args.v_stretch)
    img = resize_logo(img, args.width, px_h)
    rows = trim_half_rows(image_to_halfblock_rows(img, stretch_x=max(1, args.stretch)))
    if not rows:
        raise SystemExit("No visible pixels")

    OUT_H.write_text(emit_header(rows), encoding="utf-8")
    w = max(len(r) for r in rows)
    print(f"Wrote {OUT_H} ({len(rows)} rows x {w} cells, half-block) from {args.png}")


if __name__ == "__main__":
    main()
