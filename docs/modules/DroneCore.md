# DroneCore — 8HP

Two-voice detuned oscillator core. The base voice of the Amplified Futures stack — a pair of sine-to-harmonic oscillators running symmetrically detuned around a shared pitch centre. Designed to pair with DroneClone or run in polyphonic stacks.

---

## Signal flow

```
V/OCT IN ──► [pitch + detune] ──► Voice A (+detune/2)  ──►┐
                                  Voice B (−detune/2)  ──►┤ mix ──► OUT
                                                            │
                              [TIMBRE] blends 2nd/3rd/4th harmonics
V/OCT IN ──────────────────────────────────────────────► V/OCT THRU
```

---

## Controls

| Control | Range | Default | Notes |
|---|---|---|---|
| PITCH | 82–1319 Hz | 220 Hz | Base frequency (A2–E6 approx) |
| DETUNE | 0–100¢ | 12¢ | Symmetric split: ±DETUNE/2 per voice |
| TIMBRE | 0–1 | 0 | 0 = pure sine; 1 = 2nd+3rd+4th harmonics (Branca third-bridge model) |

Every knob has an **attenuverter** (−1 to +1) and a **CV input** (±5V bipolar, scaled by atten).

---

## Ports

| Port | Type | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic pitch input (sets channel count) |
| V/OCT THRU | Output | Pass-through, same channel count |
| OUT | Output | Mixed dual-voice audio, polyphonic |
| MOD | Input | CV for PITCH (with attenuverter) |
| CV (×2) | Input | CV for DETUNE, TIMBRE |

---

## Patch tips

- **Stack 4 DroneCore** in unison with slightly offset DETUNE values (6¢, 12¢, 18¢, 24¢) for a warm orchestral spread without DroneClone.
- **Feed TIMBRE CV from Drift** at low WANDER for slow harmonic breathing.
- **Run polyphonic** (4–8 channels from HarmonicPressure) for a full chord stack — each V/OCT channel gets its own dual-voice pair.
- Keep DETUNE below 30¢ for tonal drones; push to 80–100¢ for beating interference textures.

---

## Known pairings

- **→ DroneClone** as a sub-fundamental layer (DroneCore one octave below)
- **→ Choke** for per-voice gain and muting
- **← Drift** SMOOTH output → TIMBRE CV for slow harmonic wander
- **← HarmonicPressure** → V/OCT IN for microtonal partial stacks
