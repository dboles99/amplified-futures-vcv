#!/usr/bin/env python3
"""Convert VCV Rack panel SVG root dimensions from px-like units to mm.

VCV's panel guide requires millimetres for the SVG document dimensions.
The viewBox and all artwork coordinates remain unchanged.
"""

from __future__ import annotations

import re
from pathlib import Path

SVG_ROOT_RE = re.compile(r"<svg\b.*?>", re.DOTALL)
WIDTH_RE = re.compile(r'\bwidth="([0-9]+(?:\.[0-9]+)?)(?:px)?"')
HEIGHT_RE = re.compile(r'\bheight="([0-9]+(?:\.[0-9]+)?)(?:px)?"')


def repair(path: Path) -> bool:
    text = path.read_text(encoding="utf-8")
    root_match = SVG_ROOT_RE.search(text)
    if root_match is None:
        raise RuntimeError(f"No SVG root in {path}")

    root = root_match.group(0)
    width_match = WIDTH_RE.search(root)
    height_match = HEIGHT_RE.search(root)
    if width_match is None or height_match is None:
        raise RuntimeError(f"Missing numeric root dimensions in {path}")

    width_value = float(width_match.group(1))
    height_value = float(height_match.group(1))

    # Existing panels use the Rack coordinate convention: 15 units/HP,
    # 380 units high. Convert only the physical document dimensions.
    width_mm = width_value / 15.0 * 5.08
    if abs(height_value - 380.0) > 0.01:
        raise RuntimeError(f"Unexpected panel height {height_value} in {path}")

    new_root = WIDTH_RE.sub(f'width="{width_mm:.2f}mm"', root, count=1)
    new_root = HEIGHT_RE.sub('height="128.5mm"', new_root, count=1)
    new_text = text[: root_match.start()] + new_root + text[root_match.end() :]

    if new_text == text:
        return False

    path.write_text(new_text, encoding="utf-8")
    return True


def main() -> None:
    changed: list[Path] = []
    for path in sorted(Path("res").glob("*.svg")):
        if repair(path):
            changed.append(path)

    print(f"Updated {len(changed)} SVG panel(s).")
    for path in changed:
        print(path)


if __name__ == "__main__":
    main()
