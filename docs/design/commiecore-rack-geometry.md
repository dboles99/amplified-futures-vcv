# Commiecore for Rack — panel geometry spec (v2.2.1)

Status: authoritative for the v2.2.1 redesign. Every panel in `res/` is drawn
against this document. If a panel and this document disagree, the panel is wrong.

The July 2026 failure had one root cause: **panels were authored by reading SVG
sources and never by looking at what Rack drew.** Labels sat at widget-centre y,
so Rack composited every knob over its own caption. This spec exists so that the
clearances are arithmetic, decided in writing, before anything is drawn.

---

## 1. The conversion, exactly

Rack renders SVG at **SVG_DPI = 75**.

```
1 mm  = 75 / 25.4        = 2.952756 px
1 HP  = 5.08 mm          = 15.000 px      (exact)
panel = 128.5 mm tall    = 379.43 px  →  viewBox height 380
```

> **Correction to CLAUDE.md.** The workspace notes say `1 mm = 3 px`. That is
> wrong by 1.5%. Across a full 128.5 mm panel it accumulates to **5.5 px** of
> drift. This is not academic: it is why MassDriver's V/OCT output was placed at
> y = 126 mm on a 128.5 mm panel and physically hung off the bottom edge. Use
> **2.952756**, or better, place widgets with Rack's own `mm2px()` and never
> hand-convert.

SVG document attributes, non-negotiable (per the VCV panel manual):

```xml
<svg width="{HP × 5.08}mm" height="128.5mm" viewBox="0 0 {HP × 15} 380">
```

The `width`/`height` attributes carry **mm units**. The `viewBox` is unit-less
px. Both must be present and must agree.

---

## 2. Widths — every module gets wider

The brief is more air, not more controls. Widening is also the standing lesson
from AF-06 (8 HP could not hold sixteen widgets) and from Choke and SwarmCore
(both widened 14 → 18 HP in July because the knob-to-jack clearance was
physically unsatisfiable at the old width).

| Slug | v2.2.0 HP | **v2.2.1 HP** | Δ | new width (mm) | viewBox width (px) |
|---|---|---|---|---|---|
| CollapseEG | 8 | **12** | +4 | 60.96 | 180 |
| DroneCore | 8 | **12** | +4 | 60.96 | 180 |
| Ratchet | 8 | **12** | +4 | 60.96 | 180 |
| SignalBloc | 10 | **14** | +4 | 71.12 | 210 |
| CollapseSat | 12 | **16** | +4 | 81.28 | 240 |
| Drift | 12 | **16** | +4 | 81.28 | 240 |
| FeedbackGovernor | 12 | **16** | +4 | 81.28 | 240 |
| Pulse | 12 | **16** | +4 | 81.28 | 240 |
| QuadVCA | 12 | **16** | +4 | 81.28 | 240 |
| Send | 12 | **16** | +4 | 81.28 | 240 |
| StreetGridClock | 12 | **16** | +4 | 81.28 | 240 |
| HarmonicPressure | 14 | **18** | +4 | 91.44 | 270 |
| StringMassCore | 16 | **20** | +4 | 101.60 | 300 |
| Choke | 18 | **22** | +4 | 111.76 | 330 |
| SwarmCore | 18 | **22** | +4 | 111.76 | 330 |
| DroneClone | 22 | **26** | +4 | 132.08 | 390 |
| WallConductor | 22 | **26** | +4 | 132.08 | 390 |
| MassDriver | 32 | **36** | +4 | 182.88 | 540 |
| SitarGrid | 42 | **46** | +4 | 233.68 | 690 |

**Total: 302 HP → 378 HP.**

A uniform +4 HP keeps the set coherent — every module gains the same physical
breathing room, so the design grammar reads identically at 12 HP and at 46 HP.
The narrow modules gain proportionally most, which is correct: they were the
cramped ones.

Widening costs the user rack space. That is the trade being made deliberately:
the previous set was rejected on legibility, and legibility is bought with area.

---

## 3. The three zones

Reading order is top to bottom: **what the module is doing** → **what you set** →
**what you patch**. Ports at the bottom means cables hang below the controls and
never cross them.

```
 y (mm)   zone
┌──────────────────────────────────────────────┐
│  0.0                                          │
│         MASTHEAD          module name, AF mark │
│ 11.0 ─────────────────────────────────────────│
│                                                │
│ 13.5    DISPLAY           scope / spectral     │
│                           phosphor window      │
│ 39.5 ─────────────────────────────────────────│
│                                                │
│ 44.0    CONTROL FIELD     knobs, sliders,      │
│                           switches             │
│                                                │
│ 97.0 ─────────────────────────────────────────│
│                                                │
│102.0    PORT RAIL         labelled glyph ports │
│                                                │
│122.0 ─────────────────────────────────────────│
│         foot                                   │
│128.5                                           │
└──────────────────────────────────────────────┘
```

