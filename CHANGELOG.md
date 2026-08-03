# Changelog

All notable changes to **Amplified Futures** (slug `amplified-futures`) for
VCV Rack 2.

Format follows [Keep a Changelog](https://keepachangelog.com/en/1.1.0/); this
project uses [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

**The plugin slug is permanent.** Changing it breaks every saved patch, and
VCV library issue #912 was submitted against it.

**Parameter, input and output enums are only ever appended to.** Rack
serialises by index, so inserting mid-list silently rewires every saved patch.
Panel layout may change freely — position is not serialised.

---

## [2.3.0] — unreleased

Four modules that complete the AF utility series. They have been in `main`
since 2026-07-26 and have never been released.

### Added

- **Ratchet (AF-03)** — 8 HP. Trigger burst generator; COUNT / SPREAD / PROB,
  subdivides the measured input interval. 26 core assertions.
- **Collapse EG (AF-04)** — 8 HP. Attack/decay envelope; CURVE, MISFIRE, LOOP,
  ENV/INV/EOC outputs. 19 core assertions.
- **Quad VCA (AF-05)** — 12 HP. Four-channel VCA and mixer; PRESSURE
  saturation, selectable response curve. 15 core assertions.
- **Signal Bloc (AF-06)** — 10 HP. CV glue: 2× attenuvert+offset, precision
  3-input adder, buffered 1→3 mult, all polyphonic. 13 core assertions.
- CI now runs the pure-logic core suites on every build.
- Six release-readiness gates, and the three manifest defects they caught.

### Fixed

- **Panel artwork no longer runs under the mounting screws.** Rack draws the
  four corner screws over the panel, and six panels had put artwork there.
  DroneCore's own title lost its leading `D` and trailing `E`; `AMPL. FUTURES`
  read `AM_ FUTURES` on Drift, Pulse, Send and DroneClone. Five panels also
  drew their counter-holes at the screw's *position* rather than its *centre*,
  half a screw width out, showing as a dark dot beside every screw.
- **DroneCore's panel title is now `DRNCR`.** `DRONECORE` needed 75.7 px of the
  60 px that an 8 HP panel leaves clear between its screws, so it never fitted.
  Devowelling freed enough width to set it at 16 px rather than 13 px. The
  module name in the browser is unchanged.
- **39 captions lifted out from under the widgets drawn over them.** This is the
  defect that lost the July submission — label paths at the same y as a widget
  centre — and eight panels still had it. Feedback Governor hid `KILL`
  completely; Pulse hid all four `ATN` captions; DroneCore hid six; Send hid the
  `ATN` row and clipped `V/OCT THRU`.
- Swarm Core: widen the WAV sample count before allocating. `nFrames * numCh`
  was computed in `uint32_t` and only widened afterwards, so a large enough
  frame count wrapped before reaching the vector constructor and `fread`.

### Known remaining

Recorded rather than quietly carried. Full detail in
`docs/qa/panel-audit-2026-08-03.md`.

- **SitarGrid** — the six JHALA knobs render above their own
  `JHALA BREAKDOWN` header, inside the neighbouring `GLOBAL` section.
- **SwarmCore** — four unlabelled trimpots; `V/OCT` sits diagonally below-right
  of its jack instead of above it.
- **MassDriver** — `MUTE` and `IN` captions at the foot of both channel columns
  with no widgets under them.
- **WallConductor, StringMassCore, HarmonicPressure** — attenuverters carry no
  caption, main labels sit beside their knobs rather than above, and roughly the
  top fifth of each panel is empty.
- **Send** — `FEEDBACK` is clipped at the right panel edge.

### Notes

AF-06 is 10 HP, not the 8 HP originally specced. Sixteen widgets need 344 px of
a 329 px panel at 8 HP; building with every clearance at its floor is what
caused the AF-04 collision.

---

## ⚠ Version note, recorded 2026-08-03

**The released `v2.2.0` contains 15 modules. This working tree contains 19.**

`plugin.json` was left reading `2.2.0` when Ratchet, Collapse EG, Quad VCA and
Signal Bloc were added, so two different plugins reported the same version: the
one users can download, and the one that builds from source. Anyone comparing
them by version string would conclude they were identical.

This changelog is being written for the first time today, and writing it is what
surfaced it — there was no other artifact in the repository where the two
numbers had to sit next to each other. `plugin.json` is bumped to `2.3.0` in the
same change.

Related: no `v2.1.0` tag exists, though commit `5cf1678` sets the version string
to 2.1.0 and the backlog refers to a v2.1.0 release. The panel-repair work that
commit describes shipped inside `v2.2.0`.

---

## [2.2.0] — 2026-07-26

15 modules.

### Added

- **Street Grid Clock (AF-02)** — 12 HP. Master clock; RATE / SWING / BROWNOUT,
  CLK //2 //4 //8, external clock input. Brownout sag, reset correctness.
  20 core assertions — the first automated tests any module in this plugin had.
- Factory presets: *Grid Steady*, *Brownout District*.
- `docs/wiki/` as the generated source for the GitHub wiki, with
  `tools/sync_wiki.ps1` (one-way publish) and `tools/check_wiki.py` (links, HP
  against the SVGs, module coverage, image URLs).

### Changed

- Choke and Swarm Core widened 14 → 18 HP.
- Swarm Core gained CV in and CV out; I/O layout rebuilt.
- All 14 existing panels re-rendered and re-scored against the panel rubric;
  10 were repaired.
- All SVG root dimensions set in mm.

### Fixed

- DroneClone: clipped band labels removed, V/OCT jack label cleared, knob
  labels lifted clear — measured rather than guessed.
- SitarGrid: all 15 bottom jacks labelled, knob sub-labels cleared.
- Mass Driver: labels lifted clear, clipped V/OCT output rescued.
- String Mass Core and four other panels: labels lifted clear of their widgets.

---

## [2.0.0] — 2026-06-24

14 modules.

### Added

- **Sitar Grid** — 42 HP. Modal string-resonance sequencer; three brains,
  jawari bridge, JHALA breakdown.
- **Swarm Core** — 18 HP. Insect sample engine (InsectSet32, CC-BY 4.0);
  Specimen and Swarm modes.
- CONTRIBUTING, SECURITY and a Contributor Covenant code of conduct.
- CI, release, licence and VCV Rack 2 badges; Dependabot for GitHub Actions.

### Fixed

- **Panel text converted to bezier paths on Sitar Grid and Swarm Core.**
  nanosvg does not render `<text>` with external fonts, so those labels were
  invisible in Rack.
- Collapse Saturator: `COLPSE` typo on the COLLAPSE gate jack label → `GATE`.

---

## [1.0.0] — 2026-05-20

First release. 12 modules, submitted to the VCV library as issue #912.

### Added

DroneCore · DroneClone · Send · Choke · Pulse · Drift · Harmonic Pressure ·
String Mass Core · Collapse Saturator · Feedback Governor · Wall Conductor ·
Mass Driver.

---

## Distribution

The VCV library issue (#912) was **closed and locked on 2026-07-25**, over
AI-drafted comments on the thread rather than over the code. The plugin is not
listed and `library.vcvrack.com/?brand=Amplified+Futures` returns nothing.

Releases are therefore published on GitHub. Windows x64 `.vcvplugin` is
attached to each release; macOS and Linux build from source.

**Release protocol**

1. Bump `version` in `plugin.json` **and add the section here**.
2. Push, wait for CI green.
3. `git tag v<version> && git push origin v<version>`
4. `make dist` under MSYS2, then
   `gh release create v<version> dist/*.vcvplugin --notes-file CHANGELOG.md`

Step 1 is two edits, not one. Doing only the first is what produced the 2.2.0
collision above.

[2.3.0]: https://github.com/dboles99/amplified-futures-vcv/compare/v2.2.0...HEAD
[2.2.0]: https://github.com/dboles99/amplified-futures-vcv/compare/v2.0.0...v2.2.0
[2.0.0]: https://github.com/dboles99/amplified-futures-vcv/compare/v1.0.0...v2.0.0
[1.0.0]: https://github.com/dboles99/amplified-futures-vcv/releases/tag/v1.0.0
