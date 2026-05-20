# DroneClone — 22HP

8-voice amplified string wall. The centrepiece of the Amplified Futures signal chain — inspired by massed electric guitar density. Up to 16 polyphonic channels × 8 voices = 128 simultaneous oscillators.

---

## Signal flow

```
V/OCT IN ──► [per-channel, per-voice oscillator bank]
              ├─ MASS controls active voice count (1–8)
              ├─ TENSION sets harmonic edge (saw content)
              ├─ SHIMMER adds upper partial air
              ├─ JAWARI introduces rattle/buzz (even harmonics)
              ├─ WEIGHT adds sub-octave body
              └─ DRIFT applies per-voice slow phase wander

CHOKE BTN/IN ──► instant amplitude collapse (gated)
RTN IN ──────► feedback return mixed into oscillator input

──► OUT (polyphonic audio)
──► V/OCT THRU
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| MASS | 1–8 | Active voices per channel. Higher = denser, louder wall |
| TENSION | 0–1 | Harmonic edge — adds odd-harmonic saw content |
| SHIMMER | 0–1 | Upper partial brightness — adds 4th–8th harmonics |
| JAWARI | 0–1 | Even-harmonic rattle (asymmetric buzz, like sitar jawari) |
| WEIGHT | 0–1 | Sub-octave body mix |
| DRIFT | 0–1 | Per-voice slow phase wander rate |

All knobs have attenuverter + CV input.

---

## Ports

| Port | Type | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic pitch (sets channel count) |
| V/OCT THRU | Output | Pass-through |
| OUT | Output | Polyphonic 8-voice mixed audio |
| RTN | Input | Feedback return — patched from a downstream output |
| CHOKE | Input | Gate: 10V high collapses all voices immediately |
| CV (×6) | Input | CV for each main knob |

---

## Patch tips

- **JAWARI + TENSION together** gives the characteristic no-wave guitar wall — start at TENSION 0.4, JAWARI 0.2, adjust by ear.
- **RTN feedback loop**: patch OUT → FeedbackGovernor → RTN. Keep AMOUNT below 0.4 before pushing TENSION up.
- **CHOKE as performance event**: patch a manual gate or WallConductor COLLAPSE gate → CHOKE for sudden silence arcs.
- **MASS at 2–3 with DRIFT at 0.3** gives natural chorus without muddiness. MASS 8 is for wall maximalism only.
- **WEIGHT at 0.2** adds warmth without pushing the sub into mud. Higher values work best with high-pass downstream.

---

## Known pairings

- **→ WallConductor** for section-based performance mixing
- **→ CollapseSat** for saturation and collapse events
- **← HarmonicPressure** → V/OCT for harmonic series chord stacks
- **← Drift** → DRIFT CV for externally clocked wander speed
- **← Send** RTN bus for cross-feedback between two DroneClone instances
