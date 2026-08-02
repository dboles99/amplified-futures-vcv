#!/usr/bin/env python3
"""
gates.py — six gates a panel passes before it is called done.

The July 2026 submission failed twice for the same reason: a claim was made from
reading the source instead of from looking at the render. These gates exist so
that "the panels are perfect" is a command you run, not an opinion you hold.

    G1  MANIFEST   plugin.json against the published VCV manifest spec
    G2  SOURCE     SVG document structure and renderer compatibility
    G3  GEOMETRY   widget positions: panel bounds, zones, clearances
    G4  LABELS     every control named on the panel and in code
    G5  STATE      enum stability, presets, core tests
    G6  VISUAL     rendered PNG compared against library conventions

G1-G5 are mechanical and run offline; that is the dry run. G6 needs a Rack
render and is the only gate with a human in it.

Usage:
    python tools/gates.py                 # all gates, all modules
    python tools/gates.py --gate 1        # one gate
    python tools/gates.py --module Drift  # one module
    python tools/gates.py --dry-run       # G1-G5 only, skip the render gate

Exit 0 = every gate run passed.  Exit 1 = at least one failure.

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import argparse
import json
import re
import subprocess
import sys
from dataclasses import dataclass, field
from pathlib import Path
from xml.etree import ElementTree as ET

ROOT = Path(__file__).resolve().parent.parent
SRC, RES = ROOT / "src", ROOT / "res"
PRESETS, PANELS = ROOT / "presets", ROOT / "docs" / "panels" / "rack"

MM_PER_PX = 25.4 / 75.0          # Rack renders SVG at 75 DPI
PX_PER_MM = 75.0 / 25.4          # 2.952755905...
PX_PER_HP = 15.0
PANEL_H_MM = 128.5
PANEL_H_PX = 380.0

# docs/design/commiecore-rack-geometry.md §3
ZONES = {
    "masthead": (0.0, 11.0),
    "display":  (13.5, 39.5),
    "control":  (44.0, 97.0),
    "ports":    (102.0, 122.0),
}
SIDE_MARGIN_MM = 5.0

# §4 clearances, centre to centre
CLEARANCE = {
    ("knob", "jack"):    10.09,
    ("knob", "knob"):    12.0,
    ("jack", "jack"):     9.0,
    ("trimpot", "jack"):  8.0,
}

# vcvrack.com/manual/Manifest — the complete whitelist, verbatim
VALID_TAGS = {
    "Arpeggiator", "Attenuator", "Blank", "Chorus", "Clock generator",
    "Clock modulator", "Compressor", "Controller", "Delay", "Digital",
    "Distortion", "Drum", "Dual", "Dynamics", "Effect", "Envelope follower",
    "Envelope generator", "Equalizer", "Expander", "External", "Filter",
    "Flanger", "Function generator", "Granular", "Hardware clone", "Limiter",
    "Logic", "Low-frequency oscillator", "Low-pass gate", "MIDI", "Mixer",
    "Multiple", "Noise", "Oscillator", "Panning", "Phaser", "Physical modeling",
    "Polyphonic", "Quad", "Quantizer", "Random", "Recording", "Reverb",
    "Ring modulator", "Sample and hold", "Sampler", "Sequencer", "Slew limiter",
    "Speech", "Switch", "Synth voice", "Tuner", "Utility", "Visual", "Vocoder",
    "Voltage-controlled amplifier", "Waveshaper",
}
REQUIRED_PLUGIN_FIELDS = ("slug", "name", "version", "license", "author")
SLUG_RE = re.compile(r"^[A-Za-z0-9_-]+$")
VERSION_RE = re.compile(r"^\d+\.\d+\.\d+$")

# nanosvg cannot render these; the manual lists them as unsupported
UNSUPPORTED_SVG = ("text", "tspan", "filter", "mask", "clipPath", "use",
                   "style", "foreignObject", "image", "switch")

WIDGET_KIND = {
    "knob":    re.compile(r"createParamCentered<(?:[^>]*?)(Knob|Slider|Fader)"),
    "trimpot": re.compile(r"createParamCentered<\s*Trimpot"),
    "jack":    re.compile(r"create(?:Input|Output)Centered<"),
}
MM2PX_RE = re.compile(r"mm2px\(\s*Vec\(\s*([\d.f+-]+)\s*,\s*([\d.f+-]+)\s*\)\s*\)")


@dataclass
class Result:
    gate: str
    module: str
    failures: list[str] = field(default_factory=list)
    notes: list[str] = field(default_factory=list)

    @property
    def passed(self) -> bool:
        return not self.failures


def num(token: str) -> float | None:
    try:
        return float(token.rstrip("f"))
    except ValueError:
        return None


# ── G1 ────────────────────────────────────────────────────────────────────

def gate1_manifest() -> Result:
    r = Result("G1 MANIFEST", "plugin.json")
    path = ROOT / "plugin.json"
    if not path.exists():
        r.failures.append("plugin.json missing")
        return r

    d = json.loads(path.read_text(encoding="utf-8"))

    for f in REQUIRED_PLUGIN_FIELDS:
        if not d.get(f):
            r.failures.append(f"required field .{f} missing or empty")

    if (slug := d.get("slug", "")) and not SLUG_RE.match(slug):
        r.failures.append(f"slug '{slug}' has characters outside [A-Za-z0-9_-]")
    if (v := d.get("version", "")) and not VERSION_RE.match(v):
        r.failures.append(f"version '{v}' is not MAJOR.MINOR.REVISION")
    if v.startswith("v"):
        r.failures.append("version must not carry a 'v' prefix")
    if v and not v.startswith("2."):
        r.failures.append(f"MAJOR must match the Rack version (2), got '{v}'")

    # An empty optional field is worse than an absent one: it ships a blank
    # link. This was flagged on the June submission for authorUrl.
    for k, val in d.items():
        if k != "modules" and isinstance(val, str) and not val.strip():
            r.failures.append(f"optional field .{k} is present but empty — remove it")

    seen = set()
    for m in d.get("modules", []):
        ms = m.get("slug", "?")
        if not m.get("slug"):
            r.failures.append("a module has no slug")
        elif not SLUG_RE.match(ms):
            r.failures.append(f"module slug '{ms}' has invalid characters")
        if ms in seen:
            r.failures.append(f"duplicate module slug '{ms}'")
        seen.add(ms)
        if not m.get("name"):
            r.failures.append(f"module '{ms}' has no name")
        for tag in m.get("tags", []):
            if tag not in VALID_TAGS:
                r.failures.append(
                    f"module '{ms}' uses tag '{tag}', which is not in the "
                    f"VCV whitelist")

    # every declared module must have a panel and a source file
    for m in d.get("modules", []):
        ms = m.get("slug")
        if ms and not (RES / f"{ms}.svg").exists():
            r.failures.append(f"module '{ms}' declared with no res/{ms}.svg")

    r.notes.append(f"{len(d.get('modules', []))} modules declared")
    return r


# ── G2 ────────────────────────────────────────────────────────────────────

def gate2_source(slug: str) -> Result:
    r = Result("G2 SOURCE", slug)
    path = RES / f"{slug}.svg"
    if not path.exists():
        r.failures.append("panel SVG missing")
        return r

    raw = path.read_text(encoding="utf-8", errors="replace")

    for tag in UNSUPPORTED_SVG:
        if re.search(rf"<{tag}[\s>/]", raw):
            r.failures.append(f"<{tag}> is not rendered by nanosvg — remove it")

    try:
        root = ET.fromstring(raw)
    except ET.ParseError as e:
        r.failures.append(f"SVG does not parse: {e}")
        return r

    width, height = root.get("width", ""), root.get("height", "")
    viewbox = root.get("viewBox", "")

    if not width.endswith("mm"):
        r.failures.append(f"width='{width}' must be in mm")
    if not height.endswith("mm"):
        r.failures.append(f"height='{height}' must be in mm")
    if height.endswith("mm") and abs(num(height[:-2]) - PANEL_H_MM) > 0.01:
        r.failures.append(f"height must be {PANEL_H_MM}mm, got {height}")

    parts = viewbox.split()
    if len(parts) != 4:
        r.failures.append(f"viewBox='{viewbox}' is malformed")
        return r

    vb_w, vb_h = float(parts[2]), float(parts[3])
    if abs(vb_h - PANEL_H_PX) > 0.5:
        r.failures.append(f"viewBox height must be {PANEL_H_PX}, got {vb_h}")

    hp = vb_w / PX_PER_HP
    if abs(hp - round(hp)) > 1e-6:
        r.failures.append(
            f"viewBox width {vb_w} is not a whole HP multiple ({hp:.3f} HP)")

    if width.endswith("mm"):
        expected = round(hp) * 5.08
        if abs(num(width[:-2]) - expected) > 0.02:
            r.failures.append(
                f"width {width} disagrees with viewBox: {round(hp)} HP "
                f"should be {expected:.2f}mm")

    r.notes.append(f"{round(hp)} HP, viewBox {vb_w:.0f}x{vb_h:.0f}")
    return r


# ── G3 ────────────────────────────────────────────────────────────────────

def widget_positions(text: str) -> list[tuple[str, float, float]]:
    """(kind, x_mm, y_mm) for every mm2px-placed widget."""
    out = []
    for line in text.splitlines():
        m = MM2PX_RE.search(line)
        if not m:
            continue
        x, y = num(m.group(1)), num(m.group(2))
        if x is None or y is None:
            continue
        kind = "other"
        for k, pattern in WIDGET_KIND.items():
            if pattern.search(line):
                kind = k
                break
        else:
            if "createParamCentered" in line:
                kind = "knob"
        out.append((kind, x, y))
    return out


def gate3_geometry(slug: str) -> Result:
    r = Result("G3 GEOMETRY", slug)
    cpp = find_source(slug)
    svg = RES / f"{slug}.svg"
    if cpp is None or not svg.exists():
        r.failures.append("source or panel missing")
        return r

    root = ET.fromstring(svg.read_text(encoding="utf-8", errors="replace"))
    vb_w = float(root.get("viewBox", "0 0 0 0").split()[2])
    width_mm = vb_w / PX_PER_HP * 5.08

    widgets = widget_positions(cpp.read_text(encoding="utf-8", errors="replace"))
    if not widgets:
        r.notes.append("no mm2px widgets found (panel may use Vec px directly)")
        return r

    for kind, x, y in widgets:
        if not (SIDE_MARGIN_MM <= x <= width_mm - SIDE_MARGIN_MM):
            r.failures.append(
                f"{kind} at x={x}mm breaks the {SIDE_MARGIN_MM}mm side margin "
                f"(panel is {width_mm:.2f}mm)")
        if not (0 <= y <= PANEL_H_MM):
            r.failures.append(f"{kind} at y={y}mm is off the panel")
        elif y > ZONES["ports"][1]:
            r.failures.append(
                f"{kind} at y={y}mm is below the port rail "
                f"(ends {ZONES['ports'][1]}mm) — this is the MassDriver failure")

    jacks = [(x, y) for k, x, y in widgets if k == "jack"]
    knobs = [(x, y) for k, x, y in widgets if k == "knob"]

    for label, group, key in (("jack", jacks, ("jack", "jack")),
                              ("knob", knobs, ("knob", "knob"))):
        minimum = CLEARANCE[key]
        for i in range(len(group)):
            for j in range(i + 1, len(group)):
                (ax, ay), (bx, by) = group[i], group[j]
                d = ((ax - bx) ** 2 + (ay - by) ** 2) ** 0.5
                if d < minimum - 0.01:
                    r.failures.append(
                        f"{label}s at ({ax},{ay}) and ({bx},{by}) are {d:.2f}mm "
                        f"apart, minimum {minimum}mm")

    r.notes.append(f"{len(widgets)} widgets: {len(knobs)} knob-class, "
                   f"{len(jacks)} jacks")
    return r


# ── G4 ────────────────────────────────────────────────────────────────────

def gate4_labels(slug: str) -> Result:
    r = Result("G4 LABELS", slug)
    cpp = find_source(slug)
    if cpp is None:
        r.failures.append("source missing")
        return r

    proc = subprocess.run(
        [sys.executable, str(ROOT / "tools" / "check_labels.py"), cpp.stem],
        capture_output=True, text=True, encoding="utf-8", errors="replace")
    if proc.returncode != 0:
        for line in proc.stdout.splitlines():
            if "·" in line:
                r.failures.append(line.strip())

    svg = RES / f"{slug}.svg"
    if svg.exists():
        raw = svg.read_text(encoding="utf-8", errors="replace")
        # After the text-to-paths pass every label is a <path>. A panel with no
        # paths at all has no printed labels on it.
        if raw.count("<path") < 2:
            r.failures.append(
                "panel has almost no <path> elements — labels were never drawn "
                "or never converted to outlines")

    return r


# ── G5 ────────────────────────────────────────────────────────────────────

def gate5_state(slug: str, baseline: str = "v2.2.0") -> Result:
    r = Result("G5 STATE", slug)
    cpp = find_source(slug)
    if cpp is None:
        r.failures.append("source missing")
        return r

    rel = cpp.relative_to(ROOT).as_posix()
    old = subprocess.run(["git", "show", f"{baseline}:{rel}"],
                         capture_output=True, text=True, cwd=ROOT,
                         encoding="utf-8", errors="replace")
    if old.returncode == 0:
        for kind, pattern in (("ParamId", re.compile(r"enum\s+ParamId\s*\{(.*?)\}", re.S)),
                              ("InputId", re.compile(r"enum\s+InputId\s*\{(.*?)\}", re.S)),
                              ("OutputId", re.compile(r"enum\s+OutputId\s*\{(.*?)\}", re.S))):
            a = pattern.search(old.stdout)
            b = pattern.search(cpp.read_text(encoding="utf-8", errors="replace"))
            if not a or not b:
                continue
            before = [x.strip() for x in re.sub(r"//.*", "", a.group(1)).split(",")
                      if x.strip()]
            after = [x.strip() for x in re.sub(r"//.*", "", b.group(1)).split(",")
                     if x.strip()]
            # Appending is allowed. Reordering or removing breaks every patch.
            if after[:len(before)] != before:
                r.failures.append(
                    f"{kind} order changed since {baseline} — this breaks saved "
                    f"patches. New members must be appended, never inserted.")
    else:
        r.notes.append(f"no {baseline} baseline for this module (new since then)")

    d = PRESETS / slug
    if d.exists():
        presets = list(d.glob("*.vcvm"))
        for p in presets:
            try:
                json.loads(p.read_text(encoding="utf-8"))
            except json.JSONDecodeError as e:
                r.failures.append(f"preset {p.name} is not valid JSON: {e}")
        r.notes.append(f"{len(presets)} preset(s)")
    else:
        r.notes.append("no presets")

    return r


# ── G6 ────────────────────────────────────────────────────────────────────

def gate6_visual(slug: str) -> Result:
    """The render gate. Mechanical checks on the PNG; the comparison against
    the library page is a human judgement this gate sets up but cannot make."""
    r = Result("G6 VISUAL", slug)
    png = PANELS / f"{slug}.png"
    if not png.exists():
        r.failures.append(
            f"no Rack render at {png.relative_to(ROOT)} — run "
            f"Rack.exe -u <clean dir> -t 2 before this gate can pass")
        return r

    svg = RES / f"{slug}.svg"
    if svg.exists():
        root = ET.fromstring(svg.read_text(encoding="utf-8", errors="replace"))
        hp = round(float(root.get("viewBox", "0 0 0 0").split()[2]) / PX_PER_HP)
        expected = (int(hp * 15 * 2), 760)
        try:
            import struct
            data = png.read_bytes()
            if data[:8] == b"\x89PNG\r\n\x1a\n":
                w, h = struct.unpack(">II", data[16:24])
                if (w, h) != expected:
                    r.failures.append(
                        f"render is {w}x{h}, expected {expected[0]}x{expected[1]} "
                        f"for {hp} HP at -t 2")
                else:
                    r.notes.append(f"render {w}x{h} correct for {hp} HP")
        except Exception as e:
            r.failures.append(f"could not read PNG header: {e}")

    r.notes.append("VISUAL REVIEW REQUIRED — compare against "
                   "library.vcvrack.com and record what could be better")
    return r


# ── driver ────────────────────────────────────────────────────────────────

def find_source(slug: str) -> Path | None:
    exact = SRC / f"{slug}.cpp"
    if exact.exists():
        return exact
    for p in SRC.glob("*.cpp"):
        if p.stem.lower() == slug.lower():
            return p
    # CollapseSat -> CollapseSaturator, and similar
    for p in SRC.glob("*.cpp"):
        if p.stem.lower().startswith(slug.lower()):
            return p
    return None


def module_slugs() -> list[str]:
    path = ROOT / "plugin.json"
    if not path.exists():
        return sorted(p.stem for p in RES.glob("*.svg"))
    d = json.loads(path.read_text(encoding="utf-8"))
    return [m["slug"] for m in d.get("modules", []) if m.get("slug")]


def main() -> int:
    ap = argparse.ArgumentParser(description="Six gates for panel readiness.")
    ap.add_argument("--gate", type=int, choices=range(1, 7), action="append")
    ap.add_argument("--module", action="append")
    ap.add_argument("--dry-run", action="store_true",
                    help="G1-G5 only; skip the render gate")
    ap.add_argument("--quiet", action="store_true", help="failures only")
    args = ap.parse_args()

    gates = args.gate or list(range(1, 7))
    if args.dry_run:
        gates = [g for g in gates if g != 6]
    slugs = args.module or module_slugs()

    results: list[Result] = []
    if 1 in gates:
        results.append(gate1_manifest())

    per_module = {2: gate2_source, 3: gate3_geometry, 4: gate4_labels,
                  5: gate5_state, 6: gate6_visual}
    for g in gates:
        if g in per_module:
            for slug in slugs:
                results.append(per_module[g](slug))

    by_gate: dict[str, list[Result]] = {}
    for r in results:
        by_gate.setdefault(r.gate, []).append(r)

    total_failures = 0
    for gate in sorted(by_gate):
        rs = by_gate[gate]
        failed = [r for r in rs if not r.passed]
        total_failures += len(failed)
        status = "PASS" if not failed else f"FAIL ({len(failed)}/{len(rs)})"
        print(f"\n{gate}  —  {status}")
        for r in rs:
            if r.failures:
                print(f"    {r.module}")
                for f in r.failures:
                    print(f"        ✗ {f}")
            elif not args.quiet and r.notes:
                print(f"    {r.module}: {'; '.join(r.notes)}")

    print(f"\n{'=' * 64}")
    if total_failures:
        print(f"{total_failures} module-gate failure(s). Not ready.\n")
        return 1
    print("All gates passed.\n")
    return 0


if __name__ == "__main__":
    sys.exit(main())
