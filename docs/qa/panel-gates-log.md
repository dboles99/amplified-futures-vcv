# Panel gates log

Every defect this project has actually shipped, written as a gate the next set
of modules has to pass before anyone looks at it.

The rule for adding to this list: **a defect only earns a gate once it has
reached a render.** Hypothetical failures do not go here. Everything below cost
real time or a real submission.

Carry this file forward to the next Rack 2 module set. A gate that only lives
in someone's head is not a gate.

---

## G-01 · Nothing but a band may enter a screw corner

**Cost:** DroneCore shipped with its own title unreadable — the mounting screws
cover the leading `D` and trailing `E`. Six other panels lost letters from the
`AMPL. FUTURES` footer. Present in every release to date.

Screws are drawn *over* the panel at fixed positions:

```
top-left      (15, 0)                    bottom-left   (15, 365)
top-right     (width - 30, 0)            bottom-right  (width - 30, 365)
```

Each is 15x15 px. Artwork may pass under a screw only if it spans the full
panel width (header bar, footer rule) or is a counter-hole drawn concentric
with the screw. Anything else is hidden at runtime.

**Gate:** `tools/check_corners.py` — exit 0 required.

---

## G-02 · A label never shares a y with its widget

**Cost:** the whole July 2026 submission. Ten of fourteen panels drew each knob
and port directly over its own caption. `DENSITY` rendered as `Y`, `PRESSURE`
as `RE`, `SUM` vanished entirely. The reviewer said *"the panels look broken on
this"* and the thread never recovered.

Label baselines sit **above** the widget, clear of its top edge by at least
1.2 mm. Never at widget-centre y, never beside it.

**Gate:** visual, on a Rack render. Attempts to check this geometrically from
bezier outlines were abandoned — see G-09.

---

## G-03 · Every control carries a caption

**Cost:** SwarmCore ships four unlabelled trimpots. WallConductor,
StringMassCore and HarmonicPressure label the knob and its CV jack but never
the attenuverter between them.

Every knob, trimpot, slider and jack has a visible panel caption. When it will
not fit, compress it — conventional abbreviation, then devowelled, then
initialism, then glyph — but never omit it.

**Gate:** visual for the panel; `tools/check_labels.py` for the code-side
backstop, which must also pass.

---

## G-04 · Every param and port is named in code

**Cost:** none yet — this one was clean when first measured, and that is why
aggressive panel compression is safe. `PRSSR` on the panel is legible only
because the tooltip still says `Pressure`.

Every enum member has a `configParam`, `configSwitch`, `configButton`,
`configInput` or `configOutput` call.

**Gate:** `tools/check_labels.py` — 19/19 named.

---

## G-05 · Tags come from the published whitelist

**Cost:** `Drift` shipped the tag `Modulation` and `QuadVCA` shipped `VCA`.
Neither exists. The whitelist spells the latter `Voltage-controlled amplifier`.
A manifest defect is a mechanical rejection that never reaches a human.

Also: an optional manifest field present but empty ships a blank link.
`changelogUrl` was `""`; `authorUrl` was the same defect in June.

**Gate:** `tools/gates.py --gate 1`.

---

## G-06 · 1 mm is 2.952756 px

**Cost:** MassDriver's V/OCT output was placed at y = 126 mm on a 128.5 mm
panel and hung off the bottom edge.

Rack renders SVG at 75 DPI. The workspace notes said `1 mm = 3 px`, which is
wrong by 1.5% — 5.5 px of drift across a full panel. Place widgets with Rack's
own `mm2px()` and never hand-convert.

**Gate:** `tools/gates.py --gate 3` checks widget bounds; the constant lives in
`docs/design/commiecore-rack-geometry.md` §1.

---

## G-07 · No orphaned artwork

**Cost:** MassDriver ships `MUTE` and `IN` captions at the foot of both channel
columns with no widgets under them — labels for a ninth row that does not
exist. DroneClone shipped diagonal construction guide lines and annotation
boxes into a public release.

Artwork that labels nothing, or that was scaffolding, does not ship.

**Gate:** visual, on a render.

---

## G-08 · A control group sits with its own heading

**Cost:** SitarGrid's six JHALA knobs render *above* the `JHALA BREAKDOWN`
header, inside the neighbouring `GLOBAL` section, with their captions far
below. The group reads as belonging to the wrong module section.

**Gate:** visual, on a render.

---

## G-09 · The render is the instrument, not the source

**Cost:** twice. The 2026-06-29 comment asserted the panels were fixed after
reading SVG sources; Rack drew something different and the reviewer saw it. On
2026-08-03 the same mistake nearly repeated — "the panels are no longer your
blocker" was written after checking dimensions on one panel, and DroneClone
contradicted it two panels later.

