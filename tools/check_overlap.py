#!/usr/bin/env python3
"""
check_overlap.py — find labels that a widget is drawn on top of.

This asks a different question from the checker that was abandoned. That one
asked "is this label correctly positioned above its widget", which needs to
know which label belongs to which widget, and got it wrong constantly. This
one asks "is any label underneath any widget", which needs no pairing at all
and has no judgement in it. A knob drawn over a caption is a defect however
you look at it.

That is the exact failure that lost the July submission: DENSITY rendering as
Y, PRESSURE as RE, SUM vanishing entirely.

Widgets are read from the source, labels from the panel. Modules that place
widgets with variables rather than literals cannot be read this way and are
reported as unreadable rather than silently passed — see SwarmCore.

Usage:
    python tools/check_overlap.py
    python tools/check_overlap.py DroneClone --verbose

Exit 0 = no label sits under a widget on any readable panel.

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path
from xml.etree import ElementTree as ET

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_corners import (path_bbox, rect_bbox, round_bbox,  # noqa: E402
                           transform_offset, BAND_W_FRAC)

ROOT = Path(__file__).resolve().parent.parent
SRC, RES = ROOT / "src", ROOT / "res"

PX_PER_MM = 75.0 / 25.4

# Widget radii come from Rack's own component SVGs, not from estimates. A first
# pass guessed them and was wrong by up to 50% — Trimpot is 3.02mm, not the
# 4.5mm assumed — which manufactured dozens of overlaps that do not exist.
COMPONENT_DIRS = [
    Path(r"C:\Program Files\VCV\Rack2Pro\res\ComponentLibrary"),
    Path(r"C:\Program Files\VCV\Rack2Free\res\ComponentLibrary"),
]

# Widget class name -> the SVG file Rack draws for it.
COMPONENT_SVG = {
    "Trimpot": "Trimpot", "RoundBlackKnob": "RoundBlackKnob",
    "RoundSmallBlackKnob": "RoundSmallBlackKnob",
    "RoundLargeBlackKnob": "RoundLargeBlackKnob",
    "RoundBigBlackKnob": "RoundBigBlackKnob",
    "RoundHugeBlackKnob": "RoundHugeBlackKnob",
    "Davies1900hBlackKnob": "Davies1900hBlack",
    "PJ301MPort": "PJ301M", "PJ3410Port": "PJ3410", "CL1362Port": "CL1362",
    "TL1105": "TL1105_0", "CKSS": "CKSS_0", "CKSSThree": "CKSSThree_0",
    "LEDBezel": "LEDBezel", "VCVButton": "VCVButton_0",
    "VCVSlider": "VCVSlider",
}

# Fallbacks in mm for anything the installed Rack does not provide.
FALLBACK_MM = {
    "Trimpot": 3.02, "RoundSmallBlackKnob": 3.84, "RoundBlackKnob": 4.80,
    "RoundLargeBlackKnob": 6.10, "RoundBigBlackKnob": 7.62,
    "PJ301MPort": 4.01, "TL1105": 2.5, "CKSS": 2.5, "CKSSThree": 3.0,
}
DEFAULT_RADIUS_MM = 4.0

# Fraction of a caption that must be hidden before it counts as a defect.
# Below this the label reads fine on a render; the renders were used to set it.
COVERED_MIN = 0.12

_radius_cache: dict = {}


def radius_mm(kind: str) -> float:
    if kind in _radius_cache:
        return _radius_cache[kind]

    name = COMPONENT_SVG.get(kind)
    if name:
        for d in COMPONENT_DIRS:
            f = d / f"{name}.svg"
            if not f.exists():
                continue
            head = f.read_text(encoding="utf-8", errors="replace")[:2000]
            m = re.search(r'width="([\d.]+)(?:px)?"', head)
            if m:
                r = float(m.group(1)) / 2.0 / PX_PER_MM
                _radius_cache[kind] = r
                return r

    r = FALLBACK_MM.get(kind, DEFAULT_RADIUS_MM)
    _radius_cache[kind] = r
    return r

# Ink smaller than this is a tick mark or a rule, not a caption.
MIN_LABEL_W_MM = 1.2
MIN_LABEL_H_MM = 0.6
MAX_LABEL_H_MM = 6.0

WIDGET_RE = re.compile(
    r"create(?:Param|Input|Output|Light)?Centered\s*<\s*([A-Za-z0-9_]+)"
    r"(?:\s*<[^>]*>)?\s*>\s*\(\s*mm2px\(\s*Vec\(\s*([^,]+?)\s*,\s*([^)]+?)\s*\)\s*\)")
ENUM_RE = re.compile(r"::([A-Z][A-Z0-9_]*)\s*\)")


def literal(tok: str):
    tok = tok.strip().rstrip("f")
    try:
        return float(tok)
    except ValueError:
        return None


def widgets(cpp: Path):
    """(kind, x_mm, y_mm, name) plus a count of positions we could not read."""
    text = cpp.read_text(encoding="utf-8", errors="replace")
    out, unreadable = [], 0
    for line in text.splitlines():
        stripped = line.strip()
        if stripped.startswith("//"):
            continue
        m = WIDGET_RE.search(line)
        if not m:
            continue
        kind = m.group(1)
        if kind.startswith("Screw"):
            continue
        x, y = literal(m.group(2)), literal(m.group(3))
        if x is None or y is None:
            unreadable += 1
            continue
        nm = ENUM_RE.search(line)
        out.append((kind, x, y, nm.group(1) if nm else "?"))
    return out, unreadable


def labels(svg: Path):
    """Caption-sized ink, as (x0, y0, x1, y1) in mm."""
    try:
        root = ET.fromstring(svg.read_text(encoding="utf-8", errors="replace"))
    except ET.ParseError:
        return []
    parts = root.get("viewBox", "0 0 0 380").split()
    width = float(parts[2])

    found = []
    stack = [(root, 0.0, 0.0)]
    while stack:
        el, pdx, pdy = stack.pop()
        edx, edy = transform_offset(el.get("transform", ""))
        dx, dy = pdx + edx, pdy + edy
        for child in list(el):
            stack.append((child, dx, dy))

        tag = el.tag.rsplit("}", 1)[-1]
        if tag == "path":
            bb = path_bbox(el.get("d", "") or "")
        elif tag == "rect":
            bb = rect_bbox(el)
        elif tag in ("circle", "ellipse"):
            continue          # panel decoration, not a caption
        else:
            continue
        if not bb:
            continue
        bb = (bb[0] + dx, bb[1] + dy, bb[2] + dx, bb[3] + dy)
        w = (bb[2] - bb[0]) / PX_PER_MM
        h = (bb[3] - bb[1]) / PX_PER_MM
        if bb[2] - bb[0] >= width * BAND_W_FRAC:
            continue
        if w < MIN_LABEL_W_MM or h < MIN_LABEL_H_MM or h > MAX_LABEL_H_MM:
            continue
        found.append(tuple(v / PX_PER_MM for v in bb))
    return found


def find_source(slug: str):
    p = SRC / f"{slug}.cpp"
    if p.exists():
        return p
    for q in SRC.glob("*.cpp"):
        if q.stem.lower().startswith(slug.lower()):
            return q
    return None


def disc_hits_box(cx, cy, r, box):
    """Closest point on the box to the disc centre, then compare distance."""
    nx = max(box[0], min(cx, box[2]))
    ny = max(box[1], min(cy, box[3]))
    return (nx - cx) ** 2 + (ny - cy) ** 2 < r * r


def audit(slug: str, verbose: bool):
    svg, cpp = RES / f"{slug}.svg", find_source(slug)
    if not svg.exists() or cpp is None:
        return [f"no panel or source for {slug}"], 0

    ws, unreadable = widgets(cpp)
    ls = labels(svg)
    problems = []

    if unreadable:
        problems.append(
            f"NOTE  {unreadable} widget position(s) use variables, not "
            f"literals — not checkable here, inspect the render")

    if not ws:
        return problems, 0

    hits = 0
    for kind, x, y, name in ws:
        r = radius_mm(kind)
        for box in ls:
            # Panels draw a socket or knob face in the artwork beneath the real
            # widget. That ink is concentric with the widget and is meant to be
            # covered — it is decoration, not a caption.
            bcx, bcy = (box[0] + box[2]) / 2, (box[1] + box[3]) / 2
            bw, bh = box[2] - box[0], box[3] - box[1]
            aspect = bw / max(1e-6, bh)
            if (abs(bcx - x) < 0.5 and abs(bcy - y) < 0.5
                    and bw <= r * 2.4 and bh <= r * 2.4
                    and 0.5 <= aspect <= 3.0):
                continue
            if not disc_hits_box(x, y, r, box):
                continue

            # How much of the caption is actually hidden. A label grazing a
            # rim by a tenth of a millimetre reads fine on the render; one with
            # a third of its height under a knob is the July failure.
            ox = max(0.0, min(box[2], x + r) - max(box[0], x - r))
            oy = max(0.0, min(box[3], y + r) - max(box[1], y - r))
            area = max(1e-6, (box[2] - box[0]) * (box[3] - box[1]))
            covered = (ox * oy) / area

            if covered < COVERED_MIN:
                continue

            hits += 1
            severity = "SEVERE" if covered >= 0.35 else "OVERLAP"
            problems.append(
                (covered,
                 f"{severity:8} {int(covered * 100):3d}% of a caption hidden by "
                 f"{kind} {name} at ({x}, {y})  ink "
                 f"({box[0]:.1f},{box[1]:.1f})-({box[2]:.1f},{box[3]:.1f})"))
            break

    # worst first
    ranked = [p for p in problems if isinstance(p, tuple)]
    notes = [p for p in problems if not isinstance(p, tuple)]
    ranked.sort(key=lambda t: -t[0])
    return notes + [t[1] for t in ranked], hits


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("-")]
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    slugs = args or sorted(p.stem for p in RES.glob("*.svg"))

    total_hits = 0
    dirty = 0
    for slug in slugs:
        problems, hits = audit(slug, verbose)
        total_hits += hits
        if problems:
            if hits:
                dirty += 1
            print(f"\n  {slug}")
            for p in problems:
                print(f"      {p}")

    print(f"\n{total_hits} overlap(s) on {dirty} of {len(slugs)} panel(s).\n")
    return 1 if total_hits else 0


if __name__ == "__main__":
    sys.exit(main())
