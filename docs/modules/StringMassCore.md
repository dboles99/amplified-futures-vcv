# String Mass Core — 16HP

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

| Mode | Description |
|---|---|
| UNIS | All voices at input pitch ± SPREAD/2 symmetric detuning |
| HARM | 16 voices distributed across 8 odd-harmonic sections [1, 3/2, 5/4, 7/4, 9/8, 11/8, 13/8, 15/8]. SPREAD controls within-section detune |
| JUST | Voices mapped to Ptolemaic just-intonation ratios (12-note chromatic). SPREAD adds ensemble colour |
| MICRO | Spectral microtonality — deterministic per-voice frequency offset derived from spectral series |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic (each channel = one chord/note) |
| V/OCT THRU | Output | Pass-through |
| OUT | Output | Polyphonic normalised audio |
| CV (×3) | Input | CV for MASS, SPREAD, TIMBRE |

---

## Patch tips

- **HARM mode + HarmonicPressure → V/OCT**: the canonical Amplified Futures patch. HarmonicPressure outputs harmonic partials; StringMassCore spreads voices within each section.
- **MASS 4–8, SPREAD 8–15¢**: sweet spot for dense but focused wall texture without muddiness.
- **UNIS mode + SPREAD → Drift SMOOTH**: drifting unison chorus. Pair with Choke for channel mixing.
- **JUST mode** works well for tonal drone chord work — the JI ratios produce natural harmonic locking.
- **TIMBRE → 0.3–0.5**: adds harmonic bite without full odd-harmonic saturation. Full TIMBRE (1.0) gives dense buzzy character.

---

## Known pairings

- **← HarmonicPressure** V/OCT for harmonic series chord stacks
- **→ WallConductor** CH inputs as section sources
- **→ CollapseSat** for saturation and compression
- **← Drift** SMOOTH → SPREAD CV for slow detune breathing
