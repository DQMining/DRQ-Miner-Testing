#!/usr/bin/env python3
"""Emit DrqBanner.cpp art arrays (run from repo root)."""
import pyfiglet


def trim_art(text: str) -> list[str]:
    lines = text.splitlines()
    while lines and not lines[-1].strip():
        lines.pop()
    while lines and not lines[0].strip():
        lines.pop(0)
    return lines


def esc_cpp(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def main() -> None:
    drq = trim_art(pyfiglet.Figlet(font="doh").renderText("DRQ"))
    miner = trim_art(pyfiglet.Figlet(font="epic").renderText("MINER"))
    print(f"// DRQ {len(drq)} lines, MINER {len(miner)} lines")
    for name, lines in [("kDrqArt", drq), ("kMinerArt", miner)]:
        print(f"    static const char *{name}[] = {{")
        for line in lines:
            print(f'        "{esc_cpp(line)}",')
        print("    };")
        print(f"    static const size_t {name}Count = sizeof({name}) / sizeof({name}[0]);")
        print()


if __name__ == "__main__":
    main()
