# Mass Driver (AF-01) — 32HP

16-channel no-wave signal mixer. Per-channel GAIN and MUTE. Master DENSITY sweep, PRESSURE saturation, WIDTH stereo spread, FEEDBACK loop, COLLAPSE gate. Five outputs: OUT L/R, AUX L/R, SUM. The large-format conductor for full 16-source signal stacks.

---

## Signal flow

```text
CH 1–16 IN
    │
    ├─ MUTE toggle (per-channel LED)
    ├─ GAIN (0–2×, per-channel)
    ├─ DENSITY sweep: gain = clamp(DENSITY×16 − i, 0, 1) for channel i (0-indexed)
    ├─ WIDTH pan: linear spread −WIDTH to +WIDTH across channels 1–16
    │
    ├─ feedbackL/R × FEEDBACK ────────────────────────────────────────────►─┐
    │                                                                        │
    │  sumL/sumR ──► AUX L/R (pre-PRESSURE, with MASS scale)                │
    │  sum mono  ──► SUM (pre-PRESSURE, mono, with MASS scale)              │
    │                                                                        │
    └─ PRESSURE tanh ──► × collapseEnv ──► × MASS ──► OUT L / OUT R ◄──────┘

COLLAPSE BTN / COLLAPSE IN ──► collapseEnv → 0 instantly, 1.5 s exponential recovery
```

DENSITY formula per channel i (0-indexed): `gain = clamp(DENSITY × 16 − i, 0, 1)`

At DENSITY 0.25: channels 1–4 active. At 0.5: channels 1–8. At 0.75: 1–12. At 1.0: all 16.

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| GAIN 1–16 | 0–15 | 0–2× | 0.75 | Per-channel level. Unity at 0.75 |
| MUTE 1–16 | 16–31 | Toggle | off | Per-channel mute. Red LED when active |
| DENSITY | 32 | 0–1 | 1.0 | Sweeps channels 1 → 16 in sequence |
| DENSITY ATTEN | 33 | −1 to +1 | 0 | Attenuverter for DENSITY CV |
| PRESSURE | 34 | 0–1 | 0.25 | tanh drive: 0 = clean, 1 = hard clip |
| PRESSURE ATTEN | 35 | −1 to +1 | 0 | Attenuverter for PRESSURE CV |
| WIDTH | 36 | 0–1 | 0.8 | Stereo spread: 0 = mono centre, 1 = full L/R |
| WIDTH ATTEN | 37 | −1 to +1 | 0 | Attenuverter for WIDTH CV |
| MASS | 38 | 0–1 | 0.75 | Master output level (0–5 V scale) |
| MASS ATTEN | 39 | −1 to +1 | 0 | Attenuverter for MASS CV |
| FEEDBACK | 40 | 0–1 | 0 | 1-sample feedback loop amount. Hard-capped at 0.92× |
| FEEDBACK ATTEN | 41 | −1 to +1 | 0 | Attenuverter for FEEDBACK CV |
| COLLAPSE | 42 | Button/gate | — | Instantly drops output to zero; 1.5 s exponential recovery |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| CH 1–16 | Input | Audio | Per-channel signal inputs |
| DENSITY CV | Input | CV | Modulates DENSITY (scaled by DENSITY ATTEN) |
| PRESSURE CV | Input | CV | Modulates PRESSURE |
| WIDTH CV | Input | CV | Modulates WIDTH |
| MASS CV | Input | CV | Modulates MASS |
| FEEDBACK CV | Input | CV | Modulates FEEDBACK |
| COLLAPSE IN | Input | Gate | High = collapse triggered |
| V/OCT IN | Input | CV | Pass-through |
| OUT L | Output | Audio | Full processed stereo left (post-PRESSURE, post-COLLAPSE) |
| OUT R | Output | Audio | Full processed stereo right |
| AUX L | Output | Audio | Pre-PRESSURE stereo left (post-DENSITY, post-WIDTH) |
| AUX R | Output | Audio | Pre-PRESSURE stereo right |
| SUM | Output | Audio | Pre-PRESSURE mono sum |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 32 | DENSITY | CC 1 (mod wheel) |
| 34 | PRESSURE | CC 11 (expression) |
| 36 | WIDTH | CC 10 (pan) |
| 38 | MASS | CC 7 (volume) |
| 40 | FEEDBACK | CC 14 |
| 42 | COLLAPSE | CC 15 (momentary, value ≥ 64 triggers) |

Per-channel GAIN (indices 0–15) can be mapped to faders: CH1 GAIN → CC 16, CH2 → CC 17, etc. (General MIDI fader convention).

