# Next session — panel conformance, remaining work

Written 2026-08-03 at the close of the panel-repair session. Branch
`feat/commiecore-panels-v2.2.1`, head `5033ccb`, version `2.3.0` unreleased.

Read `panel-gates-log.md` first. It is fifteen gates written from defects that
actually shipped, and G-09 and G-13 in particular will save this session from
repeating the last one.

---

## Where things stand

Two systematic defect classes are closed and render-verified.

| Gate | Command | State |
|---|---|---|
| G-01 corners | `python tools/check_corners.py` | **0 of 19** |
| G-02 overlaps | `python tools/check_overlap.py` | 4 flagged, all DroneClone, believed section bands |
| G-03/04 naming | `python tools/check_labels.py` | **19 of 19 named** |
| G-05 manifest, geometry, state | `python tools/gates.py --dry-run` | **all pass** |

Nothing below is a regression. All of it predates this work and none of it was
ever recorded before the audit.

---

## Task 1 — settle the four DroneClone flags

`check_overlap` still reports four spans on DroneClone at 17–33 mm wide and
~2.7 mm tall. That is section-band geometry, not caption geometry, so they are
probably false positives — but "probably" is what G-09 exists to prevent.

Render DroneClone, look at the four coordinates, and then either:

- widen the band filter in `check_overlap.py` so it stops reporting them, or
- fix them if they are real.

Do not leave them reported-but-ignored. A gate with known-noise output stops
being read.

    python tools/check_overlap.py DroneClone --verbose

---

## Task 2 — SitarGrid's JHALA group

The six JHALA knobs render **above** the `JHALA BREAKDOWN` header, inside the
neighbouring `GLOBAL` section, with their captions far below them. The group
reads as belonging to the wrong module section.

This is a widget-coordinate change in `src/SitarGrid.cpp`, not artwork. The
knobs sit at `y = 85 mm` in the `rX[]` block; the header band is below them.
Either move the knobs down under their header or move the header up above them
— whichever leaves the `GLOBAL` block coherent.

**SitarGrid places widgets with variables, so `check_overlap` cannot read it.**
It reports `38 widget position(s) use variables`. This one is verified by eye,
on a render, only.

---

## Task 3 — the unlabelled attenuverters

Four panels label the knob and its CV jack but never the attenuverter between
them: **SwarmCore** (4), **WallConductor**, **StringMassCore**,
**HarmonicPressure**.

Every one of these params *is* named in code — `check_labels` passes 19/19 — so
the tooltip resolves. What is missing is the panel caption.

Add `ATN` above each. The ladder in
`docs/design/commiecore-rack-geometry.md` §10 covers the compression; `ATN` is
rung 2 and is what Drift, Pulse and Send already use, so the set stays
consistent.

Authoring route: add `<text>` to the SVG, then

    .\tools\text_to_paths.ps1 -Module <Slug>
    python tools/check_corners.py <Slug>
    python tools/check_overlap.py <Slug>

---

## Task 4 — MassDriver's orphaned captions

`MUTE` and `IN` at the foot of both channel columns label a ninth row that does
not exist. Delete them. Same class as the construction guide lines stripped
from DroneClone in July.

They are at roughly `y = 115 mm`, `x ≈ 9.5` and `x ≈ 29` on the left column and
mirrored on the right. Confirm against the render before deleting — G-09.

---

## Task 5 — Send's clipped FEEDBACK caption

`FEEDBACK` runs off the right panel edge. Either move it inboard or devowel it
to `FDBK`, which is rung 3 of the ladder and fits comfortably.

---

## Task 6 — labels beside knobs

**WallConductor**, **StringMassCore** and **HarmonicPressure** place their main
captions to the *left* of the knob. The rest of the set places them above.
HarmonicPressure does both on the same panel — `PITCH` and `SPREAD` beside,
`PARTIAL` and `COUNT` above.

This is the largest remaining item and the most judgement-heavy. It is also
the one most visible as "this brand has no house style". Same three panels
carry roughly 145 px of dead space directly under the header, which is where
the captions could go if the control block moves up.

Treat it as one layout pass across the three panels, not three separate fixes.

---

## Task 7 — the six panels never inspected

CollapseSat, FeedbackGovernor, QuadVCA, CollapseEG, SignalBloc,
StreetGridClock.

The last four are AF-series and are expected clean on the evidence of Ratchet.
CollapseSat and FeedbackGovernor are original-generation and should be assumed
to carry the systematic defects until a render says otherwise.

    # render everything into a clean user dir
    python tools/gates.py --dry-run

Then open each PNG. Nineteen images is one sitting.

---

## Task 8 — close out

1. `python tools/gates.py` — all six, including the render gate
2. Score every panel on the rubric in `docs/qa/panel-rubric-2026-07-25.md`,
   from the PNG, not the SVG
3. Regenerate `docs/panels/` and the contact sheet
4. Update the wiki source in `docs/wiki/`, then `tools/check_wiki.py --network`
5. Merge to `master`, tag `v2.3.0`, confirm CI green
6. `make dist`, attach the `.vcvplugin` to a GitHub release

---

## The submission, when the panels are done

**Do not open a new issue.** The library's own instructions say *"Create exactly
one thread in the Issue Tracker"* — #912 is that thread, and it is locked. A
second thread for the same slug reads as routing around a moderator decision,
and the Enforcement section treats that as ban-worthy.

The route is **one email to support@vcvrack.com**, which is the appeal channel
the rules name.

What this session can hand over: version, commit hash, the audit document, and
before/after renders. What it cannot hand over is the text. The rule is that a
post must reflect *your* judgement and voice, and the thread was closed because
that was not true of the last ones. Terse is correct and is what the process
asks for anyway — version, hash, what changed.

---

## Things to not do again

- **Do not redesign the panels.** A commiecore three-zone pilot was built and
  reverted this session: Drift at 16 HP came out sparser and lower-contrast than
  the 12 HP version it replaced, and introduced a label collision. Fundamental's
  LFO fits more into 10 HP. The infrastructure survives in
  `tools/make_panel.py`, `tools/panel_layouts.json` and `src/AFCommiecore.hpp`
  for a *future* module set. It is not for this one.
- **Do not widen modules that merely have room.** Widen only where a clearance
  is unsatisfiable — Choke and SwarmCore genuinely had to grow in July.
- **Do not trust a new checker.** Every one written this session was confidently
  wrong first. Run it against a known-good panel and a known-broken one before
  believing a single number it prints.
