# String Mass Core — 16HP

![String Mass Core panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/StringMassCore.png)

16-voice harmonic mass oscillator. The synthesis engine for the Amplified Futures harmonic series — generates dense polyphonic chord masses in four tuning modes. 1/√N amplitude normalised. Takes polyphonic V/OCT from HarmonicPressure or any pitch source.

---

## Signal flow

```
V/OCT IN (poly) ──► per-channel, per-voice frequency calculation
                    ├─ UNIS: all voices at V/OCT ± SPREAD detune
                    ├─ HARM: voices assigned to 8 odd-harmonic sections
                    ├─ JUST: voices on Ptolemaic JI ratios (12-note)
                    └─ MICRO: spectral microtonality (per-partial spread)

MASS ──► active voice count (1–16)
SPREAD ──► detune in cents (0–100¢) within each section
TIMBRE ──► harmonic content (sine → odd harmonic stack)

sum ──► 1/√N normalise ──► tanh soft clip ──► OUT (poly)
V/OCT IN ────────────────────────────────────► V/OCT THRU
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| MASS | 1–16 | Active voices per channel. More = denser, normalised amplitude |
| SPREAD | 0–100¢ | Detune spread within section or around unison |
| TIMBRE | 0–1 | Harmonic blend: 0 = pure sine, 1 = odd-harmonic stack |
| MODE | UNIS / HARM / JUST / MICRO | Tuning mode (switch) |

All knobs have attenuverter + CV.

---

## Modes

### UNIS — Unison
All voices at the input pitch ± SPREAD/2 symmetric detuning. The simplest mode — a thick chorus. Useful for drone walls where pitch identity matters more than harmonic structure.

### HARM — Odd Harmonic Sections
16 voices distributed across 8 sections based on the odd harmonic series (partials 1, 3, 5, 7, 9, 11, 13, 15 divided into one octave):

| Section | Ratio | Interval above root | Frequency multiplier |
|---|---|---|---|
| 1 | 1:1 | Unison | 1.000 |
| 2 | 3:2 | Perfect 5th | 1.500 |
| 3 | 5:4 | Major 3rd | 1.250 |
| 4 | 7:4 | Harmonic 7th (flat) | 1.750 |
| 5 | 9:8 | Major 2nd | 1.125 |
| 6 | 11:8 | Augmented 4th (neutral) | 1.375 |
| 7 | 13:8 | Minor 6th+ (neutral) | 1.625 |
| 8 | 15:8 | Major 7th | 1.875 |

SPREAD controls within-section detune. The canonical Amplified Futures mode.

### JUST — Ptolemaic Just Intonation
Voices mapped to a 12-note chromatic scale in Ptolemaic (5-limit) just intonation. See **[[Music-Theory]]** for the full ratio table. Produces natural harmonic locking — intervals are pure, not tempered.

### MICRO — Spectral Microtonality
Deterministic per-voice frequency offsets derived from the spectral series. Each voice gets a unique micro-pitch with no simple interval relationship. The most dissonant mode — useful for noise textures and extreme harmonic pressure.

---

## Ports

| Port | Type | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic (each channel = one chord/note) |
| V/OCT THRU | Output | Pass-through |
| OUT | Output | Polyphonic normalised audio |
| CV (×3) | Input | CV for MASS, SPREAD, TIMBRE |

---

## Amplitude normalisation

With N active voices, each voice's amplitude is scaled by 1/√N. This keeps the perceived loudness roughly constant as MASS changes — adding voices fills out the texture rather than clipping the output.

At MASS 16 each voice is at 1/4 amplitude, but the perceptual density is high. The output is then soft-clipped with tanh to catch occasional transient peaks.

---

## Patch tips

- **HARM mode + HarmonicPressure → V/OCT**: the canonical Amplified Futures patch. HarmonicPressure outputs harmonic partials; StringMassCore spreads voices within each section.
- **MASS 4–8, SPREAD 8–15¢**: sweet spot for dense but focused wall texture without muddiness.
- **UNIS mode + SPREAD → Drift SMOOTH**: drifting unison chorus. Pair with Choke for channel mixing.
- **JUST mode** works well for tonal drone chord work — the JI ratios produce natural harmonic locking.
- **TIMBRE → 0.3–0.5**: adds harmonic bite without full odd-harmonic saturation. Full TIMBRE (1.0) gives dense buzzy character.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Harmonic-Pressure]] | Primary V/OCT source |
| [[Wall-Conductor]] | CH inputs as section sources |
| [[Collapse-Saturator]] | Post-mass saturation |
| [[Drift]] | SMOOTH → SPREAD CV for slow detune breathing |
| [[Mass-Driver]] | 16 channels from StringMassCore × multiple into Mass Driver banks |

---

## See also

[[Harmonic-Pressure]] · [[Music-Theory]] · [[Playbooks]] · [[Wall-Conductor]]
