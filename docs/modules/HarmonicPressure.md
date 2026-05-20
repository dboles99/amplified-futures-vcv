# Harmonic Pressure — 14HP

Harmonic series pitch CV generator. Generates a polyphonic V/OCT output where each channel corresponds to one partial of the harmonic series above a root pitch. Designed as the tuning source for StringMassCore.

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

| Mode | Description |
|---|---|
| JUST | Exact harmonic series: partial n at root + log₂(n) V/OCT |
| EQUAL | Same as JUST but rounded to nearest 12-TET semitone |
| MICRO | Adds deterministic spectral microtonality offsets per partial |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| VOCT OUT | Output | Polyphonic V/OCT, COUNT channels |
| CV (×4) | Input | CV for PITCH, PARTIAL, COUNT, SPREAD |

---

## Patch tips

- **PARTIAL 1, COUNT 8** = fundamental + 7 partials. The classic harmonic partial cluster.
- **PARTIAL 3, COUNT 6** = skip fundamental and octave, start on 5th harmonic — immediately tense, no "root gravity".
- **EQUAL mode** turns the harmonic series into chromatic clusters — more "wrong note" than "natural harmonic". Useful for noise-rock textures.
- **SPREAD at 0.3**: adds natural ensemble warmth without losing pitch identity.
- **PITCH CV from Drift** STEP output: stochastic root transpositions. Combine with slow COUNT modulation for evolving harmonic fields.
- **Feed VOCT OUT → StringMassCore AND DroneCore simultaneously**: DroneCore locks to individual partials while StringMassCore spreads voices within each section.

---

## Known pairings

- **→ StringMassCore** V/OCT IN — primary use case
- **→ DroneCore** V/OCT IN for per-partial dual-voice pairs
- **← Drift** SMOOTH → PITCH CV for slow root transpositions
