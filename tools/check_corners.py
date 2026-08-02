#!/usr/bin/env python3
"""
check_corners.py — no artwork may sit under a mounting screw.

Rack places screws at fixed positions and draws them *over* the panel. Anything
in those four squares is covered at runtime, which is why DRONECORE loses its
leading D and trailing E, and why AMPL. FUTURES reads AM_ FUTURES on six other
panels.

Unlike label placement, this is not a judgement call — it is four rectangles
and a bounding-box test. That is why this check is worth having and the label
one was not.

Screw geometry, from the widget constructors used across the set:

    top-left      (RACK_GRID_WIDTH, 0)
    top-right     (box.size.x - 2*RACK_GRID_WIDTH, 0)
    bottom-left   (RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)
    bottom-right  (box.size.x - 2*RACK_GRID_WIDTH, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)

RACK_GRID_WIDTH is 15 px, RACK_GRID_HEIGHT is 380 px, and a screw is 15 px
across, positioned by its top-left corner.

Usage:
    python tools/check_corners.py                # all panels
    python tools/check_corners.py DroneCore
    python tools/check_corners.py --verbose      # list every offending path

Exit 0 = all four corners clear on every panel.

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path
from xml.etree import ElementTree as ET

ROOT = Path(__file__).resolve().parent.parent
RES = ROOT / "res"

GRID = 15.0            # RACK_GRID_WIDTH
HEIGHT = 380.0         # RACK_GRID_HEIGHT
SCREW = 15.0           # screw sprite is one grid unit square

# A little slack: ink that merely grazes the screw edge is not what breaks a
# panel, and demanding zero contact would flag hairline background rectangles.
MARGIN = 1.0

NUM_RE = re.compile(r"-?\d*\.?\d+(?:[eE][+-]?\d+)?")
CMD_RE = re.compile(r"([MmLlHhVvCcSsQqTtAaZz])([^MmLlHhVvCcSsQqTtAaZz]*)")

# Panel-spanning bands legitimately run under the screws: the orange header bar
# and the footer rule are meant to pass behind them, and every panel in the set
# does this deliberately. Only ink narrower than the panel is content that can
# be lost. Without this the check reports 19 of 19 and says nothing.
BAND_W_FRAC = 0.95


def path_bbox(d: str):
    x = y = start_x = start_y = 0.0
    xs: list[float] = []
    ys: list[float] = []
    ax = ay = 0.0

    for cmd, body in CMD_RE.findall(d):
        vals = [float(v) for v in NUM_RE.findall(body)]
        rel = cmd.islower()
        c = cmd.upper()

        if c == "Z":
            x, y = start_x, start_y
            continue
        if c in ("H", "V"):
            for v in vals:
                if c == "H":
                    x = x + v if rel else v
                else:
                    y = y + v if rel else v
                xs.append(x); ys.append(y)
            continue

        stride = {"M": 2, "L": 2, "T": 2, "S": 4, "Q": 4, "C": 6, "A": 7}.get(c, 2)
        if stride > len(vals):
            continue
        for i in range(0, len(vals) - stride + 1, stride):
            chunk = vals[i:i + stride]
            if c == "A":
                ax = x + chunk[5] if rel else chunk[5]
                ay = y + chunk[6] if rel else chunk[6]
                xs.append(ax); ys.append(ay)
            else:
                for j in range(0, stride, 2):
                    ax = x + chunk[j] if rel else chunk[j]
                    ay = y + chunk[j + 1] if rel else chunk[j + 1]
                    xs.append(ax); ys.append(ay)
            x, y = ax, ay
            if c == "M" and i == 0:
                start_x, start_y = x, y

    if not xs:
        return None
    return (min(xs), min(ys), max(xs), max(ys))


TRANSLATE_RE = re.compile(r"translate\(\s*(-?[\d.eE+]+)\s*[, ]\s*(-?[\d.eE+]+)?\s*\)")
MATRIX_RE = re.compile(r"matrix\(([^)]*)\)")


def transform_offset(spec: str):
    """Accumulated (dx, dy) from a transform attribute.

    Only translation matters here: nothing in these panels rotates or scales a
    label. A checker that ignores transforms reports a fix as still broken,
    which is worse than not checking at all.
    """
    dx = dy = 0.0
    if not spec:
        return dx, dy
    for m in TRANSLATE_RE.finditer(spec):
        dx += float(m.group(1))
        dy += float(m.group(2) or 0.0)
    for m in MATRIX_RE.finditer(spec):
        vals = [float(v) for v in re.split(r"[,\s]+", m.group(1).strip()) if v]
        if len(vals) == 6:
            dx += vals[4]
            dy += vals[5]
    return dx, dy


def rect_bbox(el):
    try:
        x = float(el.get("x", 0)); y = float(el.get("y", 0))
        return (x, y, x + float(el.get("width", 0)), y + float(el.get("height", 0)))
    except (TypeError, ValueError):
        return None


def round_bbox(el):
    """Counter-holes ship as <circle>. Skipping them made the check blind to a
    defect visible in every render: a dark dot beside each screw."""
    try:
        cx = float(el.get("cx", 0)); cy = float(el.get("cy", 0))
        rx = float(el.get("rx") or el.get("r") or 0)
        ry = float(el.get("ry") or el.get("r") or 0)
        if rx <= 0 or ry <= 0:
            return None
        return (cx - rx, cy - ry, cx + rx, cy + ry)
    except (TypeError, ValueError):
        return None


def screw_zones(width: float):
    return {
        "top-left":     (GRID, 0.0, GRID + SCREW, SCREW),
        "top-right":    (width - 2 * GRID, 0.0, width - 2 * GRID + SCREW, SCREW),
        "bottom-left":  (GRID, HEIGHT - GRID, GRID + SCREW, HEIGHT),
        "bottom-right": (width - 2 * GRID, HEIGHT - GRID,
                         width - 2 * GRID + SCREW, HEIGHT),
    }


def overlaps(a, b, margin=MARGIN):
    return not (a[2] <= b[0] + margin or a[0] >= b[2] - margin
                or a[3] <= b[1] + margin or a[1] >= b[3] - margin)


def audit(slug: str, verbose: bool):
    svg = RES / f"{slug}.svg"
    if not svg.exists():
        return [f"{slug}: no panel"]

    try:
        root = ET.fromstring(svg.read_text(encoding="utf-8", errors="replace"))
    except ET.ParseError as e:
        return [f"{slug}: SVG does not parse — {e}"]

    parts = root.get("viewBox", "0 0 0 380").split()
    width, height = float(parts[2]), float(parts[3])
    zones = screw_zones(width)

    hits: dict[str, list] = {k: [] for k in zones}

    # ancestor transforms accumulate, so walk with the inherited offset
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
        elif tag in ("rect", "image"):
            bb = rect_bbox(el)
        elif tag in ("circle", "ellipse"):
            bb = round_bbox(el)
        else:
            continue
        if not bb:
            continue
        bb = (bb[0] + dx, bb[1] + dy, bb[2] + dx, bb[3] + dy)

        w, h = bb[2] - bb[0], bb[3] - bb[1]
        if w <= 0 or h <= 0:
            continue
        if w >= width * BAND_W_FRAC:
            continue          # header bar, footer rule, background — by design

        for name, zone in zones.items():
            if not overlaps(bb, zone):
                continue
            # A counter-hole is drawn concentric with the screw and smaller
            # than it, so the screw covering it is the whole point. CollapseSat
            # and SitarGrid both do this deliberately.
            zcx, zcy = (zone[0] + zone[2]) / 2, (zone[1] + zone[3]) / 2
            bcx, bcy = (bb[0] + bb[2]) / 2, (bb[1] + bb[3]) / 2
            if (abs(bcx - zcx) < 2.0 and abs(bcy - zcy) < 2.0
                    and w <= SCREW and h <= SCREW):
                continue
            hits[name].append((tag, bb, w, h))

    problems = []
    for name, found in hits.items():
        if not found:
            continue
        problems.append(
            f"{name:13} {len(found)} element(s) under the screw")
        if verbose:
            for tag, bb, w, h in found[:6]:
                problems.append(
                    f"                  <{tag}> "
                    f"({bb[0]:.1f},{bb[1]:.1f})-({bb[2]:.1f},{bb[3]:.1f}) "
                    f"{w:.1f}x{h:.1f}px")
    return problems


def main() -> int:
    args = [a for a in sys.argv[1:] if not a.startswith("-")]
    verbose = "--verbose" in sys.argv or "-v" in sys.argv
    slugs = args or sorted(p.stem for p in RES.glob("*.svg"))

    dirty = 0
    for slug in slugs:
        problems = audit(slug, verbose)
        if problems:
            dirty += 1
            print(f"\n  {slug}")
            for p in problems:
                print(f"      {p}")

    print(f"\n{dirty} of {len(slugs)} panel(s) have artwork under a screw.\n")
    return 1 if dirty else 0


if __name__ == "__main__":
    sys.exit(main())
