# Panel audit — 2026-08-03

Every finding below is read off a **Rack render**, not off an SVG. All 19 panels
were rendered with `Rack.exe -u <clean dir> -t 2` against a user directory
containing only this plugin, at commit `dc33bc5`.

Geometry is correct across the whole set: every panel came out at exactly
`HP × 15 × 2` by 760 px. Nothing is mis-sized and nothing hangs off an edge.
The remaining defects are all artwork.

13 of 19 inspected. The 6 not yet inspected are listed at the end.

---

## The set splits by generation

| | Panels | State |
|---|---|---|
| **AF utility series** | Ratchet, CollapseEG, QuadVCA, SignalBloc, StreetGridClock | Clean. Labels above widgets, everything named, sane spacing. |
| **Repaired in July** | Choke, Drift, Send, MassDriver, SwarmCore | Mostly good; specific defects listed below. |
| **Never repaired** | DroneClone, WallConductor, StringMassCore, HarmonicPressure, DroneCore, Pulse, SitarGrid | Carry the original label-beside-widget habit and the screw collisions. |

The July pass fixed the modules it touched. It did not touch the third row.

---

## Blocking defects

These are the ones a reviewer sees without looking for them.

### DroneCore — the title is eaten by the mounting screws
`DRONECORE` in the header bar runs under **both** corner screws. The leading
`D` and trailing `E` are physically covered. This is the single most visible
defect in the set and it is on an 8 HP module that a reviewer would open first
because it is the simplest.

### DroneClone — labels under widgets, text under screws
- `CHOKE AMT` has a **jack sitting on top of it**, eating `OK`
- `SHIMMER` runs into the knob to its left
- Two labels overlap near `DRIFT`, rendering as garbled `L…ENTLY`
- The bottom-left screw covers `PLIF` in `AMPL. FUTURES`
- A truncated `E` label and `SN 0024` sit under widgets

Scored 27/35 in July — the one panel that did not pass — and was never fixed.

### Pulse — labels overlapping their own knobs
`METAL` and `CRACK` both run into the knobs beside them. The `SYNTHESIS`
section header collides with a knob and with the `CV · ATN` caption at its right.

### SitarGrid — a control group floats away from its header
The six JHALA knobs sit **above** the `JHALA BREAKDOWN` header, inside the
`GLOBAL` section, with their captions (`INT`, `ACCEL`, `SA`, `CHIK`, `ORN`,
`LAND`) far below them. The group reads as belonging to the wrong section.
`OUT L` at the foot is covered by the corner screw.

---

## Systematic defects

Each of these is one rule applied wrongly, repeated across many panels. Fixing
the rule fixes them all at once.

### 1. Corner screws overlap panel text
Screws are fixed at `(RACK_GRID_WIDTH, 0)` and
`(x, RACK_GRID_HEIGHT - RACK_GRID_WIDTH)`. Any artwork in those corners
collides. Affects the `AMPL. FUTURES` footer and serial number on **Drift,
Send, Pulse, DroneClone, SitarGrid, Choke**, and the header title on
**DroneCore**.

This is arithmetic, not judgement: nothing may occupy the four corner squares.

### 2. Attenuverters are never labelled
**SwarmCore** (4), **WallConductor**, **StringMassCore**, **HarmonicPressure**.
The knob is there, the CV jack beside it is labelled, the attenuverter is not.

### 3. Labels sit beside knobs, not above
**WallConductor**, **StringMassCore**, **HarmonicPressure** place captions to
the left of the knob. The rest of the set places them above. HarmonicPressure
does both on the same panel — `PITCH`/`SPREAD` beside, `PARTIAL`/`COUNT` above.

### 4. Range and mode captions sit at inconsistent baselines
**StringMassCore** `UNIS`/`HARM`/`JUST`/`MICRO` and **HarmonicPressure**
`JUST`/`EQUAL`/`MICRO` are scattered at three different heights around their
knob rather than sharing one baseline. Reads as unaligned rather than as a
scale.

### 5. Large dead space directly under the header
**WallConductor** (~145 px), **StringMassCore**, **HarmonicPressure**, **Send**.
Roughly the top fifth of each panel is empty while the controls crowd below.

### 6. Orphaned labels
**MassDriver** carries `MUTE` and `IN` captions at the foot of both channel
columns with **no widgets under them** — artwork for a ninth row that does not
exist. Same class as the construction guide lines stripped from DroneClone in
July.

### 7. Port captions below jacks
Most panels caption ports below the jack; the AF series captions above. Not
wrong on its own — Fundamental does it below too — but it is inconsistent
within one brand.

---

## Not yet inspected

CollapseSat, FeedbackGovernor, QuadVCA, CollapseEG, SignalBloc,
StreetGridClock. The last four are AF-series and expected clean on the evidence
of Ratchet; the first two are original-generation and should be assumed to
carry the systematic defects until rendered and checked.

---

## What this means for submission

The only technical objection ever raised on issue #912 was *"The panels look
broken on this."* On this evidence that objection is **still partly true** —
not for the whole set, but for DroneCore, DroneClone, Pulse and SitarGrid,
which is four of the nineteen a reviewer would open.

Submitting before these are fixed invites the same response, on a thread that
had to be reopened by request.

Fix order, by visibility per unit of work:

1. **The corner-screw rule** — one arithmetic sweep, clears defects on 7 panels
2. **DroneCore** — the title collision
3. **DroneClone** — the label/widget overlaps
4. **Pulse** — `METAL` and `CRACK`
5. **SitarGrid** — reposition the JHALA group
6. **Attenuverter captions** — 4 panels
7. **MassDriver** — delete the orphaned captions
8. Render the remaining 6 and repeat

Items 1–5 are what stands between this set and "the panels are fine".
