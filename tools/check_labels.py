#!/usr/bin/env python3
"""
check_labels.py — every param, input and output must be named in code.

Panel text is compressed by the labelling ladder (see
docs/design/commiecore-rack-geometry.md §10), so a knob may legitimately read
"PRSSR" on the panel. Rack's hover tooltip is what resolves that back to
"Pressure" — but only if configParam/configInput/configOutput was called for it.

An unnamed param is invisible to the user no matter how good the panel is.
This script counts enum members against config* calls and reports the gap.

Usage:
    python tools/check_labels.py            # all modules
    python tools/check_labels.py Drift      # one module

Exit 0 = every enum member is named.  Exit 1 = gaps found.

Copyright (c) 2026 Daniel Boles. MIT.
"""

from __future__ import annotations

import re
import sys
from pathlib import Path

SRC = Path(__file__).resolve().parent.parent / "src"

# enum ParamId { A, B, ..., PARAMS_LEN };
ENUM_RE = {
    "param":  re.compile(r"enum\s+ParamId\s*\{(.*?)\}", re.S),
    "input":  re.compile(r"enum\s+InputId\s*\{(.*?)\}", re.S),
    "output": re.compile(r"enum\s+OutputId\s*\{(.*?)\}", re.S),
}
CONFIG_RE = {
    # configSwitch and configButton name a param just as configParam does;
    # omitting them reports every module with a toggle as broken.
    "param":  re.compile(r"\bconfig(?:Param|Switch|Button)\s*(?:<[^>]*>)?\s*\("),
    "input":  re.compile(r"\bconfigInput\s*\("),
    "output": re.compile(r"\bconfigOutput\s*\("),
}
# Both conventions are in use: the AF modules end with PARAMS_LEN, SitarGrid
# (ported from an older scaffold) ends with NUM_PARAMS. Counting the sentinel as
# a member inflates the declared count and reports a phantom gap.
SENTINELS = {
    "param":  ("PARAMS_LEN", "NUM_PARAMS"),
    "input":  ("INPUTS_LEN", "NUM_INPUTS"),
    "output": ("OUTPUTS_LEN", "NUM_OUTPUTS"),
}


def enum_members(body: str, sentinels: tuple[str, ...]) -> list[str]:
    body = re.sub(r"//.*", "", body)
    body = re.sub(r"/\*.*?\*/", "", body, flags=re.S)
    names = []
    for raw in body.split(","):
        name = raw.strip()
        if not name or name in sentinels:
            continue
        # strip explicit values: FOO = 3
        name = name.split("=")[0].strip()
        if re.fullmatch(r"[A-Z_][A-Z0-9_]*", name):
            names.append(name)
    return names


# A config call inside a loop names one thing per iteration. Bounds appear both
# as literals (`i < 4`) and as enum members (`i < PARAMS_LEN`), and both forms
# are used in this codebase — handling only literals reports DroneClone's ten
# loop-configured attenuverters as unnamed.
FOR_RE = re.compile(
    r"for\s*\(\s*(?:int|size_t|auto)\s+\w+\s*=\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*;"
    r"\s*\w+\s*<\s*([A-Za-z_][A-Za-z0-9_]*|\d+)\s*;")


def build_symbols(text: str) -> dict[str, int]:
    """Every enum member mapped to its ordinal, so enum-bounded loops resolve."""
    symbols: dict[str, int] = {}
    for kind, pattern in ENUM_RE.items():
        m = pattern.search(text)
        if not m:
            continue
        members = enum_members(m.group(1), SENTINELS[kind])
        for index, name in enumerate(members):
            symbols[name] = index
        for sentinel in SENTINELS[kind]:
            symbols[sentinel] = len(members)
    return symbols


def resolve(token: str, symbols: dict[str, int]) -> int | None:
    if token.isdigit():
        return int(token)
    return symbols.get(token)


def loop_spans(text: str, symbols: dict[str, int]) -> list[tuple[int, int, int]]:
    """(body_start, body_end, iterations) for each resolvable for-loop."""
    spans = []
    for m in FOR_RE.finditer(text):
        lo = resolve(m.group(1), symbols)
        hi = resolve(m.group(2), symbols)
        if lo is None or hi is None:
            continue
        if hi - lo <= 0:
            continue

        # The loop body may be braced or a single statement. DroneClone's ten
        # attenuverters are configured by a braceless one-liner; scanning ahead
        # for the next "{" would swallow an unrelated block and miscount.
        brace = text.find("{", m.end())
        semi = text.find(";", m.end())
        if brace == -1 or (semi != -1 and semi < brace):
            if semi == -1:
                continue
            spans.append((m.end(), semi, hi - lo))
            continue

        depth, i = 0, brace
        while i < len(text):
            if text[i] == "{":
                depth += 1
            elif text[i] == "}":
                depth -= 1
                if depth == 0:
                    break
            i += 1
        spans.append((brace, i, hi - lo))
    return spans


def count_configured(text: str, pattern: re.Pattern,
                     spans: list[tuple[int, int, int]]) -> int:
    """Each call counts once, or once per iteration if inside a loop."""
    total = 0
    for m in pattern.finditer(text):
        multiplier = 1
        for start, end, iterations in spans:
            if start < m.start() < end:
                multiplier = max(multiplier, iterations)
        total += multiplier
    return total


def audit(path: Path) -> list[str]:
    text = path.read_text(encoding="utf-8", errors="replace")
    symbols = build_symbols(text)
    spans = loop_spans(text, symbols)
    problems = []
    for kind, pattern in ENUM_RE.items():
        m = pattern.search(text)
        if not m:
            continue
        declared = enum_members(m.group(1), SENTINELS[kind])
        configured = count_configured(text, CONFIG_RE[kind], spans)
        if configured < len(declared):
            problems.append(
                f"{kind}s: {len(declared)} declared, {configured} named "
                f"— {len(declared) - configured} with no tooltip"
            )
    return problems


def main() -> int:
    targets = sorted(SRC.glob("*.cpp"))
    if len(sys.argv) > 1:
        wanted = {a.lower() for a in sys.argv[1:]}
        targets = [p for p in targets if p.stem.lower() in wanted]
        if not targets:
            print(f"no module matching {sys.argv[1:]}")
            return 1

    skip = {"plugin"}
    failures = 0
    clean = 0

    for path in targets:
        if path.stem in skip:
            continue
        problems = audit(path)
        if problems:
            failures += 1
            print(f"\n  {path.stem}")
            for p in problems:
                print(f"      · {p}")
        else:
            clean += 1

    print(f"\n{clean} module(s) fully named, {failures} with gaps.\n")
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
