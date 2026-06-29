#!/usr/bin/env python3
"""Convert VCV Rack panel SVG root dimensions from px-like units to mm.

VCV's panel guide requires millimetres for the SVG document dimensions.
The viewBox and all artwork coordinates remain unchanged.
"""

from __future__ import annotations

import re
from pathlib import Path

SVG_ROOT_RE = re.compile(r"<svg\b.*?>", re.DOTALL)
WIDTH_RE = re.compile(r"""\bwidth\s*=\s*(["'])([0-9]+(?:\.[0-9]+)?)(px|mm)?\1""")
HEIGHT_RE = re.compile(r"""\bheight\s*=\s*(["'])([0-9]+(?:\.[0-9]+)?)(px|mm)?\1""")


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

    width_quote = width_match.group(1)
    width_value = float(width_match.group(2))
    width_unit = width_match.group(3)
    height_quote = height_match.group(1)
    height_value = float(height_match.group(2))
    height_unit = height_match.group(3)

    if width_unit == "mm" and height_unit == "mm":
        return False
    if width_unit == "mm" or height_unit == "mm":
        raise RuntimeError(f"Mixed root dimension units in {path}")

    # Existing panels use the Rack coordinate convention: 15 units/HP,
    # 380 units high. Convert only the physical document dimensions.
    width_mm = width_value / 15.0 * 5.08
    if abs(height_value - 380.0) > 0.01:
        raise RuntimeError(f"Unexpected panel height {height_value} in {path}")

    new_root = WIDTH_RE.sub(f"width={width_quote}{width_mm:.2f}mm{width_quote}", root, count=1)
    new_root = HEIGHT_RE.sub(f"height={height_quote}128.5mm{height_quote}", new_root, count=1)
    new_text = text[: root_match.start()] + new_root + text[root_match.end() :]

    if new_text == text:
        return False

    path.write_text(new_text, encoding="utf-8")
    return True


def main() -> None:
    repo_root = Path(__file__).resolve().parents[1]
    res_dir = repo_root / "res"
    if not res_dir.is_dir():
        raise RuntimeError(f"Missing panel directory: {res_dir}")

    changed: list[Path] = []
    for path in sorted(res_dir.glob("*.svg")):
        if repair(path):
            changed.append(path)

    print(f"Updated {len(changed)} SVG panel(s).")
    for path in changed:
        print(path.relative_to(repo_root))


if __name__ == "__main__":
    main()