No claim about a panel is made from the SVG, from a checker, or from a
summary. Render with `Rack.exe -u <clean dir> -t 2` and look at the PNG.

A corollary, learned the same day: a checker that cries wolf is worse than no
checker. A geometric label-placement detector produced 109 findings on two
panels where the render showed five, because widget coordinates are often
symbolic (`x1`, `ky1`) rather than literal. It was deleted rather than fixed.
Check what is genuinely mechanical — corners, manifests, enum order. Look at
the rest.

---

## G-10 · Enum order is append-only

**Cost:** none yet, and it must stay that way. Rack serialises params, inputs
and outputs by position. Inserting a member mid-list silently breaks every
saved patch and every preset in `presets/`.

Widget coordinates may move freely. Enum order may not.

**Gate:** `tools/gates.py --gate 5` diffs each enum against the previous
release tag.

---

## G-11 · Width follows content

**Cost:** a pilot redesign widened Drift from 12 HP to 16 HP on a flat +4 HP
rule. The result was measurably worse than what already shipped — sparser,
lower contrast, and it introduced a label collision. Fundamental's LFO fits
four params and eight ports into 10 HP; the 16 HP version held less and looked
emptier.

Widen a module because its clearances are unsatisfiable at the current width —
Choke had knob centres 8.6 mm from their own CV jacks where 10.09 mm is
required, and SwarmCore needed 130 mm of height on a 128.5 mm panel. Both
genuinely had to grow. Do not widen a module that merely has room.

Negative space frames something dense. It cannot frame more negative space.

---

## G-13 · A checker is wrong until it has been proven wrong four times

**Cost:** most of the 2026-08-03 session. Every checker written that day gave a
confidently false answer first:

- `check_corners` reported **19 of 19** panels broken, because it counted the
  header band that is *meant* to pass under the screws. Then it reported its
  own fix as still broken, because it ignored `transform`. Then it missed the
  counter-hole defect entirely, because five panels store them as `<circle>`
  and it only read `<path>`.
- `check_labels` reported **11 modules** with unnamed params. All eleven were
  false: `configParam` hides inside literal-bound loops, enum-bounded loops and
  braceless one-liners, `configSwitch`/`configButton` also name a param, and
  SitarGrid ends its enums `NUM_INPUTS` where the AF modules use `INPUTS_LEN`.
- `check_overlap` reported **97** overlaps using guessed widget radii. Trimpot
  is 3.02 mm, not the 4.5 mm assumed — a 50% error. Reading the real sizes out
  of Rack's own component SVGs cut it to 44.
- `gates.py` G3 failed a panel over its **mounting screws**, which sit at
  y = 124 mm by design.

Before believing a checker, run it against something known-good and something
known-broken. A checker that cries wolf gets ignored, and an ignored checker is
worse than none.

---

## G-14 · Decoration is concentric; text is not

**Cost:** a caption on Pulse was moved *underneath* the CRACK knob by an
automated fix, and the checker then called the panel clean.

Panels draw sockets, knob faces and button bezels in the artwork beneath the
real widget. That ink is meant to be covered and must be excluded. But the
exclusion has to be tight: real decoration is drawn **exactly** concentric with
its widget — within about 0.05 mm — whereas a caption that merely happens to
land near a widget centre is off by around 1 mm. Testing concentricity within
0.5 mm separates them cleanly; testing within 1 mm hides real defects.

Aspect ratio alone does not work: button bezels are 2:1, and so are short
captions.

---

## G-15 · Moving a label needs a clear-gap search, not a direction

**Cost:** the first automated caption lift moved labels out from under a
trimpot and straight into the jack above it. Pulse stacks a trimpot, a jack and
another trimpot inside 20 mm; there is no room to simply "move it up".

Search outward from the current position and take the smallest offset that
clears **every** widget, not just the one the label started under. Also respect
direction: a range caption below a knob — `82-1319 Hz` — belongs there and
needs to clear the knob's bottom edge, not be hoisted over the top of it into
the main label.

If no offset clears everything, say so and stop. That is a layout change, not
a nudge.

---

## G-12 · Check the whole set, not the ones you changed

**Cost:** the July repair pass fixed the modules it touched and left seven
untouched. Those seven — DroneClone, WallConductor, StringMassCore,
HarmonicPressure, DroneCore, Pulse, SitarGrid — still carry the original
defects, and DroneClone is the worst panel in the set months after it was
scored 27/35.

Every panel renders and is scored on every release, not just the edited ones.