---

## Recommended configurations

**Mass Build** — DENSITY starts at 0, PRESSURE 0, MASS 0.75, WIDTH 0.8, FEEDBACK 0. Slowly raise DENSITY with mod wheel from 0 → 1 over 4 minutes. Channels enter one at a time. Then push PRESSURE to 0.4 for harmonic density.

**Feedback Wall** — DENSITY 1.0, FEEDBACK 0.65, PRESSURE 0.35, MASS 0.6. The 1-sample feedback loop accumulates. COLLAPSE clears it. 1.5 s recovery. Use AUX L/R for a pre-saturation reference.

**Section Architecture** — CH1–4: oscillators. CH5–8: string masses. CH9–12: filtered noise. CH13–16: percussion. All at unity GAIN. DENSITY sweeps entire categories in at threshold values 0.25 / 0.5 / 0.75 / 1.0.

**Parallel Processing** — AUX L/R → external processor (compressor, reverb) → back into CH13–16 inputs. OUT L/R carries the saturated full mix. AUX carries the dry pre-pressure signal. Blend the processed AUX return with the direct OUT.

---

## Basic setup — sound in 60 seconds

1. Add Mass Driver to your patch.
2. Patch 4 source signals → CH1–CH4.
3. Patch OUT L and OUT R → your audio interface.
4. Set DENSITY to 1.0, PRESSURE 0.25, WIDTH 0.8, MASS 0.75, FEEDBACK 0.
5. You have a 4-channel stereo mix with light saturation and full stereo width.
6. Map DENSITY to mod wheel. Pull to 0 — only CH1 is audible.
7. Press COLLAPSE. Release. Adjust RECOVERY timing via the 1.5 s fixed recovery arc.

---

## How-tos

### DENSITY as single performance gesture

- Map DENSITY to MIDI CC 1 (mod wheel).
- Route distinct source types to channel banks: CH1–4 = drones, CH5–8 = masses, CH9–12 = noise.
- Push the mod wheel from 0 → 1 over a performance section.
- The entire orchestral texture expands from one voice to all 16.

### AUX for parallel compression

- Patch AUX L and AUX R → an external compressor or limiter.
- Patch the compressed output → CH15 and CH16 inputs with GAIN at 0.3.
- DENSITY must reach ≥ 0.94 for CH15–16 to fully open.
- The parallel compressed signal blends into the dense mix only at maximum DENSITY.

### V/OCT thru for pitch tracking

- Patch a keyboard V/OCT → Mass Driver V/OCT IN.
- Patch V/OCT THRU → HarmonicPressure V/OCT IN.
- HarmonicPressure follows the keyboard root. StringMassCore follows HarmonicPressure.
- The entire harmonic pitch architecture tracks the keyboard without breaking the Mass Driver signal path.

### COLLAPSE as structural climax

- Build toward maximum DENSITY and PRESSURE over a long performance arc.
- At the climax, press COLLAPSE. The output drops to zero instantly.
- FEEDBACK state clears — the loop resets.
- Over 1.5 s the signal returns. At high PRESSURE the recovery is harmonically dense.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| COLLECTIVE (collective-refusal) | DENSITY 1.0 — all 16 channels active, maximum collective mass |
| PRESSURE (rent-pressure) | PRESSURE + FEEDBACK building — unrelenting accumulation toward saturation |
| MANAGED COLLAPSE | COLLAPSE gate — the signature mass collapse gesture |
| GRID (work-clock) | Per-channel MUTE pattern as a fixed grid assignment — which voices occupy which positions |
| FAILURE (managed-collapse) | FEEDBACK pushed to 0.7–0.85 — the feedback loop approaches instability |

Macro assignments: DENSITY → COLLECTIVE macro. PRESSURE → PRESSURE macro. MASS → VOICE macro (master loudness). COLLAPSE → FAILURE macro trigger.

---

## Known pairings

| Module | Role |
| --- | --- |
| StringMassCore ×4 | 16 voices of harmonic mass — one per channel bank |
| HarmonicPressure | Polyphonic V/OCT into StringMassCore → Mass Driver |
| WallConductor | Feeds Mass Driver AUX for a second-stage DENSITY + COLLAPSE layer |
| FeedbackGovernor | External feedback loop in parallel with Mass Driver FEEDBACK |
| CollapseSat | Post-process OUT L/R for additional harmonic character |
| Drift | SMOOTH → DENSITY CV for slow autonomous mass evolution |
