# Music Theory Reference

Reference material for the harmonic, tuning, and frequency concepts used throughout the Amplified Futures modules. Covers both practical lookup tables and the underlying physics and music theory.

---

## Contents

1. [The Harmonic Series](#the-harmonic-series)
2. [V/OCT Standard and Frequency Reference](#voct-standard-and-frequency-reference)
3. [Partials → Intervals Table](#partials--intervals-table)
4. [Just Intonation vs Equal Temperament](#just-intonation-vs-equal-temperament)
5. [Ptolemaic Just Intonation (12-note scale)](#ptolemaic-just-intonation-12-note-scale)
6. [Odd Harmonic Series — StringMassCore HARM mode](#odd-harmonic-series--stringmasscore-harm-mode)
7. [Common Chords in V/OCT](#common-chords-in-voct)
8. [Quick Cheat Sheet](#quick-cheat-sheet)

---

## The Harmonic Series

When any pitched sound is produced — a plucked string, a blown pipe, an oscillator — the sound is not a single pure frequency. It is a *superposition* of multiple sine waves at integer multiples of the fundamental frequency. These are called **partials** or **overtones**.

A note at 110 Hz (A2) contains:

| Partial | Frequency | Interval above fundamental |
|---|---|---|
| 1 (fundamental) | 110 Hz | — |
| 2 | 220 Hz | Octave |
| 3 | 330 Hz | Octave + perfect 5th |
| 4 | 440 Hz | 2 octaves |
| 5 | 550 Hz | 2 octaves + major 3rd (slightly flat) |
| 6 | 660 Hz | 2 octaves + perfect 5th |
| 7 | 770 Hz | 2 octaves + flat minor 7th (not in 12-TET) |
| 8 | 880 Hz | 3 octaves |
| … | … | … |

**Why this matters:** The natural harmonic series defines which intervals sound consonant (partials align) or dissonant (partials clash). The tuning choices in HarmonicPressure and StringMassCore are grounded in this physics.

### The series is infinite — but useful content is in partials 1–16

The amplitude of each partial typically decreases with harmonic number. In electronic synthesis with HarmonicPressure, every partial has equal amplitude — this is more like a bright buzzing tone than a natural acoustic instrument, which is part of the no-wave aesthetic.

### Odd vs even harmonics

- **Odd harmonics** (1, 3, 5, 7, 9…): characteristic of square waves and some reed instruments. Sound brighter and more "hollow".
- **Even harmonics** (2, 4, 6, 8…): characteristic of triangle waves and bowed strings. Add octave doubling and "warmth".
- Both together: full harmonic content (saw wave, guitar).

StringMassCore's HARM mode uses **odd harmonics only** (sections at ratios 1, 3, 5, 7, 9, 11, 13, 15 — all odd).

---

## V/OCT Standard and Frequency Reference

V/OCT (volts per octave) is the standard pitch CV format in Eurorack/VCV Rack. The relationship is:

**f = f₀ × 2^(V/oct)**

where f₀ = 261.63 Hz (C4) at 0V.

### Octave reference

| Note | V/OCT | Frequency (Hz) |
|---|---|---|
| C0 | −4.000 | 16.35 |
| C1 | −3.000 | 32.70 |
| C2 | −2.000 | 65.41 |
| C3 | −1.000 | 130.81 |
| C4 | 0.000 | 261.63 |
| C5 | +1.000 | 523.25 |
| C6 | +2.000 | 1046.50 |
| C7 | +3.000 | 2093.00 |
| C8 | +4.000 | 4186.01 |

### Chromatic semitone reference (one octave from C4)

Each semitone = 1/12 V ≈ 83.3 mV.

| Note | Semitones from C4 | V/OCT | Frequency (Hz) |
|---|---|---|---|
| C4 | 0 | 0.0000 | 261.63 |
| C#4/Db4 | 1 | 0.0833 | 277.18 |
| D4 | 2 | 0.1667 | 293.66 |
| D#4/Eb4 | 3 | 0.2500 | 311.13 |
| E4 | 4 | 0.3333 | 329.63 |
| F4 | 5 | 0.4167 | 349.23 |
| F#4/Gb4 | 6 | 0.5000 | 369.99 |
| G4 | 7 | 0.5833 | 392.00 |
| G#4/Ab4 | 8 | 0.6667 | 415.30 |
| A4 | 9 | 0.7500 | 440.00 |
| A#4/Bb4 | 10 | 0.8333 | 466.16 |
| B4 | 11 | 0.9167 | 493.88 |
| C5 | 12 | 1.0000 | 523.25 |

### Reference frequencies

| Standard pitch | Note | Frequency |
|---|---|---|
| Concert A | A4 | 440.00 Hz |
| Middle C | C4 | 261.63 Hz |
| Low bass | C2 | 65.41 Hz |
| Human voice lower | E2 | 82.41 Hz |
| DroneCore min pitch | A2 | ~82 Hz |
| DroneCore max pitch | E6 | ~1319 Hz |

---

## Partials → Intervals Table

For **HarmonicPressure**: partial n at root pitch P gives output V/OCT = P + log₂(n).

For root = 0V (C4), the output pitches are:

| Partial (n) | V/OCT output | Note (root C) | 12-TET nearest | Cents deviation | Interval name | Consonance |
|---|---|---|---|---|---|---|
| 1 | 0.000 | C4 | C | 0¢ | Unison | Perfect |
| 2 | 1.000 | C5 | C | 0¢ | Octave | Perfect |
| 3 | 1.585 | G5 | G | +2¢ | Perfect 5th | Perfect |
| 4 | 2.000 | C6 | C | 0¢ | Octave × 2 | Perfect |
| 5 | 2.322 | E6 | E | −14¢ | Major 3rd | Consonant |
| 6 | 2.585 | G6 | G | +2¢ | Perfect 5th | Perfect |
| 7 | 2.807 | Bb6♭ | Bb | −31¢ | Septimal m7 | Dissonant* |
| 8 | 3.000 | C7 | C | 0¢ | Octave × 3 | Perfect |
| 9 | 3.170 | D7 | D | +4¢ | Major 2nd | Mild |
| 10 | 3.322 | E7 | E | −14¢ | Major 3rd | Consonant |
| 11 | 3.459 | F+7 | — | +49¢ above F | 11th harmonic | No 12-TET equiv. |
| 12 | 3.585 | G7 | G | +2¢ | Perfect 5th | Perfect |
| 13 | 3.700 | Ab+7 | — | +41¢ above Ab | 13th harmonic | No 12-TET equiv. |
| 14 | 3.807 | Bb♭7 | Bb | −31¢ | Septimal m7 | Dissonant* |
| 15 | 3.907 | B7 | B | −12¢ | Major 7th | Mild |
| 16 | 4.000 | C8 | C | 0¢ | Octave × 4 | Perfect |

\* The septimal minor 7th (7:4) is a "flat" Bb — 31 cents flatter than equal temperament. It is the characteristic interval of blues, jazz, and barbershop harmony. Acoustically, it produces zero beating with the fundamental. In EQUAL tuning mode, HarmonicPressure rounds it to the nearest 12-TET pitch.

**Intervals 11 and 13 have no equivalent in 12-tone equal temperament.** They fall between semitones and represent genuine microtonality. In JUST mode they appear at their exact acoustic frequencies; in EQUAL mode they are rounded to the nearest semitone, losing their character.

---

## Just Intonation vs Equal Temperament

### Equal Temperament (12-TET)
The standard Western tuning system. Each octave is divided into 12 equal semitones by multiplying frequency by 2^(1/12) ≈ 1.05946 per step. All intervals except the octave are slightly out of tune with the harmonic series — but the error is consistent and all keys are equally usable.

### Just Intonation (JI)
Intervals are defined as simple integer ratios — exactly the harmonic series. Intervals are *acoustically pure*: no beating between the harmonics of the two notes. But each JI scale is built from a specific root and different keys require retuning.

### Comparison of common intervals

| Interval | JI ratio | JI cents | 12-TET cents | Difference |
|---|---|---|---|---|
| Perfect unison | 1:1 | 0 | 0 | 0¢ |
| Minor 2nd | 16:15 | 112 | 100 | +12¢ |
| Major 2nd | 9:8 | 204 | 200 | +4¢ |
| Minor 3rd | 6:5 | 316 | 300 | +16¢ |
| Major 3rd | 5:4 | 386 | 400 | **−14¢** |
| Perfect 4th | 4:3 | 498 | 500 | −2¢ |
| Augmented 4th | 45:32 | 590 | 600 | −10¢ |
| Perfect 5th | 3:2 | 702 | 700 | **+2¢** |
| Minor 6th | 8:5 | 814 | 800 | +14¢ |
| Major 6th | 5:3 | 884 | 900 | −16¢ |
| Minor 7th | 9:5 | 1018 | 1000 | +18¢ |
| Septimal m7 | 7:4 | 969 | 1000 | **−31¢** |
| Major 7th | 15:8 | 1088 | 1100 | −12¢ |
| Octave | 2:1 | 1200 | 1200 | 0¢ |

The most audible differences are the **major 3rd** (14¢ flat in JI) and the **septimal m7** (31¢ flat). These are the tensions you hear when HarmonicPressure outputs partial 5 or 7.

### Why use JI in Amplified Futures?

JI tuning produces **harmonic locking** — when multiple voices are in JI relationships, their overtone series align and reinforce each other acoustically. This creates a richer, more resonant sound than equal temperament at the cost of being tied to a specific root.

In the context of massed oscillators (StringMassCore 16 voices, DroneClone 8 voices), this locking produces the characteristic "wall" resonance. With equal temperament the same mass of voices sounds more "electronic" and less cohesive.

---

## Ptolemaic Just Intonation (12-note scale)

StringMassCore's **JUST mode** uses a 12-note chromatic scale in Ptolemaic (5-limit) just intonation. These ratios use only the prime factors 2, 3, and 5 (no 7 — that's the septimal system).

| Degree | Ratio | Cents | Note from C | 12-TET note | Deviation |
|---|---|---|---|---|---|
| 1 | 1:1 | 0 | C | C | 0¢ |
| b2 | 16:15 | 112 | Db | C# | +12¢ |
| 2 | 9:8 | 204 | D | D | +4¢ |
| b3 | 6:5 | 316 | Eb | Eb | +16¢ |
| 3 | 5:4 | 386 | E | E | **−14¢** |
| 4 | 4:3 | 498 | F | F | −2¢ |
| b5 | 45:32 | 590 | F# | F# | −10¢ |
| 5 | 3:2 | 702 | G | G | +2¢ |
| b6 | 8:5 | 814 | Ab | Ab | +14¢ |
| 6 | 5:3 | 884 | A | A | −16¢ |
| b7 | 16:9 | 996 | Bb | Bb | −4¢ |
| 7 | 15:8 | 1088 | B | B | −12¢ |
| 8 | 2:1 | 1200 | C | C | 0¢ |

The most striking deviations from 12-TET are the major 3rd (E, −14¢), minor 3rd (Eb, +16¢), and major 6th (A, −16¢). These make JI chords sound "warmer" and more "in-tune" acoustically but slightly "off" compared to equal temperament expectations.

---

## Odd Harmonic Series — StringMassCore HARM mode

StringMassCore's HARM mode distributes 16 voices across 8 sections based on the **odd harmonics** (1, 3, 5, 7, 9, 11, 13, 15) folded into one octave.

| Section | Harmonic | Octave-folded ratio | Cents above root | Note from C | Musical interval |
|---|---|---|---|---|---|
| 1 | 1st | 1:1 | 0 | C | Unison |
| 2 | 3rd | 3:2 | 702 | G | Perfect 5th |
| 3 | 5th | 5:4 | 386 | E | Major 3rd |
| 4 | 7th | 7:4 | 969 | Bb♭ | Septimal minor 7th |
| 5 | 9th | 9:8 | 204 | D | Major 2nd |
| 6 | 11th | 11:8 | 551 | F+ | 11th harmonic (neutral 4th) |
| 7 | 13th | 13:8 | 841 | Ab+ | 13th harmonic (neutral 6th) |
| 8 | 15th | 15:8 | 1088 | B | Major 7th |

"Octave-folded" means dividing the harmonic down by 2 until it falls within one octave (1:1 to 2:1). For example, the 3rd harmonic (3:1 = two octaves + perfect 5th) becomes 3:2 (within one octave).

The chord implied by these 8 sections on root C is:
**C – D – E – G – Bb♭ – F+ – Ab+ – B**

This is not a standard Western chord — it contains:
- A perfect major triad (C, E, G)
- A septimal 7th (Bb♭, 31¢ flat of 12-TET Bb)
- The 11th harmonic F+ (49¢ above F, no 12-TET equivalent)
- The 13th harmonic Ab+ (41¢ above Ab, no 12-TET equivalent)
- A nearly-pure major 7th (B)

It is the **natural acoustic chord** — the chord that resonates above any bass note in nature. It is the chord of the harmonic series itself.

---

## Common Chords in V/OCT

Practical reference for patching multi-channel V/OCT sources. Root = 0V (C4) in all examples.

### Standard triads (12-TET)

| Chord | Intervals | V/OCT values | Notes |
|---|---|---|---|
| Major | R, M3, P5 | 0, 0.333, 0.583 | C–E–G |
| Minor | R, m3, P5 | 0, 0.250, 0.583 | C–Eb–G |
| Diminished | R, m3, d5 | 0, 0.250, 0.500 | C–Eb–Gb |
| Augmented | R, M3, A5 | 0, 0.333, 0.667 | C–E–G# |
| Sus2 | R, M2, P5 | 0, 0.167, 0.583 | C–D–G |
| Sus4 | R, P4, P5 | 0, 0.417, 0.583 | C–F–G |
| Power | R, P5 | 0, 0.583 | C–G |

### Extended (12-TET)

| Chord | Intervals | V/OCT values | Notes |
|---|---|---|---|
| Major 7 | R, M3, P5, M7 | 0, 0.333, 0.583, 0.917 | C–E–G–B |
| Dominant 7 | R, M3, P5, m7 | 0, 0.333, 0.583, 0.833 | C–E–G–Bb |
| Minor 7 | R, m3, P5, m7 | 0, 0.250, 0.583, 0.833 | C–Eb–G–Bb |
| Major add9 | R, M2, M3, P5 | 0, 0.167, 0.333, 0.583 | C–D–E–G |

### Just intonation chords

These use pure JI ratios, not 12-TET semitones. The differences are subtle but audible — JI chords have zero beating.

| Chord | Ratios | V/OCT values | Note |
|---|---|---|---|
| JI Major | 1:1, 5:4, 3:2 | 0, 0.322, 0.585 | M3 is 14¢ flatter than 12-TET |
| JI Minor | 1:1, 6:5, 3:2 | 0, 0.263, 0.585 | m3 is 16¢ sharper than 12-TET |
| Harmonic 7th | 1:1, 5:4, 3:2, 7:4 | 0, 0.322, 0.585, 0.807 | The "barbershop 7th" — 31¢ flat Bb |

### Microtonal clusters from HarmonicPressure

Using HarmonicPressure in JUST mode, root C4:

| PARTIAL | COUNT | Chord name | V/OCT cluster |
|---|---|---|---|
| 1 | 4 | Low harmonic tetrad | C4, C5, G5, C6 |
| 1 | 6 | Major-ish hexad | C4, C5, G5, C6, E6♭, G6 |
| 1 | 8 | Full octave cluster | C4–C5–G5–C6–E6♭–G6–Bb6♭♭–C7 |
| 3 | 4 | 5th-anchored cluster | G5, C6, E6♭, G6 |
| 5 | 4 | 3rd-anchored cluster | E6♭, G6, Bb6♭♭, C7 |
| 7 | 4 | Septimal cluster | Bb6♭♭, C7, D7, E7♭ |

---

## Quick Cheat Sheet

### V/OCT → Interval (one octave from C)

```
0.000  = C    (unison)
0.083  = C#
0.167  = D
0.250  = Eb   ← minor 3rd
0.333  = E    ← major 3rd (12-TET), JI major 3rd is 0.322
0.417  = F
0.500  = F#   ← tritone
0.583  = G    ← perfect 5th
0.667  = Ab
0.750  = A
0.833  = Bb   ← minor 7th
0.917  = B
1.000  = C    (octave)
```

### Partial → note from C (quick lookup)

```
P1  = C  (fundamental)
P2  = C  +1 octave
P3  = G  +2¢ (octave + P5)
P4  = C  +2 octaves
P5  = E  −14¢ (M3 + 2 oct)
P6  = G  +2¢ (P5 + 2 oct)
P7  = Bb −31¢ (flat! septimal m7)
P8  = C  +3 octaves
P9  = D  +4¢
P10 = E  −14¢
P11 = F+ +49¢ (no 12-TET equiv.)
P12 = G  +2¢
P13 = Ab +41¢ (no 12-TET equiv.)
P14 = Bb −31¢
P15 = B  −12¢
P16 = C  +4 octaves
```

### Key frequencies

```
A2  = 110.00 Hz  (DroneCore low)
E3  = 164.81 Hz
A3  = 220.00 Hz
C4  = 261.63 Hz  (middle C, 0V)
A4  = 440.00 Hz  (concert A)
C5  = 523.25 Hz
A5  = 880.00 Hz
C6  = 1046.50 Hz
```

### JI ratios vs 12-TET (in cents)

```
P5:  JI 702¢  vs 12-TET 700¢  → +2¢  (barely noticeable)
M3:  JI 386¢  vs 12-TET 400¢  → −14¢ (audible, warmer)
m3:  JI 316¢  vs 12-TET 300¢  → +16¢ (audible)
m7:  JI 969¢  vs 12-TET 1000¢ → −31¢ (dramatic, blues feel)
M7:  JI 1088¢ vs 12-TET 1100¢ → −12¢ (noticeable)
```

---

## Further reading

- The harmonic series: any standard acoustics textbook. *The Science of Sound* (Rossing/Moore) is thorough.
- Just intonation: Kyle Gann's *Just Intonation Explained* — accessible online introduction.
- Septimal harmony: Ben Johnston's microtonal notation system explores septimal intervals in a composerly context.
- Ptolemaic tuning: Harry Partch's *Genesis of a Music* — foundational text on JI practice.

---

*See also: [[Harmonic-Pressure]] · [[String-Mass-Core]] · [[DroneClone]] · [[Home]]*
