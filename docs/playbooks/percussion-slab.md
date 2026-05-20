# Playbook: Percussion Slab

**Concept:** Pulse drives rhythmic structure — triggers StringMassCore as a pitched percussion instrument, with Choke controlling per-hit gain and Drift adding stochastic velocity. WallConductor COLLAPSE gate tied to a specific beat for periodic rhythmic drops.

---

## Module roster

```
Pulse → StringMassCore (triggered, not sustained)
      → Choke MUTE CVs (rhythmic gating of drone layers)
      → WallConductor COLLAPSE (downbeat drops)
Drift → Pulse HIT CV (stochastic velocity)
HarmonicPressure → StringMassCore VOCT (pitched hits)
DroneCore × 2 → Choke (sustained layers being muted rhythmically)
```

---

## Patch diagram

```
[Pulse] GATE ──► [StringMassCore] — use GATE to trigger env (not native, see note)
[Pulse] OUT  ──► [Choke] CH3 IN  (percussion hit as audio channel)

[Pulse] GATE ──► [Choke] MUTE CV CH1  (mutes drone A on hit)
[Pulse] GATE ──► [Choke] MUTE CV CH2  (mutes drone B on hit — inverted via attenuverter −1)

[Drift] STEP ──► [Pulse] HIT CV (atten: +0.4)   ← stochastic velocity
[Drift] GATE ──► [Pulse] TRG IN                  ← Drift as slow irregular clock

[HarmonicPressure] VOCT OUT ──► [StringMassCore] VOCT IN
[StringMassCore] OUT ──► [Choke] CH4 IN (harmonic mass as texture layer)

[DroneCore A] OUT ──► [Choke] CH1 IN
[DroneCore B] OUT ──► [Choke] CH2 IN

[Choke] OUT L/R ──► [WallConductor] CH1 IN (main)

[Pulse] GATE (step 0 only — use separate Pulse) ──► [WallConductor] COLLAPSE IN

[WallConductor] OUT L/R ──► [output]
```

> **Note on StringMassCore as percussion:** StringMassCore is a continuous oscillator — to use it percussively, route Pulse OUT → CollapseSat SC IN to give each Pulse hit a transient drive boost, creating attack on the mass texture.

---

## Starting parameters

### Pulse (primary rhythm)
- Kick Pattern preset: steps 0, 4, 8, 12
- HIT: 0.7, DECAY: 0.45, METAL: 0.65, CRACK: 0.35
- Clock: 120 BPM external or use Drift B GATE

### Drift A (velocity modulation)
- RATE: 0.55, WANDER: 0.85, SLEW: 0.0 (raw steps for velocity jumps)
- STEP → Pulse HIT CV (atten: +0.4)

### Drift B (clock source — irregular time)
- RATE: 0.4, WANDER: 0.6, SLEW: 0.0
- GATE → Pulse TRG IN (for unclocked version)

### HarmonicPressure
- PARTIAL: 3, COUNT: 4, TUNING: EQUAL
- SPREAD: 0.05 (tight — percussion wants pitch identity)

### StringMassCore
- MASS: 4, SPREAD: 0.1, TIMBRE: 0.6
- MODE: HARM, SECTION: 1

### DroneCore A
- PITCH: 0V, DETUNE: 10¢, TIMBRE: 0.2

### DroneCore B
- PITCH: −1V (sub octave), DETUNE: 6¢, TIMBRE: 0.0

### Choke
- CH1 GAIN: 0.5, CH2 GAIN: 0.4, CH3 GAIN: 0.8, CH4 GAIN: 0.35
- All TONE: 0.7
- MAIN: 0.6

### WallConductor
- DENSITY: 1.0, PRESSURE: 0.3, WIDTH: 0.7
- RECOVERY: 0.15 (fast — short rhythmic drop)

---

## Performance arc

| Phase | Action |
|---|---|
| Skeleton | Pulse only, no drone layers. Kick pattern, METAL-heavy. |
| Add sub | Unmute Choke CH2 (DroneCore B at −1V). Low body under kicks. |
| Add texture | Unmute Choke CH4 (StringMassCore). Harmonic mass on every beat. |
| Full mix | All channels. Drift velocity making hits irregular. |
| WallConductor drops | Tie step 0 gate (separate clock → first beat only) to WallConductor COLLAPSE. Hear the wall drop on bar 1. |
| Sparse clock | Switch Pulse TRG to Drift B GATE. Irregular hits — rhythm dissolves into texture. |
| Noise flood | Push Pulse METAL to 0.0 and DECAY to 0.8 — becomes continuous noise bed. |

---

## Notes

- The Choke MUTE CV inversion (atten −1) on CH2 means when Pulse fires, CH2 opens rather than closes — counterpoint to CH1 muting.
- Short WallConductor RECOVERY (0.15) is key here — makes the collapse feel rhythmic rather than atmospheric.
- Drift B as clock (not a steady tempo) works best when the piece is already established — introduce it as a destabilisation device.
