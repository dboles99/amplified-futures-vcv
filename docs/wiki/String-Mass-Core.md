# String Mass Core — 16 HP

![String Mass Core in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/StringMassCore.png)

Sixteen-voice harmonic mass oscillator. Where DroneClone shapes a wall by harmonic content, String Mass Core shapes it by *tuning* — MODE decides how the voices are distributed in pitch, from plain unison through odd-harmonic sections, Ptolemaic just intonation, and a microtonal mode where every voice drifts at its own rate. The sum is 1/√M normalised, so raising MASS adds density without adding level.

---

## Sound in 60 seconds

1. Add String Mass Core. Patch a V/OCT source into **V/OCT IN** and **OUT** to your interface.
2. It arrives in **HARM** mode with MASS 4, SPREAD 30% and TIMBRE 30% — hold a note and you already have four voices across two harmonic sections.
3. Turn **MODE** to UNIS. The four voices collapse onto one pitch and beat against each other.
4. Raise **MASS** towards 16. Density climbs; loudness does not.
5. Turn **MODE** to JUST. The voices redistribute onto just-intonation ratios and the mass locks into a chord.

---

## Signal flow

~~~text
V/OCT IN (poly) ──► [MODE decides how the M voices are tuned]
                     ├─ UNIS:  all M at input pitch, spread symmetrically
                     ├─ HARM:  M divided across SECTION harmonic sections
                     ├─ JUST:  M across Ptolemaic 12-note JI ratios
                     │          (spread narrowed to 30%)
                     └─ MICRO: all at fundamental, each voice given slow
                                vibrato at its own rate

                      [TIMBRE] blends sine → odd-harmonic stack
                              │
        sum of M voices ──► × 1/√M ──► tanh ──► OUT (poly)
V/OCT IN ──────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![String Mass Core panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/StringMassCore.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| MASS | 1–16 voices | 4 | Active voices per channel. Snaps to integers |
| SPREAD | 0–100% | 30% | Detune spread. Full scale is 50 cents across the voice set |
| TIMBRE | 0–100% | 30% | 0 is a pure sine; 100% is an odd-harmonic stack |
| MODE | UNIS / HARM / JUST / MICRO | **HARM** | How the voices are distributed in pitch. Snaps |
| SECTION | 1 / 2 / 4 sections | 2 | Harmonic section count — **HARM mode only**. Snaps |

MASS, SPREAD and TIMBRE each have an attenuverter (−1 to +1) and a CV input. MODE and SECTION are knob-only.

SPREAD reads as a percentage but the underlying range is 50 cents end to end, so 30% is roughly 15 cents of total spread. In JUST mode it is scaled to 30% of that, keeping the ratios legible.

### The four modes

| Mode | How the voices are tuned |
|---|---|
| **UNIS** | All voices at the input pitch, spread symmetrically across ±SPREAD |
| **HARM** | Voices divided evenly across SECTION harmonic sections, spread within each. The default |
| **JUST** | Voices across Ptolemaic just-intonation ratios, with spread narrowed so the intervals stay clear |
| **MICRO** | All voices at the fundamental, each given slow vibrato at its own distinct rate. Shimmer that never repeats |

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic pitch; sets the channel count |
| CV ×3 | Input | MASS, SPREAD and TIMBRE, each via its attenuverter |
| OUT | Output | Polyphonic mixed audio, 1/√M normalised then soft-clipped |
| V/OCT THRU | Output | Pass-through at the same channel count |

---

## Patch recipes

**Harmonic partial stack.** MODE HARM, SECTION 4, MASS 8, SPREAD 16%, TIMBRE 30%, fed eight channels from [[Harmonic-Pressure]] at PARTIAL 1, COUNT 8. Each channel gets eight voices across four sections — dense but coherent.

**Just intonation drone.** MODE JUST, MASS 6, SPREAD 8%, TIMBRE 10%, on a single V/OCT channel. Six voices on Ptolemaic ratios; they lock naturally and stay warm.

**Unison chorus wall.** MODE UNIS, MASS 4, SPREAD 25%, TIMBRE 0, with [[Drift]] SMOOTH → SPREAD CV at attenuverter +0.4. The spread wavers slowly — a chorus without a chorus module.

**Spectral shimmer.** MODE MICRO, MASS 12, SPREAD 40%, TIMBRE 20%. Every voice vibrates at its own rate, so the twelve never lock into a pattern.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Harmonic-Pressure]] | V/OCT OUT → V/OCT IN; the intended pitch source |
| [[Wall-Conductor]] | OUT → channel input, for DENSITY and COLLAPSE over the mass |
| [[Drift]] | SMOOTH → SPREAD or MASS CV for slow structural movement |
| [[Collapse-Saturator]] | OUT → IN to add edge to an otherwise clean mass |
| [[DroneClone]] | Run alongside — tuning-shaped mass against harmonics-shaped wall |

---

## See also

[[DroneCore]] · [[DroneClone]] · [[Harmonic-Pressure]] · [[Music-Theory]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/StringMassCore.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/StringMassCore.md)
