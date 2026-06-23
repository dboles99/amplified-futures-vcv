# Harmonic Pressure — 14HP

Polyphonic harmonic series V/OCT generator. Generates a polyphonic V/OCT output where each channel corresponds to one partial of the harmonic series above a root pitch. The tuning source for StringMassCore and any module that reads polyphonic V/OCT.

---

## Signal flow

```text
VOCT IN ──► root pitch (optional — or use PITCH knob alone)
PITCH ──► root pitch offset (−2 to +2 Oct)

for i = 0 to COUNT−1:
    n = PARTIAL + i
    V/OCT[i] = rootPitch + log2(n)          ← JUST mode (exact harmonic series)
             | round((root + log2(n)) × 12) / 12  ← EQUAL (12-TET quantised)
             | rootPitch + log2(n) + microOffset[i]  ← MICRO (JI + deterministic spread)

SPREAD ──► per-partial deterministic detune for ensemble colour (small cents offset)

──► VOCT OUT (polyphonic, COUNT channels)
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| PITCH | 0 | −2 to +2 Oct | 0 | Root pitch offset. 0 = C4 when no V/OCT IN connected |
| PITCH ATTEN | 1 | −1 to +1 | 0 | Attenuverter for PITCH CV |
| SPREAD | 2 | 0–1 | 0 | Per-partial ensemble detune — deterministic, small cents offset per channel |
| SPREAD ATTEN | 3 | −1 to +1 | 0 | Attenuverter for SPREAD CV |
| PARTIAL | 4 | 1–16 | 1 | First partial to output. 1 = fundamental, 2 = octave, 3 = perfect 5th above octave |
| COUNT | 5 | 1–16 | 8 | Number of partials to output (= polyphonic channel count) |
| TUNING | 6 | 0–2 | 0 | 0 = JUST, 1 = EQUAL, 2 = MICRO. Snap-enabled |

---

## Tuning modes

| Mode | Description |
| --- | --- |
| JUST | Exact harmonic series ratios: partial n at root + log₂(n) V/OCT |
| EQUAL | Same as JUST but each partial rounded to nearest 12-TET semitone |
| MICRO | JUST + deterministic per-partial intonation offsets (SPREAD controls magnitude) |

EQUAL mode quantises every partial to the chromatic scale, producing cluster chords instead of pure harmonic ratios. Useful for chromatic noise-rock textures where JI intervals feel too consonant.

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| V/OCT IN | Input | CV | Root pitch input (optional). Adds to PITCH knob offset |
| PITCH CV | Input | CV | CV for PITCH offset (scaled by PITCH ATTEN) |
| SPREAD CV | Input | CV | CV for SPREAD |
| VOCT OUT | Output | CV / poly | Harmonic series V/OCT, COUNT channels |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | PITCH | CC 14 |
| 2 | SPREAD | CC 15 |
| 4 | PARTIAL | CC 16 (0–7 = partial 1, 8–15 = partial 2, etc. — quantised in 8 steps of 127) |
| 5 | COUNT | CC 17 |
| 6 | TUNING | CC 18 (0–42 = JUST, 43–84 = EQUAL, 85–127 = MICRO) |

PARTIAL and COUNT are snap-enabled — CC values map to integer partial numbers automatically.

---

## Recommended configurations

**Standard Partial Cluster** — PARTIAL 1, COUNT 8, TUNING JUST, SPREAD 0. Outputs fundamental + 7 partials (1, 2, 3, 4, 5, 6, 7, 8). Feeds 8-channel polyphonic V/OCT into StringMassCore for the canonical AF harmonic stack.

**Upper Partial Tension** — PARTIAL 3, COUNT 6, TUNING JUST, SPREAD 0.2. Skips the fundamental and octave. Starts at the 3rd partial (perfect 5th above octave) — immediately tense, no tonal gravity from the root.

**Chromatic Cluster** — PARTIAL 1, COUNT 6, TUNING EQUAL, SPREAD 0. Every partial is rounded to the nearest semitone. Produces a chromatic cluster chord rather than a harmonic series. Works well for noise-rock harmonic tension.

**Microtonal Swarm** — PARTIAL 1, COUNT 12, TUNING MICRO, SPREAD 0.5. Each of 12 partials is JI-tuned with added ensemble spread. 12 channels into StringMassCore MODE HARM — a 12 × 8 = 96 voice mass.

---

## Basic setup — sound in 60 seconds

1. Add HarmonicPressure to your patch.
2. Patch a V/OCT source (keyboard, sequencer) → HarmonicPressure V/OCT IN.
3. Patch HarmonicPressure VOCT OUT → StringMassCore V/OCT IN.
4. Patch StringMassCore OUT → your audio output.
5. Set PARTIAL to 1, COUNT to 4, TUNING to JUST.
6. Play a note. You hear 4 harmonic partials through StringMassCore.
7. Raise COUNT to 8. The mass fills out.

---

## How-tos

### Skip the root for immediate tension

- Set PARTIAL to 3, COUNT to 5. TUNING JUST.
- The output starts at the 3rd partial (E above middle C if root is C).
- No fundamental, no octave — pure upper-register tension.
- Feed into StringMassCore MODE HARM. The resulting wall has no tonal anchor.

### PITCH root transposition

- Patch Drift STEP OUT → PITCH CV (PITCH ATTEN +0.5).
- Set Drift RATE 0.15, WANDER 0.6, SLEW 0.9 (instant jumps).
- Each Drift step shifts the root by a random ±0.5 Oct (±6 semitones at full atten).
- Combine with EQUAL tuning so transpositions stay on semitone boundaries.

### Dual StringMassCore feed

- Patch VOCT OUT → StringMassCore A V/OCT IN AND DroneCore V/OCT IN simultaneously.
- DroneCore locks to individual partials (each partial gets its own dual-voice pair).
- StringMassCore spreads additional voices within each partial's section.
- Route both to separate WallConductor channels for independent DENSITY control.

### SPREAD for ensemble warmth

- TUNING MICRO, SPREAD 0.3–0.5.
- Each partial is detuned by a deterministic offset — simulating 100 players each tuning the same pitch slightly differently.
- Very low SPREAD (0.1): just barely alive. High SPREAD (0.7): rich ensemble colour with slight pitch ambiguity.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| GRID (work-clock) | EQUAL tuning + fixed PARTIAL/COUNT — rigid harmonic grid |
| COLLECTIVE (collective-refusal) | COUNT 12–16 — maximum partial density, collective harmonic field |
| PRESSURE (rent-pressure) | Upper partials only (PARTIAL 5+) — high-register tension without resolution |
| SPACE (static-witness) | JUST mode, PARTIAL 1, COUNT 2 — two pure harmonic partials, wide open space between |
| CARE (mutual-aid) | JUST mode, low SPREAD, PARTIAL 1 + COUNT 4 — natural resonant consonance |

---

## Known pairings

| Module | Role |
| --- | --- |
| StringMassCore | V/OCT IN — primary use case |
| DroneCore | V/OCT IN for per-partial dual-voice pairs |
| DroneClone | V/OCT IN for harmonic series chord walls |
| Drift | SMOOTH → PITCH CV for slow root transpositions |
| Choke | Route different partial ranges to separate channels for layered mixing |
