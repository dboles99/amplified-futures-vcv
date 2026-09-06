# Changelog

Amplified Futures (`amplified-futures`) for VCV Rack 2.

Versions follow `MAJOR.MINOR.REVISION`, and MAJOR tracks the Rack major version
it is built for — so every release here is `2.x.y`.
See <https://vcvrack.com/manual/Manifest>.

## 2.3.1 — 2026-09-06

The first release that macOS and Linux users can install. No new modules.

### Added

- **Builds for all four platforms.** `win-x64`, `lin-x64`, `mac-x64` and
  `mac-arm64`, attached to the release automatically. CI has compiled Linux and
  macOS since 2.3.0 and discarded the results every time — it ran `make`, never
  `make dist`, so 2.3.0 shipped a Windows build alone and most of the audience
  could not install at all. **mac-arm64 had never been built**, which on current
  hardware is most Mac users. Both Mac slices come from one runner — arm64
  natively, x64 cross-compiled by Rack's own `CROSS_COMPILE` path — and each
  binary is read back with `file` to confirm it is the architecture its
  filename claims, rather than trusting the name Rack generated.
- **Five presets, derived rather than chosen.**

  *Harmonic Pressure* gains **Branca Mass**, **Chatham 7-4** and **Raga Drone**
  — each the same three layers (table, drift rate, coherence) at different
  settings rather than a mode of its own, which is what the single architecture
  buys. All three declare drift rate and coherence explicitly; the three that
  shipped earlier stop at the tuning parameter and inherit whatever the defaults
  happen to be, which is not the same as declaring them. Branca Mass is authored
  in DRIFT tuning, not JUST: drift is scoped to DRIFT mode, so a JUST preset
  carrying a rate would sit still and the number would be decoration.

  *Drone Clone* gains **Disco Strings** at 17.5 cents, the midpoint of the
  10–25 cents measured as F0 dispersion across a large string section, and
  **Vinyl Wow** at 8.656 cents — a 0.5 mm off-centre pressing read at 100 mm
  radius. Vinyl Wow's drift *rate* is only approximated, because Drone Clone has
  no drift-rate control; the wiki says so rather than implying the figure was
  realised. Drone Clone cents throughout are adjacent-voice spacing, not the
  span of the stack: adjacent voices are what beat.

### Changed

- **Drone Clone reports SPREAD in Hz.** Cents do not tell you what you will
  hear — beat rate is linear in frequency, so the same detune shimmers in the
  bass and roughens in the treble. The right-click menu now gives the rate at
  the current pitch. The range is unchanged; altering it would change the sound
  of every saved patch. The figure is the rate between *adjacent* voices, not
  the extremes: at SPREAD 1.0 the outermost pair is 1200 cents apart, which is a
  different note rather than a beat.

## 2.3.0 — 2026-09-05

Adds the four AF utility modules that have been on `master` since 2026-07-26 but
were never released, and fixes a module that has never made a sound.

### Added

- **Swarm Core sample banks.** Two curated banks of 32 recordings ship in the
  repository — Cicadidae (23 species) and Orthoptera (9) — picked round-robin
  across species and stored as the mono 16-bit 44.1 kHz, 5-second form the
  loader always reduced them to. **27 MB in place of 324 MB**, and a build from
  source now has samples for the first time. The bank is chosen from the module
  context menu and stored in the patch by name, so adding or reordering banks
  cannot repoint an existing patch. InsectSet32 is CC BY 4.0; `ATTRIBUTION.md`
  ships beside the audio.
- **Harmonic Pressure drift.** DRIFT RATE and DRIFT COHERENCE, driving a real
  drift engine — coherence 0 transposes the whole stack together, 1 lets
  partials drift independently.


- **Ratchet** (AF-03, 8 HP) — trigger burst generator. COUNT/SPREAD/PROB,
  subdividing the measured input interval.
- **Collapse EG** (AF-04, 8 HP) — attack/decay envelope with CURVE, MISFIRE and
  LOOP; ENV, INV and EOC outputs.
- **Quad VCA** (AF-05, 12 HP) — four-channel VCA and mixer with PRESSURE
  saturation and a selectable response curve.
- **Signal Bloc** (AF-06, 10 HP) — CV glue: two attenuvert+offset channels, a
  precision 3-input adder and a buffered 1→3 mult. All polyphonic.

### Fixed

