#!/usr/bin/env python3
#
# NOT USED BY THE SHIPPED PANELS.
#
# This is infrastructure for a commiecore three-zone redesign that was piloted
# on Drift and reverted on 2026-08-03: at 16 HP it came out sparser and lower
# contrast than the 12 HP panel it replaced, and introduced a label collision.
# See docs/qa/next-session-plan.md, "Things to not do again".
#
# Kept for a future module set. The nineteen panels in res/ are hand-authored
# and are repaired with check_corners / fix_corners / check_overlap /
# fix_overlap, not regenerated from here.
#
"""
make_panel.py — generate commiecore panel SVGs from a layout declaration.

Nineteen panels hand-drawn nineteen times is nineteen chances to put a label at
widget-centre y. Declaring the layout and generating the artwork makes the
geometry correct by construction, and makes a change to the grammar a one-line
edit rather than a nineteen-file edit.

Every number here comes from docs/design/commiecore-rack-geometry.md. The
generator refuses to emit a panel that breaks the spec rather than emitting one
that needs catching later by gates.py.

Output carries <text> elements. Run the Inkscape text-to-paths pass afterwards
(tools/text_to_paths.ps1) — nanosvg renders neither text nor fonts.

Usage:
    python tools/make_panel.py --module Drift
    python tools/make_panel.py --all
    python tools/make_panel.py --all --check     # geometry only, write nothing

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
LAYOUTS = ROOT / "tools" / "panel_layouts.json"
RES = ROOT / "res"

PX_PER_MM = 75.0 / 25.4
PX_PER_HP = 15.0
PANEL_H_MM = 128.5
PANEL_H_PX = 380.0
HP_MM = 5.08

Z = {
    "masthead": (0.0, 11.0),
    "display":  (13.5, 39.5),
    "control":  (44.0, 97.0),
    "ports":    (102.0, 122.0),
}
SIDE = 5.0

CLEARANCE = {("knob", "knob"): 12.0, ("jack", "jack"): 9.0,
             ("knob", "jack"): 10.09, ("trimpot", "jack"): 8.0,
             ("trimpot", "trimpot"): 8.0, ("knob", "trimpot"): 9.0}

RADIUS_MM = {"knob": 4.0, "trimpot": 3.0, "jack": 3.2, "switch": 3.0}

COL = {
    "surface": "#111410", "raised": "#1A1F16", "well": "#0C0F0A",
    "phosphorBg": "#060807", "phosphor": "#39FF14",
    "cream": "#E8E4D4", "steel": "#8A9080", "mutedOlive": "#6B7A58",
    "structOrange": "#C8661A", "safetyOrange": "#E87D00",
    "border": "#1C2318", "borderDeep": "#242B1E",
}
PORT_STROKE = {
    "audio": "#C8661A", "cv": "#C8B84A", "pitch": "#C8DBC0",
    "clock": "#C8B84A", "gate": "#6BFF2A", "trig": "#C8B84A",
}

LABEL_SIZE = 5.0          # px
LABEL_GAP_MM = 1.2        # baseline to widget top edge (spec §4)


def mm(v: float) -> float:
    return v * PX_PER_MM


class PanelError(Exception):
    pass


def check_geometry(slug: str, hp: int, widgets: list[dict]) -> list[str]:
    """Return every spec violation. Empty list means the layout is legal."""
    problems = []
    width_mm = hp * HP_MM

    for w in widgets:
        kind, x, y, name = w["kind"], w["x"], w["y"], w.get("label", "?")
        r = RADIUS_MM.get(kind, 3.0)

        if x - r < SIDE or x + r > width_mm - SIDE:
            problems.append(
                f"{name} ({kind}) at x={x} breaks the {SIDE}mm side margin "
                f"on a {width_mm:.2f}mm panel")

        zone = w.get("zone")
        if zone and zone in Z:
            top, bot = Z[zone]
            if y - r < top or y + r > bot:
                problems.append(
                    f"{name} ({kind}) at y={y} (r={r}) leaves the {zone} zone "
                    f"[{top}, {bot}]")

        # the label sits above the widget and must clear the zone top too
        if w.get("label"):
            baseline = y - r - LABEL_GAP_MM
            if zone and zone in Z and baseline < Z[zone][0]:
                problems.append(
                    f"{name} label baseline {baseline:.2f}mm is above the "
                    f"{zone} zone top {Z[zone][0]}mm")

    for i in range(len(widgets)):
        for j in range(i + 1, len(widgets)):
            a, b = widgets[i], widgets[j]
            key = tuple(sorted((a["kind"], b["kind"])))
            key = key if key in CLEARANCE else tuple(reversed(key))
            minimum = CLEARANCE.get(key)
            if minimum is None:
                continue
            d = ((a["x"] - b["x"]) ** 2 + (a["y"] - b["y"]) ** 2) ** 0.5
            if d < minimum - 1e-6:
                problems.append(
                    f"{a.get('label','?')} and {b.get('label','?')} are "
                    f"{d:.2f}mm apart; {a['kind']}/{b['kind']} needs {minimum}mm")

    return problems


def emit(slug: str, spec: dict) -> str:
    hp = spec["hp"]
    widgets = spec["widgets"]
    width_mm = hp * HP_MM
    width_px = hp * PX_PER_HP

    o: list[str] = []
    a = o.append

    a(f'<svg xmlns="http://www.w3.org/2000/svg" version="1.1" '
      f'width="{width_mm:.2f}mm" height="{PANEL_H_MM}mm" '
      f'viewBox="0 0 {width_px:.0f} {PANEL_H_PX:.0f}">')
    a(f'  <!-- {slug} — {hp} HP. Generated by tools/make_panel.py. '
      f'Do not hand-edit; edit tools/panel_layouts.json. -->')

    # body
    a(f'  <rect x="0" y="0" width="{width_px:.2f}" height="{PANEL_H_PX:.0f}" '
      f'fill="{COL["surface"]}"/>')

    # masthead rule — the name itself is drawn in NanoVG, never here
    a(f'  <rect x="0" y="{mm(Z["masthead"][1]):.2f}" width="{width_px:.2f}" '
      f'height="1.2" fill="{COL["structOrange"]}"/>')

    # display well
    dx, dy = mm(SIDE), mm(Z["display"][0])
    dw = mm(width_mm - SIDE * 2)
    dh = mm(Z["display"][1] - Z["display"][0])
    a(f'  <rect x="{dx:.2f}" y="{dy:.2f}" width="{dw:.2f}" height="{dh:.2f}" '
      f'rx="2" fill="{COL["phosphorBg"]}" stroke="{COL["borderDeep"]}" '
      f'stroke-width="1"/>')

    # a faint rule under the control field separates it from the port rail
    ry = mm((Z["control"][1] + Z["ports"][0]) / 2.0)
    a(f'  <rect x="{mm(SIDE):.2f}" y="{ry:.2f}" width="{dw:.2f}" height="0.7" '
      f'fill="{COL["border"]}"/>')

    # section headings
    for s in spec.get("sections", []):
        a(f'  <text x="{mm(s["x"]):.2f}" y="{mm(s["y"]):.2f}" '
          f'font-family="sans-serif" font-size="{LABEL_SIZE - 0.4:.1f}" '
          f'font-weight="bold" letter-spacing="0.6" fill="{COL["structOrange"]}" '
          f'text-anchor="{s.get("anchor", "middle")}">{s["text"]}</text>')

    # widgets: outline plus a label that always sits above it
    for w in widgets:
        kind, x, y = w["kind"], w["x"], w["y"]
        r = RADIUS_MM.get(kind, 3.0)
        cx, cy = mm(x), mm(y)

        if kind == "jack":
            stroke = PORT_STROKE.get(w.get("signal", "cv"), COL["steel"])
            a(f'  <circle cx="{cx:.2f}" cy="{cy:.2f}" r="{mm(r):.2f}" '
              f'fill="{COL["well"]}" stroke="{stroke}" stroke-width="1.1"/>')
        elif kind == "trimpot":
            a(f'  <circle cx="{cx:.2f}" cy="{cy:.2f}" r="{mm(r):.2f}" '
              f'fill="{COL["raised"]}" stroke="{COL["borderDeep"]}" '
              f'stroke-width="0.8"/>')
        else:
            a(f'  <circle cx="{cx:.2f}" cy="{cy:.2f}" r="{mm(r):.2f}" '
              f'fill="{COL["raised"]}" stroke="{COL["border"]}" '
              f'stroke-width="1"/>')

        if w.get("label"):
            by = mm(y - r - LABEL_GAP_MM)
            fill = COL["cream"] if kind in ("knob", "jack") else COL["mutedOlive"]
            size = LABEL_SIZE if kind != "trimpot" else LABEL_SIZE - 0.8
            a(f'  <text x="{cx:.2f}" y="{by:.2f}" font-family="sans-serif" '
              f'font-size="{size:.1f}" letter-spacing="0.4" fill="{fill}" '
              f'text-anchor="middle">{w["label"]}</text>')

    a('</svg>')
    return "\n".join(o) + "\n"


def main() -> int:
    ap = argparse.ArgumentParser()
    ap.add_argument("--module", action="append")
    ap.add_argument("--all", action="store_true")
    ap.add_argument("--check", action="store_true",
                    help="validate geometry, write nothing")
    args = ap.parse_args()

    if not LAYOUTS.exists():
        print(f"no layout file at {LAYOUTS.relative_to(ROOT)}")
        return 1
    layouts = json.loads(LAYOUTS.read_text(encoding="utf-8"))

    # keys beginning with _ are notes in the layout file, not modules
    slugs = args.module or (sorted(k for k in layouts if not k.startswith("_"))
                            if args.all else [])
    if not slugs:
        ap.error("give --module SLUG or --all")

    failed = 0
    for slug in slugs:
        spec = layouts.get(slug)
        if spec is None:
            print(f"  {slug}: no layout declared")
            failed += 1
            continue

        problems = check_geometry(slug, spec["hp"], spec["widgets"])
        if problems:
            failed += 1
            print(f"\n  {slug} — {len(problems)} geometry violation(s)")
            for p in problems:
                print(f"      x {p}")
            continue

        if args.check:
            print(f"  {slug}: {spec['hp']} HP, {len(spec['widgets'])} widgets — OK")
            continue

        out = RES / f"{slug}.svg"
        out.write_text(emit(slug, spec), encoding="utf-8")
        print(f"  {slug}: wrote {out.relative_to(ROOT)} "
              f"({spec['hp']} HP, {len(spec['widgets'])} widgets)")

    if failed:
        print(f"\n{failed} module(s) failed geometry validation.\n")
        return 1
    print(f"\n{len(slugs)} module(s) OK.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
