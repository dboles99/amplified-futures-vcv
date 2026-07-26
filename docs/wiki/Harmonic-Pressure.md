# Harmonic Pressure — 14HP

![Harmonic Pressure panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/HarmonicPressure.png)

Harmonic series pitch CV generator. Generates a polyphonic V/OCT output where each channel corresponds to one partial of the harmonic series above a root pitch. Designed as the primary tuning source for StringMassCore.

The module turns music theory into patch cable routing: each channel of its output is a pure harmonic partial, calculated exactly according to acoustic physics.

---

## Signal flow

```
PITCH (root V/OCT) ──► harmonic series calculation
PARTIAL (first) ──► starting partial number (1 = fundamental, 2 = octave, etc.)
COUNT (how many) ──► output channel count

for i in 0..COUNT:
    n = PARTIAL + i
    V/OCT[i] = PITCH + log2(n)        ← JUST mode (exact harmonic series)
             | round(V/OCT × 12) / 12  ← EQUAL mode (12-TET quantised)
             | + spectral micro offset  ← MICRO mode

SPREAD ──► per-partial ensemble colour (small cents offset, deterministic)

──► VOCT OUT (polyphonic, COUNT channels)
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| PITCH | −5 to +5V | Root pitch in V/OCT (0V = C4) |
| PARTIAL | 1–16 | First partial to output. 1 = fundamental, 2 = 1 octave up, 3 = perfect 5th + octave |
| COUNT | 1–16 | Number of partials output (= output channel count) |
| SPREAD | 0–1 | Ensemble colour — small per-partial detune for natural chorus |
| TUNING | JUST / EQUAL / MICRO | Tuning mode (switch) |

All knobs have attenuverter + CV.

---

## Tuning modes

| Mode | Description | Harmonic character |
|---|---|---|
| JUST | Exact harmonic series: partial n at root + log₂(n) V/OCT | Natural acoustic resonance, dissonant intervals in upper partials |
| EQUAL | Rounds each partial to nearest 12-TET semitone | More "chromatic cluster" sound, less acoustic resonance |
| MICRO | Adds deterministic spectral microtonality per partial | Most dissonant, quasi-noise in upper register |

---

## Partial → interval reference

Starting from root **C4** (PITCH = 0V):

| PARTIAL | Exact V/OCT | Note (root C) | Cents deviation from 12-TET | Musical character |
|---|---|---|---|---|
| 1 | 0.000 | C4 | 0¢ | Root — maximum gravity |
| 2 | 1.000 | C5 | 0¢ | Octave — still root character |
| 3 | 1.585 | G5 | +2¢ | Perfect 5th — open, powerful |
| 4 | 2.000 | C6 | 0¢ | 2nd octave |
| 5 | 2.322 | E6 | −14¢ | Major 3rd — warm, slightly flat |
| 6 | 2.585 | G6 | +2¢ | Perfect 5th again |
| 7 | 2.807 | Bb6 | −31¢ | Septimal minor 7th — "flat Bb", tense |
| 8 | 3.000 | C7 | 0¢ | 3rd octave |
| 9 | 3.170 | D7 | +4¢ | Major 2nd — bright |
| 10 | 3.322 | E7 | −14¢ | Major 3rd again |
| 11 | 3.459 | F#/Gb7 | −49¢ | 11th harmonic — between F and F#, no 12-TET equivalent |
| 12 | 3.585 | G7 | +2¢ | Perfect 5th |
| 13 | 3.700 | Ab7 | +41¢ | Between Ab and A — microtonal |
| 14 | 3.807 | Bb7 | −31¢ | Septimal m7 again |
| 15 | 3.907 | B7 | −12¢ | Major 7th, slightly flat |
| 16 | 4.000 | C8 | 0¢ | 4th octave |

> See **[[Music-Theory]]** for deeper explanation of these intervals and why they appear.

---

## PARTIAL as a performance control

This is the key insight of Harmonic Pressure: moving PARTIAL while a patch is running changes the *harmonic register* of everything connected downstream.

| PARTIAL setting | Tonal feeling |
|---|---|
| 1–2 | Root gravity — stable, fundamental |
| 3 | Skips octave/fundamental, begins on 5th — immediately tense |
| 5–7 | Upper harmonic territory — major 3rd, flat 7th, septimal tensions |
| 7–9 | High harmonic tension, microtonal territory begins |
| 11–13 | No 12-TET equivalent — pure microtonality, dissonant |
| 14–16 | Extreme upper series — almost noise-like at COUNT 8+ |

---

## Useful PARTIAL + COUNT combinations

| PARTIAL | COUNT | Result |
|---|---|---|
| 1 | 8 | Fundamental + 7 partials — classic harmonic cluster |
| 1 | 4 | Sparse low-harmonic presence |
| 3 | 6 | Skip fundamental, start on 5th — no "root gravity" |
| 7 | 6 | From septimal 7th up — intense microtonal region |
| 9 | 4 | High 2nd, 3rd, 4th, 5th — dense upper cluster |
| 1 | 16 | All 16 partials — maximum density |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| VOCT OUT | Output | Polyphonic V/OCT, COUNT channels |
| CV (×4) | Input | CV for PITCH, PARTIAL, COUNT, SPREAD |

---

## Patch tips

- **SPREAD at 0.3**: adds natural ensemble warmth without losing pitch identity.
- **PITCH CV from Drift** STEP output: stochastic root transpositions. Combine with slow COUNT modulation for evolving harmonic fields.
- **Feed VOCT OUT → StringMassCore AND DroneCore simultaneously**: DroneCore locks to individual partials while StringMassCore spreads voices within each section.
- **EQUAL mode** turns the harmonic series into chromatic clusters — more "wrong note" than "natural harmonic". Useful for noise-rock textures.

---

## Known pairings

| Module | Routing |
|---|---|
| [[String-Mass-Core]] | Primary V/OCT destination |
| [[DroneCore]] | V/OCT IN for per-partial dual-voice pairs |
| [[Drift]] | SMOOTH → PITCH CV for slow root transpositions |

---

## See also

[[String-Mass-Core]] · [[Music-Theory]] · [[Playbooks]] · [[DroneCore]]
