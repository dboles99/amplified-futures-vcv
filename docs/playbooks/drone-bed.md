# Playbook: Drone Bed

**Concept:** Polyphonic DroneCore stack — 4 instances at different detune amounts routed through Choke for per-layer mixing. Drift modulates TIMBRE and DETUNE for slow harmonic breathing. No percussion, pure sustain.

---

## Module roster

```
Drift × 2 → DroneCore × 4 → Choke → [output]
```

---

## Patch diagram

```
[Drift A] SMOOTH ──► [DroneCore 1] DETUNE CV (atten: +0.4)
                └──► [DroneCore 2] DETUNE CV (atten: +0.3)
[Drift A] STEP   ──► [DroneCore 3] DETUNE CV (atten: +0.25)

[Drift B] SMOOTH ──► [DroneCore 1] TIMBRE CV (atten: +0.5)
                └──► [DroneCore 2] TIMBRE CV (atten: −0.3)  ← inverted for contrast
[Drift B] STEP   ──► [DroneCore 4] TIMBRE CV (atten: +0.4)

[DroneCore 1] OUT ──► [Choke] CH1 IN
[DroneCore 2] OUT ──► [Choke] CH2 IN
[DroneCore 3] OUT ──► [Choke] CH3 IN
[DroneCore 4] OUT ──► [Choke] CH4 IN

[Choke] OUT L/R ──► [output]
```

All 4 DroneCores share same V/OCT source (or run free at PITCH = 0V).

---

## Starting parameters

### DroneCore 1 — foundation
- PITCH: 0V, DETUNE: 8¢, TIMBRE: 0.1

### DroneCore 2 — doubled layer (up 1 oct: PITCH knob +1)
- PITCH: +1V, DETUNE: 14¢, TIMBRE: 0.2

### DroneCore 3 — wide detune layer
- PITCH: 0V, DETUNE: 22¢, TIMBRE: 0.0

### DroneCore 4 — harmonic layer
- PITCH: 0V, DETUNE: 35¢, TIMBRE: 0.6

### Drift A (DETUNE modulation)
- RATE: 0.08, WANDER: 0.3, SLEW: 0.35

### Drift B (TIMBRE modulation)
- RATE: 0.13, WANDER: 0.25, SLEW: 0.5

### Choke
- GAIN all: 0.65, TONE all: 0.75
- MAIN: 0.7 (leave headroom — running 4 layers)

---

## Performance arc

| Phase | Action |
|---|---|
| Open | CH1 only (mute CH2–4). Let Drift A establish detune drift. |
| Layer 1→2 | Unmute CH2. Watch beating patterns emerge between layers. |
| Layer 1–3 | Unmute CH3. Now 3 detune layers — beating gets complex. |
| Full stack | Unmute CH4. Detune interference creates dense, evolving wash. |
| Thin | Solo CH4 (mute 1–3) for the harmonic-heavy single layer. Compare with full stack. |
| Drift rate push | Increase Drift A RATE to 0.4 — faster stochastic detune jumps. |
| Fade | Pull Choke MAIN slowly to 0. |

---

## Notes

- Different RATE values on Drift A and B (0.08 vs 0.13) prevent synchronisation — the ratio is close to prime-number relationship.
- Inverting the TIMBRE CV atten on DroneCore 2 means it goes darker as the others go brighter — creates motion within the static pitch.
- This patch works entirely without a clock source.