| Zone | y range (mm) | height | rule |
|---|---|---|---|
| Masthead | 0 – 11.0 | 11.0 | Module name only. Drawn in **NanoVG from C++**, never SVG text. |
| *gap* | 11.0 – 13.5 | 2.5 | negative space, always empty |
| Display | 13.5 – 39.5 | 26.0 | Phosphor window. Full width less side margin. |
| *gap* | 39.5 – 44.0 | 4.5 | negative space, always empty |
| Control field | 44.0 – 97.0 | 53.0 | Knobs, sliders, switches, their labels and satellites |
| *gap* | 97.0 – 102.0 | 5.0 | negative space, always empty |
| Port rail | 102.0 – 122.0 | 20.0 | Jacks and their labels only. Nothing else. |
| Foot | 122.0 – 128.5 | 6.5 | negative space, always empty |

**18.5 mm of the 128.5 mm panel — 14% — is mandated empty.** Those gaps are not
slack to be borrowed when a layout gets tight. A module that does not fit inside
its zones gets wider, never denser. That rule is the whole point.

Side margin: **5.0 mm** on both edges at every width. Nothing but the panel
background may enter it.

---

## 4. Clearances

Derived from the widget radii Rack actually uses, plus the legibility rubric.

| Pair | minimum centre-to-centre | note |
|---|---|---|
| Knob ↔ its CV jack | **10.09 mm** | the July Choke failure was 8.6 mm |
| Knob ↔ knob | 12.0 mm | |
| Jack ↔ jack | 9.0 mm | |
| Trimpot ↔ jack | 8.0 mm | |
| Widget edge ↔ panel edge | 5.0 mm | the side margin |
| Widget edge ↔ zone boundary | 1.5 mm | |

**Label rule, absolute:** a label's baseline sits **above** its widget, never at
widget-centre y. Minimum clearance from label baseline to widget top edge:
**1.2 mm**. Port labels sit above the jack, inside the port rail.

This single rule is what broke ten of fourteen panels in July. It is checked
mechanically by `/panel-check` and visually on the Rack render.

---

## 5. Text

Zero `<text>` elements ship. Ever.

- **Module name:** drawn in C++ via NanoVG in `draw()`. Not in the SVG at all.
- **Every other label:** authored as SVG text, then converted to bezier paths
  with Inkscape (`Path > Object to Path`) before commit.
- Verification: `grep -l "<text" res/*.svg` must return nothing.

The VCV manual is explicit that the renderer supports neither text nor fonts.

---

## 6. Palette

Ported from `af-ui/include/af-ui/CommiecoreTokens.hpp` so the Rack panels and the
JUCE products are the same instrument family.

| Role | Token | Hex |
|---|---|---|
| Panel surface | `kSurface` | `#111410` |
| Raised control face | `kRaised` | `#1A1F16` |
| Recessed well | `kWell` | `#0C0F0A` |
| Display ground | `kPhosphorBg` | `#060807` |
| Display trace | `kPhosphor` | `#39FF14` |
| Display bloom | `kPhosphorGlow` | `#0E2010` |
| Primary label | `kCream` | `#E8E4D4` |
| Secondary label | `kSteel` | `#8A9080` |
| Dim / range label | `kMutedOlive` | `#6B7A58` |
| Section identity | `kStructureOrange` | `#C8661A` |
| Masthead / brand | `kSafetyOrange` | `#E87D00` |
| Structural rule | `kBorder` | `#1C2318` |
| Faint separator | `kBorderDeep` | `#242B1E` |

Port stroke colours keep their existing signal semantics:

| Port kind | stroke | token |
|---|---|---|
| Audio | `#C8661A` | `kStructureOrange` |
| CV / modulation | `#C8B84A` | `kFadedYellow` |
| Pitch (V/OCT) | `#C8DBC0` | `kPaleJade` |
| Clock / trigger | `#C8B84A` | `kSignalClock` |
| Gate | `#6BFF2A` | `kAcidPunk` |

Only flat fills and simple two-stop linear gradients. No filters, no masks, no
`<use>`, no CSS — the manual lists all of these as unsupported.

---

## 7. Display zone contract

The display is a `TransparentWidget` subclass drawing into the phosphor window
from a lock-free ring buffer the module writes in `process()`.

- Buffer: 512 frames, written at a decimated rate, never allocated in `process()`
- The module owns the buffer; the widget only reads
- Modes per module: `SCOPE` (time domain), `SPECTRUM` (magnitude), `METER`
  (level/activity). A module declares one; it is not user-switchable in v2.2.1.
- If a module has nothing meaningful to show, the window still exists and draws
  the phosphor ground plus its grid. **The zone is structural, not optional** —
  it is what makes nineteen different modules read as one instrument family.

