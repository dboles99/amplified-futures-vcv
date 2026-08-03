#!/usr/bin/env python3
"""
fix_overlap.py — lift captions out from under the widgets drawn over them.

The July 2026 submission was lost to this: ten of fourteen panels put label
paths at the same y as the widget centre, so Rack drew each knob and port over
its own caption. The repair pass fixed the panels it touched. Eight panels
still have it.

The artwork itself is correct — it is simply at the wrong height. So each
offending caption is translated up until it clears the widget's top edge by the
same 1.2 mm the spec requires of new panels. No bezier is redrawn.

Widget radii come from Rack's own component SVGs via check_overlap.

Usage:
    python tools/fix_overlap.py --report
    python tools/fix_overlap.py --apply
    python tools/fix_overlap.py --apply --module Pulse

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import argparse
import re
import shutil
import sys
from pathlib import Path

sys.path.insert(0, str(Path(__file__).resolve().parent))
from check_corners import (path_bbox, transform_offset,  # noqa: E402
                           BAND_W_FRAC)

TRANSFORM_RE = re.compile(r'\btransform="([^"]*)"')
from check_overlap import (widgets, find_source, radius_mm, disc_hits_box,  # noqa: E402
                           PX_PER_MM, MIN_LABEL_W_MM, MIN_LABEL_H_MM,
                           MAX_LABEL_H_MM, COVERED_MIN)

ROOT = Path(__file__).resolve().parent.parent
RES = ROOT / "res"

LABEL_GAP_MM = 1.2        # spec §4: baseline clears the widget top edge
PATH_EL_RE = re.compile(r"<path\b[^>]*?/>|<path\b[^>]*?>.*?</path>", re.S)
D_RE = re.compile(r'\bd="([^"]*)"')


def plan(slug: str):
    svg = RES / f"{slug}.svg"
    cpp = find_source(slug)
    if not svg.exists() or cpp is None:
        return None, []

    text = svg.read_text(encoding="utf-8", errors="replace")
    vb = re.search(r'viewBox="([^"]*)"', text)
    if not vb:
        return text, []
    width_px = float(vb.group(1).split()[2])

    ws, _ = widgets(cpp)
    if not ws:
        return text, []

    moves = []
    for match in PATH_EL_RE.finditer(text):
        el = match.group(0)
        dm = D_RE.search(el)
        if not dm:
            continue
        bb = path_bbox(dm.group(1))
        if not bb:
            continue
        tdx, tdy = transform_offset(
            (TRANSFORM_RE.search(el).group(1) if TRANSFORM_RE.search(el) else ""))
        bb = (bb[0] + tdx, bb[1] + tdy, bb[2] + tdx, bb[3] + tdy)

        if bb[2] - bb[0] >= width_px * BAND_W_FRAC:
            continue
        box = tuple(v / PX_PER_MM for v in bb)
        w, h = box[2] - box[0], box[3] - box[1]
        if w < MIN_LABEL_W_MM or h < MIN_LABEL_H_MM or h > MAX_LABEL_H_MM:
            continue

        def coverage_at(offset_mm):
            """Worst coverage of this caption by any widget, shifted by offset,
            plus the widget responsible."""
            shifted = (box[0], box[1] + offset_mm, box[2], box[3] + offset_mm)
            sw = shifted[2] - shifted[0]
            sh = shifted[3] - shifted[1]
            worst_cov, culprit = 0.0, None
            for kind, x, y, name in ws:
                r = radius_mm(kind)
                bcx = (shifted[0] + shifted[2]) / 2
                bcy = (shifted[1] + shifted[3]) / 2
                if (abs(bcx - x) < 0.5 and abs(bcy - y) < 0.5
                        and sw <= r * 2.4 and sh <= r * 2.4
                        and 0.5 <= sw / max(1e-6, sh) <= 3.0):
                    continue               # concentric decoration, not text
                if not disc_hits_box(x, y, r, shifted):
                    continue
                ox = max(0.0, min(shifted[2], x + r) - max(shifted[0], x - r))
                oy = max(0.0, min(shifted[3], y + r) - max(shifted[1], y - r))
                cov = (ox * oy) / max(1e-6, sw * sh)
                if cov > worst_cov:
                    worst_cov, culprit = cov, (kind, name, cov)
            return worst_cov, culprit

        cov0, culprit0 = coverage_at(0.0)
        if cov0 < COVERED_MIN:
            continue

        # Stacked widgets — Pulse puts a trimpot, a jack and another trimpot in
        # 20mm — mean the first clear gap may be either side. Search outward
        # from zero and take the smallest move that clears everything.
        best = None
        step = 0.2
        for i in range(1, int(16.0 / step) + 1):
            for cand in (-i * step, i * step):
                cov, _ = coverage_at(cand)
                if cov < COVERED_MIN:
                    best = cand
                    break
            if best is not None:
                break

        if best is None:
            moves.append((match.span(), None, box,
                          (0.0, culprit0[0], culprit0[1], cov0)))
            continue
        moves.append((match.span(), best * PX_PER_MM, box,
                      (best, culprit0[0], culprit0[1], cov0)))

    return text, moves


def run(slug: str, dry: bool) -> int:
    text, moves = plan(slug)
    if text is None or not moves:
        return 0

    print(f"\n  {slug}")
    for _, dy_px, box, (_, kind, name, covered) in moves:
        if dy_px is None:
            print(f"      NO CLEAR GAP  caption at "
                  f"({box[0]:.1f},{box[1]:.1f}) is {int(covered * 100)}% under "
                  f"{kind} {name} and every nearby position is also covered "
                  f"- needs a layout change, not a nudge")
            continue
        print(f"      move {dy_px:6.1f}px  caption at "
              f"({box[0]:.1f},{box[1]:.1f})  was {int(covered * 100)}% under "
              f"{kind} {name}")
    if dry:
        return len(moves)

    svg = RES / f"{slug}.svg"
    if not svg.with_suffix(".svg.bak").exists():
        shutil.copy2(svg, svg.with_suffix(".svg.bak"))

    out = text
    for (start, end), dy_px, _, _ in sorted(moves, key=lambda m: -m[0][0]):
        if dy_px is None:
            continue
        el = out[start:end]
        tm = TRANSFORM_RE.search(el)
        if tm:
            el = (el[:tm.start(1)] + f"translate(0,{dy_px:.3f}) "
                  + tm.group(1) + el[tm.end(1):])
        else:
            tag_end = el.index(" ") if " " in el[:80] else 5
            el = (el[:tag_end] + f' transform="translate(0,{dy_px:.3f})"'
                  + el[tag_end:])
        out = out[:start] + el + out[end:]

    svg.write_text(out, encoding="utf-8")
    print("      written")
    return len(moves)


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--module", action="append")
    ap.add_argument("--apply", action="store_true")
    ap.add_argument("--report", action="store_true")
    args = ap.parse_args()
    if not (args.apply or args.report):
        ap.error("pass --report or --apply")

    slugs = args.module or sorted(p.stem for p in RES.glob("*.svg"))
    total = sum(run(s, dry=not args.apply) for s in slugs)
    print(f"\n{total} caption(s) "
          f"{'lifted' if args.apply else 'would be lifted'}.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
