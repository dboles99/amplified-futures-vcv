# Pulse — 12HP

16-step no-wave step percussion sequencer. 4×4 toggle grid, white noise synthesis with HIT level, DECAY time, METAL filter, and CRACK transient burst. TRG clock in, audio out. Inspired by primitive drum machine aesthetics and Branca's percussive use of attack.

---

## Signal flow

```
TRG IN ──► [step counter] ──► active step? ──► [noise burst]
                                               ├─ HIT   : amplitude
                                               ├─ DECAY : 8–500ms exponential decay
                                               ├─ METAL : LP filter (360→80Hz) — 0=open, 1=dark
                                               └─ CRACK : 4ms transient burst (attack click)
                                               └──► OUT
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| HIT | 0–1 | Peak amplitude of triggered burst |
| DECAY | 0–1 | 0 = 8ms (tight click), 1 = 500ms (long thud) |
| METAL | 0–1 | Low-pass filter on noise — 0 = open (hi-hat), 1 = dark (kick/thud) |
| CRACK | 0–1 | Adds 4ms sharp transient on top of noise burst — "attack click" |
| Grid (4×4) | Toggle | 16 step on/off buttons. Active steps fire on clock |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| TRG IN | Input | Clock pulse — fires next step on rising edge |
| OUT | Output | Percussive noise audio |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Patch tips

- **METAL → 0 + CRACK → 0.7**: hi-hat texture. Clock at 8th notes.
- **METAL → 0.8 + DECAY → 0.6 + CRACK → 0.2**: kick-adjacent thud for anchoring drones.
- **Run two Pulse modules**: one for hi-hat (METAL 0, DECAY 0.1), one for thud (METAL 0.7, DECAY 0.5). Offset their grids.
- **HIT CV from Drift** STEP output: stochastic velocity for humanised feel.
- **Slow clock (0.5–2Hz) + full grid on**: produces a continuous noise texture with slow amplitude envelope — useful as a modulation source or noise bed.

---

## Known pairings

- **← Drift** GATE output as clock for tempo-synced randomness
- **→ Choke** MUTE CVs for rhythmic gating of drone channels
- **→ WallConductor** COLLAPSE gate on downbeats
- **← any clock source** TRG IN
