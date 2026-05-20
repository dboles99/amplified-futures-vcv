# Wall Conductor — 22HP

Section-based performance mixer/conductor. The live performance control surface for the Amplified Futures stack — modelled on a conductor orchestrating massed signal sections. DENSITY sweeps channels in, PRESSURE saturates, WIDTH spreads, FEEDBACK loops, COLLAPSE drops it all.

---

## Signal flow

```
CH1–4 IN ──► [DENSITY gate per channel] ──► constant-power pan (WIDTH)
feedbackL/R ──► [FEEDBACK amount] ──►──────────────────────────────────┐
                                                                        ▼
                                          sumL/sumR ──► PRESSURE (tanh) ──► × collapseEnv
                                                                        │
COLLAPSE BTN/IN ──► instant drop ──► collapseEnv ──► RECOVERY rise ──►┘
                                                                   └──► OUT L / OUT R
V/OCT IN ──────────────────────────────────────────────────────────────► V/OCT THRU
```

DENSITY gate per channel: `gain = clamp(DENSITY × 4 − i, 0, 1)` — channels sweep in one at a time as DENSITY rises.

---

## Controls

| Control | Range | Notes |
|---|---|---|
| DENSITY | 0–1 | 0 = silence, 0.25 = CH1 only, 0.5 = CH1+2, 0.75 = CH1–3, 1 = all 4 |
| PRESSURE | 0–1 | Drive into tanh saturation — 0 = clean, 1 = heavy clip |
| WIDTH | 0–1 | Stereo spread — 0 = mono centre, 1 = full L/R |
| FEEDBACK | 0–1 | One-sample output→input feedback amount. Keep below 0.5 |
| RECOVERY | 0–1 | COLLAPSE recovery time — 0 = 50ms, 1 = 10s |
| COLLAPSE | Button | Momentary: drops output to zero, rises on release per RECOVERY |

All knobs except COLLAPSE have attenuverter + CV.

---

## Ports

| Port | Type | Notes |
|---|---|---|
| CH1–4 IN | Input | Section inputs (mono or poly summed) |
| COLLAPSE IN | Input | Gate — high = collapsed, low = recovering |
| CV (×5) | Input | CV for DENSITY, PRESSURE, WIDTH, FEEDBACK, RECOVERY |
| OUT L / OUT R | Output | Stereo master |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Patch tips

- **DENSITY as the primary performance control**: automate with Drift SMOOTH for slow orchestral builds.
- **COLLAPSE + long RECOVERY (0.8)**: press COLLAPSE, release — 3–4s shaped rise back in. Press and hold for silence.
- **FEEDBACK at 0.2–0.3 + PRESSURE at 0.5**: creates dense harmonic buildup without runaway. Never push FEEDBACK above 0.7.
- **WIDTH automation**: modulate with Drift at very slow rate for slow stereo breathing.
- **Stack two WallConductors**: first handles sections 1–4, second handles 5–8 (two DroneClone instances). COLLAPSE both simultaneously from one gate.

---

## Known pairings

- **← DroneClone** (×2–4) as section inputs
- **← Pulse** GATE → COLLAPSE for beat-synced drops
- **→ CollapseSat** for post-mix saturation
- **← Drift** SMOOTH → DENSITY CV for automated section sweeps
- **← HarmonicPressure** V/OCT → DENSITY CV via attenuverter for pitch-tracked density
