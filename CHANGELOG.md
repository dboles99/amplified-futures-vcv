# Changelog

Amplified Futures (`amplified-futures`) for VCV Rack 2.

Versions follow `MAJOR.MINOR.REVISION`, and MAJOR tracks the Rack major version
it is built for — so every release here is `2.x.y`.
See <https://vcvrack.com/manual/Manifest>.

## 2.3.0 — unreleased

Adds the four AF utility modules that have been on `master` since 2026-07-26 but
were never released, and fixes a module that has never made a sound.

### Added

- **Ratchet** (AF-03, 8 HP) — trigger burst generator. COUNT/SPREAD/PROB,
  subdividing the measured input interval.
- **Collapse EG** (AF-04, 8 HP) — attack/decay envelope with CURVE, MISFIRE and
  LOOP; ENV, INV and EOC outputs.
- **Quad VCA** (AF-05, 12 HP) — four-channel VCA and mixer with PRESSURE
  saturation and a selectable response curve.
- **Signal Bloc** (AF-06, 10 HP) — CV glue: two attenuvert+offset channels, a
  precision 3-input adder and a buffered 1→3 mult. All polyphonic.

### Fixed

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