---

## 8. Polyphony and attenuverters

Attenuverters are hidden on channels carrying a polyphonic cable, toggled from
the module's right-click menu (`appendContextMenu`). The parameter itself is
never removed and its enum position never changes — only the widget's visibility.

Serialisation rule stands: **new params/inputs/outputs are appended, never
inserted.** Widget coordinates may move freely; enum order may not. That is what
keeps every saved patch working across this redesign.

---

## 9. Verification gate

A panel is not done until all of these pass. Rendering is the arbiter, not the
source.

1. `grep -l "<text" res/*.svg` → empty
2. `/panel-check --all` → clean
3. viewBox width ÷ 15 = the HP in §2, exactly
4. `width`/`height` attributes present, in mm, agreeing with the viewBox
5. Rendered via `Rack.exe -u <clean dir> -t 2`, at `HP × 15 × 2` by 760 px
6. Scored on the **rendered PNG** against `docs/qa/panel-rubric-2026-07-25.md`
7. Every port and control legible; no widget overlapping its own label
8. `make test` → all core assertions pass

### 9.1 The 100% clause

Nothing ships on a partial pass. For **every** module, on the Rack render and in
a live Rack session:

| Element | "works" means | "is seen" means |
|---|---|---|
| Ports | accepts a cable, carries signal, poly where declared | jack fully on-panel, label legible above it, stroke colour correct for its signal kind |
| Knobs | sweeps its full declared range, CV and attenuverter track | body unobstructed, label legible above it, value range marks readable |
| Sliders | sweeps full travel, no clipped end stops | track and cap fully on-panel, label legible |
| Info screens | draws from live buffer, no stale or frozen frame | window fully inside the display zone, trace visible against phosphor ground |
| Presets | every `presets/<Slug>/*.vcvm` loads and restores identical param values | — |

The preset check is mechanical: load each `.vcvm` against the rebuilt module and
diff the resulting param vector against the file. Any divergence means enum order
moved, which is a release blocker, not a warning.

**A partial pass is a fail.** The July submission was lost to claiming a fix that
the render did not support. Every row above is confirmed against a rendered PNG
or a running Rack instance — never against the SVG source.

---

## 10. Labelling — the ladder

Every port, knob and slider carries a visible label. No exceptions, at any width.
When a name does not fit the space, it is **compressed, never omitted**.

Descend this ladder only as far as necessary. Stop at the first rung that fits
the available width at the zone's type size. Never skip a rung to reach for a
glyph that looks better.

| # | Strategy | Example | When |
|---|---|---|---|
| 1 | **Full word** | `DENSITY` | fits — always preferred |
| 2 | **Conventional abbreviation** | `FREQ` `RES` `AMT` `MOD` `SYNC` | an abbreviation the synth world already reads without thinking |
| 3 | **Devowelled** | `PRESSURE` → `PRSSR`, `COLLAPSE` → `CLLPS`, `RECOVERY` → `RCVRY` | consonant skeleton survives; drop interior vowels only, keep the leading letter |
| 4 | **Initialism** | `FEEDBACK GOVERNOR` → `FG`, `V/OCT` | multi-word names only |
| 5 | **Glyph** | ⎍ clock · ⌁ trigger · ∿ audio · ⊳ output · ⊲ input | universal concepts with an unambiguous symbol |
| 6 | **Glyph + micro-label** | ∿ with `L` beneath | when the glyph alone is ambiguous between channels |

### Rules

- **Devowelling keeps the first letter and all consonants.** `PRSSR` not `PRSR`.
  A reader recovers the word from the skeleton; break the skeleton and they can't.
- **A glyph is only allowed where it is genuinely universal.** Invented symbols
  are worse than devowelled text — the reader has no key. Glyphs come from
  `AFGlyphs.hpp`, which is the shared vocabulary across all AF products.
- **Rung 5 or 6 requires the glyph to also appear in the module's wiki page**,
  with its meaning spelled out. A symbol with no published key is not a label.
- **If nothing on the ladder fits, the panel is too narrow.** Widen it. This is
  the standing rule of the whole redesign and it overrides every layout instinct
  to compress. §2 already gave every module +4 HP for exactly this reason.

### The backstop

Panel text is the primary label, never the only one. Independently of the ladder:

- Every param has a full, human name in `configParam()`
- Every input has one in `configInput()`
- Every output has one in `configOutput()`

Rack surfaces these on hover, so a devowelled `PRSSR` on the panel always resolves
to `Pressure` in the tooltip. This costs zero panel area and it is what makes
aggressive compression safe. **A module with an unnamed param or port fails the
gate even if its panel looks perfect** — verify with a grep for `configParam`,
`configInput` and `configOutput` counts matching `PARAMS_LEN`, `INPUTS_LEN` and
`OUTPUTS_LEN`.
