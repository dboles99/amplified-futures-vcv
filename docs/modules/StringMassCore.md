# String Mass Core — 16HP

16-voice harmonic mass oscillator. Generates dense polyphonic chord masses in four tuning modes. 1/√N amplitude normalised — adding more voices stays roughly the same loudness. Takes polyphonic V/OCT from HarmonicPressure or any pitch source.

---

## Signal flow

```text
V/OCT IN (poly) ──► per-channel, per-voice frequency calculation
                     │
                MODE ─┤
                     ├─ UNIS:  all M voices at V/OCT ± SPREAD/2 symmetric detune
                     ├─ HARM:  M voices distributed across 8 odd-harmonic sections
                     │         [1, 3/2, 5/4, 7/4, 9/8, 11/8, 13/8, 15/8]
                     ├─ JUST:  M voices mapped to Ptolemaic JI chromatic ratios (12-note)
                     └─ MICRO: per-voice slow vibrato at distinct rates (organic shimmer)

MASS ──► active voice count (1–16)
SPREAD ──► detune in cents within mode
TIMBRE ──► harmonic content: sine → odd-harmonic stack

sum ──► 1/√M normalise ──► tanh soft clip ──► OUT (poly, matches V/OCT channel count)
V/OCT IN ──────────────────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| MASS | 0 | 1–16 | 8 | Active voices per channel. Snap-enabled |
| MASS ATTEN | 1 | −1 to +1 | 0 | Attenuverter for MASS CV |
| SPREAD | 2 | 0–50¢ | 0 | Detune spread within section or around unison |
| SPREAD ATTEN | 3 | −1 to +1 | 0 | Attenuverter for SPREAD CV |
| TIMBRE | 4 | 0–1 | 0 | Harmonic blend: 0 = pure sine, 1 = odd-harmonic stack |
| TIMBRE ATTEN | 5 | −1 to +1 | 0 | Attenuverter for TIMBRE CV |
| MODE | 6 | 0–3 | 0 | 0=UNIS, 1=HARM, 2=JUST, 3=MICRO. Snap-enabled |
| SECTION | 7 | 1/2/4 | 1 | Harmonic section count (HARM mode only). Snap-enabled |

---

## Tuning modes

| Mode | Description |
| --- | --- |
| UNIS | All M voices at input pitch, symmetric ±SPREAD/2 cents detuning |
| HARM | M voices distributed across 8 odd-harmonic sections (1, 3/2, 5/4, 7/4, 9/8, 11/8, 13/8, 15/8 — octave reduced). SPREAD controls within-section detune |
| JUST | M voices across Ptolemaic just-intonation 12-note chromatic ratios. SPREAD adds ensemble colour |
| MICRO | All voices at fundamental with per-voice slow vibrato at distinct rates — organic ensemble shimmer |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| V/OCT IN | Input | CV / poly | Polyphonic (each channel = one chord/note) |
| MASS CV | Input | CV | CV for MASS (scaled by MASS ATTEN) |
| SPREAD CV | Input | CV | CV for SPREAD |
| TIMBRE CV | Input | CV | CV for TIMBRE |
| OUT | Output | Audio / poly | Normalised audio, channel count matches V/OCT IN |
| V/OCT THRU | Output | CV / poly | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | MASS | CC 14 |
| 2 | SPREAD | CC 15 |
| 4 | TIMBRE | CC 16 |
| 6 | MODE | CC 17 (0–31 = UNIS, 32–63 = HARM, 64–95 = JUST, 96–127 = MICRO) |

MASS and SECTION (index 7) are snap-enabled — CC values quantise to integer steps automatically.

---

## Recommended configurations

**Harmonic Partial Stack** — MODE HARM, MASS 8, SPREAD 8¢, TIMBRE 0.3. Feed 8 channels from HarmonicPressure (PARTIAL 1, COUNT 8). Each channel gets 8 voices spread across odd-harmonic sections. Dense but coherent.

**Just Intonation Drone** — MODE JUST, MASS 6, SPREAD 4¢, TIMBRE 0.1. Feed a single V/OCT (1 channel). 6 voices spread across Ptolemaic JI ratios — natural harmonic locking, warm and stable.

**Unison Chorus Wall** — MODE UNIS, MASS 4, SPREAD 12¢, TIMBRE 0. Drift SMOOTH → SPREAD CV (SPREAD ATTEN +0.4). The 4-voice unison slowly wavers between 4¢ and 20¢ spread, creating an organic chorus without using a dedicated chorus module.

**Spectral Shimmer** — MODE MICRO, MASS 12, SPREAD 20¢, TIMBRE 0.2. The per-voice vibrato rates are all distinct — the 12 voices shimmer together without ever locking into a repeating pattern.

---

## Basic setup — sound in 60 seconds

1. Add StringMassCore to your patch.
2. Patch HarmonicPressure VOCT OUT → StringMassCore V/OCT IN.
3. Patch StringMassCore OUT → WallConductor CH1 IN.
4. Set MODE to HARM, MASS to 8, SPREAD to 8¢, TIMBRE to 0.3.
5. HarmonicPressure: PARTIAL 1, COUNT 4, TUNING JUST.
6. You have a 4-channel × 8-voice = 32-voice harmonic partial stack.
7. Raise TIMBRE slowly to 0.6 — the odd harmonic content enters.

---

## How-tos

### Canonical HarmonicPressure pairing

- HarmonicPressure VOCT OUT (8 channels) → StringMassCore V/OCT IN.
- StringMassCore MODE HARM, MASS 6, SPREAD 8¢.
- Each of the 8 HarmonicPressure channels (partial 1 through 8) becomes a 6-voice section.
- Total simultaneous oscillators: 8 × 6 = 48.
- Route StringMassCore OUT → WallConductor CH1.

### SPREAD as living detune

- Patch Drift SMOOTH → SPREAD CV. SPREAD ATTEN +0.5.
- Set Drift RATE 0.08 Hz, WANDER 0.4, SLEW 0.15.
- The spread breathes between near-unison and wide chorus — the mass feels alive.
- Combine with MODE UNIS for maximum chorus effect.

### JUST mode for tonal drone

- MODE JUST, MASS 4, SPREAD 2¢, TIMBRE 0.05.
- Feed a single root pitch (one channel V/OCT).
- The 4 voices land on JI ratios: unison, minor second, major second, minor third, etc.
- Extremely stable, resonant harmonic locking. Good for long-form drone composition.

### TIMBRE sweep performance

- Map TIMBRE to MIDI CC 16.
- At CC 0: pure sine. At CC 64: moderate harmonic stack. At CC 127: dense buzzy odd harmonics.
- Sweeping TIMBRE from 0 → 1 during a performance transforms the texture from clean to dense.
- Follow with CollapseSat DRIVE at moderate level to colour the harmonic spectrum.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| COLLECTIVE (collective-refusal) | MASS 12–16 — maximum voice count, indistinct collective mass |
| PRESSURE (rent-pressure) | HARM mode + high TIMBRE — harmonic accumulation with specific ratio pressure |
| SPACE (static-witness) | JUST mode + low SPREAD + low TIMBRE — stable, witnessing presence in JI space |
| DAMAGE (managed-collapse) | SPREAD CV spike from Drift STEP — sudden detune injection as structural damage |
| CARE (mutual-aid) | UNIS mode + MASS 2, SPREAD 4¢ — two voices in close unison, mutually supportive |

---

## Known pairings

| Module | Role |
| --- | --- |
| HarmonicPressure | V/OCT IN — primary use case: per-partial voice masses |
| WallConductor | CH inputs as section sources |
| CollapseSat | Saturation of the normalised mass output |
| Drift | SMOOTH → SPREAD CV for slow detune breathing |
| Choke | Per-channel mixing when multiple StringMassCore instances are stacked |
