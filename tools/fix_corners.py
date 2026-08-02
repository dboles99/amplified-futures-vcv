#!/usr/bin/env python3
"""
fix_corners.py — lift footer artwork clear of the bottom mounting screws.

The `AMPL. FUTURES` footer and the serial number sit at y 367-375 on several
panels. The bottom screws occupy y 365-380 and are drawn over the panel, so
those captions lose their first and last letters at runtime.

The fix is a translate, not a redraw: the artwork is already correct, it is
simply 10 px too low. Wrapping the offending path in a transform preserves the
bezier outlines exactly.

Titles in the header band cannot be fixed this way — there is nowhere above
them to go. Those are handled separately; see --report.

Usage:
    python tools/fix_corners.py --report      # what would change, change nothing
    python tools/fix_corners.py --apply
    python tools/fix_corners.py --apply --module Drift

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_corners import (path_bbox, screw_zones, overlaps, SCREW,  # noqa: E402
                           GRID, HEIGHT, BAND_W_FRAC)

ROOT = Path(__file__).resolve().parent.parent
RES = ROOT / "res"

# Clearance we want between the top of the screw and the bottom of the artwork.
CLEAR_PX = 2.0

PATH_EL_RE = re.compile(r"<path\b[^>]*?/>|<path\b[^>]*?>.*?</path>", re.S)
D_RE = re.compile(r'\bd="([^"]*)"')
TRANSFORM_RE = re.compile(r'\btransform="([^"]*)"')


def plan(slug: str):
    """Return (svg_text, [(match_span, dy, bbox)]) for footer paths to lift."""
    svg = RES / f"{slug}.svg"
    if not svg.exists():
        return None, []
    text = svg.read_text(encoding="utf-8", errors="replace")

    m = re.search(r'viewBox="([^"]*)"', text)
    if not m:
        return text, []
    parts = m.group(1).split()
    width = float(parts[2])
    zones = screw_zones(width)
    bottom = {k: v for k, v in zones.items() if k.startswith("bottom")}

    moves = []
    for match in PATH_EL_RE.finditer(text):
        el = match.group(0)
        dm = D_RE.search(el)
        if not dm:
            continue
        bb = path_bbox(dm.group(1))
        if not bb:
            continue
        w = bb[2] - bb[0]
        if w >= width * BAND_W_FRAC:
            continue                       # panel-spanning band, by design

        hit = any(overlaps(bb, z) for z in bottom.values())
        if not hit:
            continue

        # counter-holes are concentric with a screw and stay put
        skip = False
        for z in bottom.values():
            zcx, zcy = (z[0] + z[2]) / 2, (z[1] + z[3]) / 2
            bcx, bcy = (bb[0] + bb[2]) / 2, (bb[1] + bb[3]) / 2
            if (abs(bcx - zcx) < 2.0 and abs(bcy - zcy) < 2.0
                    and w <= SCREW and (bb[3] - bb[1]) <= SCREW):
                skip = True
        if skip:
            continue

        screw_top = HEIGHT - GRID
        dy = -(bb[3] - (screw_top - CLEAR_PX))
        if dy >= 0:
            continue
        moves.append((match.span(), dy, bb))

    return text, moves


def apply(slug: str, dry: bool) -> int:
    text, moves = plan(slug)
    if text is None:
        print(f"  {slug}: no panel")
        return 0
    if not moves:
        return 0

    print(f"\n  {slug}")
    for (start, end), dy, bb in moves:
        print(f"      lift {dy:+.1f}px  path at "
              f"({bb[0]:.1f},{bb[1]:.1f})-({bb[2]:.1f},{bb[3]:.1f})")

    if dry:
        return len(moves)

    svg = RES / f"{slug}.svg"
    shutil.copy2(svg, svg.with_suffix(".svg.bak"))

    # apply from the end so earlier spans stay valid
    out = text
    for (start, end), dy, _ in sorted(moves, key=lambda m: -m[0][0]):
        el = out[start:end]
        tm = TRANSFORM_RE.search(el)
        if tm:
            new_el = el[:tm.start(1)] + f"translate(0,{dy:.3f}) " + \
                     tm.group(1) + el[tm.end(1):]
        else:
            insert_at = el.index(">") if el.startswith("<path") else 5
            # place the attribute just inside the opening tag
            tag_end = el.index(" ") if " " in el[:80] else 5
            new_el = el[:tag_end] + f' transform="translate(0,{dy:.3f})"' + \
                     el[tag_end:]
        out = out[:start] + new_el + out[end:]

    svg.write_text(out, encoding="utf-8")
    print(f"      written (backup at {svg.name}.bak)")
    return len(moves)


CIRCLE_EL_RE = re.compile(r"<circle\b[^>]*?/>")
CX_RE = re.compile(r'\bcx="([-\d.eE+]+)"')
CY_RE = re.compile(r'\bcy="([-\d.eE+]+)"')
R_RE = re.compile(r'\br="([-\d.eE+]+)"')


def snap_holes(slug: str, dry: bool) -> int:
    """Move counter-holes onto the screw centres.

    Every affected panel drew its holes at the screw's *position* — the
    top-left corner passed to createWidget — instead of its centre. A screw is
    15px across, so the holes sit 7.5px outboard and peek out from under the
    screw as a dark dot.
    """
    svg = RES / f"{slug}.svg"
    if not svg.exists():
        return 0
    text = svg.read_text(encoding="utf-8", errors="replace")

    m = re.search(r'viewBox="([^"]*)"', text)
    if not m:
        return 0
    width = float(m.group(1).split()[2])
    centres = [((z[0] + z[2]) / 2, (z[1] + z[3]) / 2)
               for z in screw_zones(width).values()]

    moves = []
    for match in CIRCLE_EL_RE.finditer(text):
        el = match.group(0)
        cxm, cym, rm = CX_RE.search(el), CY_RE.search(el), R_RE.search(el)
        if not (cxm and cym and rm):
            continue
        cx, cy, r = float(cxm.group(1)), float(cym.group(1)), float(rm.group(1))
        if r > SCREW / 2:
            continue
        near = min(centres, key=lambda c: (c[0] - cx) ** 2 + (c[1] - cy) ** 2)
        d2 = (near[0] - cx) ** 2 + (near[1] - cy) ** 2
        if d2 < 0.25 or d2 > (SCREW * 1.5) ** 2:
            continue          # already aligned, or not a counter-hole
        moves.append((match.span(), cx, cy, near))

    # Panels whose holes have already been through object-to-path store them as
    # <path>, where there is no cx to edit — those get a translate instead.
    path_moves = []
    for match in PATH_EL_RE.finditer(text):
        el = match.group(0)
        dm = D_RE.search(el)
        if not dm:
            continue
        bb = path_bbox(dm.group(1))
        if not bb:
            continue
        w, h = bb[2] - bb[0], bb[3] - bb[1]
        if w <= 0 or h <= 0 or w > SCREW or h > SCREW:
            continue
        cx, cy = (bb[0] + bb[2]) / 2, (bb[1] + bb[3]) / 2
        near = min(centres, key=lambda c: (c[0] - cx) ** 2 + (c[1] - cy) ** 2)
        d2 = (near[0] - cx) ** 2 + (near[1] - cy) ** 2
        if d2 < 0.25 or d2 > (SCREW * 1.5) ** 2:
            continue
        path_moves.append((match.span(), cx, cy, near))

    if not moves and not path_moves:
        return 0

    print(f"\n  {slug}")
    for _, cx, cy, near in moves + path_moves:
        print(f"      snap hole ({cx:.1f},{cy:.1f}) -> "
              f"({near[0]:.1f},{near[1]:.1f})")
    if dry:
        return len(moves) + len(path_moves)

    if not svg.with_suffix(".svg.bak").exists():
        shutil.copy2(svg, svg.with_suffix(".svg.bak"))

    out = text
    combined = ([(s, cx, cy, n, "circle") for s, cx, cy, n in moves]
                + [(s, cx, cy, n, "path") for s, cx, cy, n in path_moves])
    for (start, end), cx, cy, near, kind in sorted(combined,
                                                   key=lambda m: -m[0][0]):
        el = out[start:end]
        if kind == "circle":
            el = CX_RE.sub(f'cx="{near[0]:.3f}"', el, count=1)
            el = CY_RE.sub(f'cy="{near[1]:.3f}"', el, count=1)
        else:
            dx, dy = near[0] - cx, near[1] - cy
            tm = TRANSFORM_RE.search(el)
            if tm:
                el = (el[:tm.start(1)] + f"translate({dx:.3f},{dy:.3f}) "
                      + tm.group(1) + el[tm.end(1):])
            else:
                tag_end = el.index(" ") if " " in el[:80] else 5
                el = (el[:tag_end]
                      + f' transform="translate({dx:.3f},{dy:.3f})"'
                      + el[tag_end:])
        out = out[:start] + el + out[end:]
    svg.write_text(out, encoding="utf-8")
    print("      written")
    return len(combined)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--module", action="append")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--report", action="store_true")
    ap.add_argument("--holes", action="store_true",
                    help="snap counter-holes onto the screw centres")
    args = ap.parse_args()

    if not (args.apply or args.report):
        ap.error("pass --report or --apply")

    slugs = args.module or sorted(p.stem for p in RES.glob("*.svg"))

    if args.holes:
        total = sum(snap_holes(s, dry=not args.apply) for s in slugs)
        print(f"\n{total} hole(s) "
              f"{'snapped' if args.apply else 'would be snapped'}.\n")
        return 0

    total = sum(apply(s, dry=not args.apply) for s in slugs)
    print(f"\n{total} path(s) {'lifted' if args.apply else 'would be lifted'}.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
