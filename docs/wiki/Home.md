# Amplified Futures

[![Build](https://github.com/dboles99/amplified-futures-vcv/actions/workflows/build.yml/badge.svg)](https://github.com/dboles99/amplified-futures-vcv/actions/workflows/build.yml)
[![VCV Rack 2](https://img.shields.io/badge/VCV%20Rack-2-orange)](https://vcvrack.com)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://github.com/dboles99/amplified-futures-vcv/blob/master/LICENSE)

Nineteen VCV Rack 2 modules for dense experimental sound — massed oscillators, controlled feedback, no-wave rhythmics, microtonal pressure, modal string sequencing, bio-acoustic sampling, and a clock of its own. No-wave/noise-rock genealogy, built for live performance.

![Amplified Futures modules in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/contact-sheet.png)

---

## Three ideas behind every module

| Concept | What it means |
|---|---|
| **Mass** | Stack many voices, control them as one gesture (DENSITY, MASS knobs) |
| **Pressure** | Drive and saturate the sum, not individual channels (PRESSURE, BUZZ) |
| **Collapse** | A single gate deforms the entire sound with shaped recovery (COLLAPSE) |

Every module shares: CV + attenuverter on every CV-able knob, with the documented Swarm Core DECAY exception, plus V/OCT pass-through and the Amplified Futures panel language.

---

## Minimal patch — start here

1. **[[Harmonic-Pressure]]** — set COUNT to 8, MODE to JUST. Outputs a polyphonic harmonic series V/OCT.
2. **[[String-Mass-Core]]** — receive that poly V/OCT; MASS 12, SPREAD 0.3. You have a 12-voice just-intonated mass.
3. **[[Wall-Conductor]]** — feed the output in. Sweep DENSITY slowly. Hit COLLAPSE.
4. **[[Feedback-Governor]]** — insert between WallConductor's output and CH input for a controlled feedback tail.

From there: add **[[DroneClone]]** for a string-wall layer, **[[Drift]]** to modulate SPREAD or DENSITY, and **[[Collapse-Saturator]]** after for edge and buzz.

---

## Modules at a glance

| Module | HP | Category | Function |
|---|---|---|---|
| [[DroneCore]] | 8 | Oscillator | 2-voice detuned oscillator core |
| [[DroneClone]] | 22 | Oscillator | 8-voice amplified string wall |
| [[String-Mass-Core]] | 16 | Oscillator | 16-voice harmonic mass oscillator |
| [[Harmonic-Pressure]] | 14 | Pitch CV | Harmonic series V/OCT generator |
| [[Drift]] | 12 | Modulation | Slow random modulation source |
| [[Pulse]] | 12 | Percussion | 16-step no-wave percussion |
| [[Send]] | 12 | Routing | 2×2 cross-send feedback matrix |
| [[Choke]] | 18 | Mixer | 4-channel performance mixer |
| [[Wall-Conductor]] | 22 | Mixer | Section-based performance mixer |
| [[Mass-Driver]] | 32 | Mixer | 16-channel no-wave mixer (AF-01) |
| [[Street-Grid-Clock]] | 12 | Clock | Master clock with swing and brownout sag (AF-02) |
| [[Sitar-Grid]] | 42 | Sequencer | Modal string-resonance sequencer |
| [[Swarm-Core]] | 18 | Sampler | Bio-acoustic insect sample engine |
| [[Collapse-Saturator]] | 12 | Effect | Stereo drive with collapse |
| [[Feedback-Governor]] | 12 | Effect | Controlled feedback send/return |
| [[Collapse-EG]] | 8 | Utility | Attack/decay envelope with misfire and loop |
| [[Quad-VCA]] | 12 | Utility | Four-channel VCA and mixer |

---

## Wiki sections

- **[[Installation]]** — Get the plugin into Rack
- **[[Module-Reference]]** — All 19 modules with panel images and specs
- **[[Music-Theory]]** — Harmonic series, just intonation, V/OCT frequency reference, chord tables
- **[[Playbooks]]** — Named patch configurations for live performance
- **[[Design-System]]** — Panel language, colour system, HP grid
- **[[Building-from-Source]]** — Build and install instructions

---

*Daniel Boles / Amplified Futures — 2026*