- **Swarm Core's WAV reader rejected or silently misdecoded most formats.**
  `WAVE_FORMAT_EXTENSIBLE` was refused outright — 60 of the 670 source
  recordings use it. 24-bit and 8-bit PCM fell through to the 16-bit branch, so
  they loaded, reported success and produced noise. **Zero channels divided by
  zero and took the process down**, so a malformed WAV in a user folder crashed
  Rack. Odd-sized RIFF chunks were skipped without their pad byte, leaving the
  reader a byte out of step so it never found the data. Now handles PCM
  8/16/24/32 and float 32/64, plain or extensible, under 26 assertions.
- **Swarm Core loaded 64 cicadas and nothing else.** The loader sorted paths and
  took the first 64; `cicadidae` sorts before `orthoptera`, so 294 orthoptera
  recordings were unreachable on a module whose only job is browsing a bank.
- **Bank switching freed samples the audio thread was reading** — a
  use-after-free, not a race you could hear. Loaded banks are now never freed
  while the module lives and are published through one atomic pointer.


- **Sitar Grid produced no sound at all.** `ksPluck` wrote its excitation burst
  to the delay-line span *ahead* of the write head without advancing it, so the
  write head overwrote the whole burst before the read head — a full lap behind —
  could reach it. Every audio output read exactly 0.0 V. `chiPluck` had the same
  defect, and the sympathetic bank was silent because it is fed from the main
  string. Present in every prior release.
- Drift was tagged `Modulation` and Quad VCA `VCA`. Neither is a Rack tag; they
  are now `Low-frequency oscillator` and `Voltage-controlled amplifier`.
  See <https://github.com/VCVRack/Rack/blob/v2/src/tag.cpp>.

### Changed

- **Ports carry direction by shape.** Inputs are square, outputs hexagonal.
  AF-IDS §10 forbids signalling direction by colour alone and every panel did
  exactly that; colour is now free to keep meaning signal *type*. Both graphics
  are drawn in PJ301M's 23.7 px box, so no panel coordinate moved.
- **BREAKING for saved patches: Harmonic Pressure tuning mode 2 was MICRO and
  is now DRIFT.** The parameter keeps its range and position, so patches load
  and every other control is unaffected — but a v2.2.0 patch set to mode 2 gets
  a time-varying drift where it used to get a static per-partial offset. Modes
  0 (JUST) and 1 (EQUAL) are unchanged.


- **DroneClone 22 -> 26 HP** and **Collapse Saturator 12 -> 16 HP.** Both were
  crowded past the point where labels could clear their neighbours. Widening
  changes a module's footprint, so adjacent modules in an existing patch shift
  right; parameters serialise by index, not position, so nothing is lost.
- **Collapse Saturator gains LEVEL, MIX and SC AMT.** Pre-gain reaches x10 with
  no output trim, saturation was fully wet with no parallel blend, and the
  sidechain depth was fixed at 0.5. Every default reproduces the previous
  behaviour, so an earlier patch sounds identical.
- **Swarm Core**: DENSITY drives an internal event clock when TRIG is unpatched,
  so the module sounds standalone; SPECIMEN gains an attenuverter and CV input;
  MODE latches instead of being momentary.
- **Panel titles.** Collapse Saturator and Collapse EG both printed `COLLAPSE`
  and were indistinguishable in the browser. Now `COLLAPSE SAT` and
  `COLLAPSE EG`. Slugs are unchanged - they are permanent.
- **Mounting bolts removed** from all 19 panels. Rack does not require them and
  they occupied the corners the footer and serial mark need.
- **Port labels follow the signal grammar**: Thru Green for inputs, Signal
  Yellow for outputs. 69 were wrong.
- Panels reflowed to use their full height (Send, Wall Conductor, Collapse
  Saturator), and every label/widget and label/label overlap on the set cleared.


- The Sitar Grid string model moved to `src/dsp/SitarStringCore.hpp` with no Rack
  dependency, covered by `tests/test_sitar_string.cpp` (19 assertions). Its RNG
  is seeded per instance, so two Sitar Grids no longer pluck identical noise.

## 2.2.0 — released

15 modules. Panel repairs across the set and Rack-rendered screenshots.

> `plugin.json` on `master` continued to read `2.2.0` after the four AF utility
> modules landed, so a 15-module build and a 19-module build both reported the
> same version. That is what this file exists to prevent: a version bump and a
> changelog entry are one step, not two.

## 2.0.0

14 modules. Rebuilt for Rack 2.

## 1.0.0

12 modules. First release.
