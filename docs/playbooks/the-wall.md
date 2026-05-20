# Playbook: The Wall

**Concept:** Full Branca orchestra simulation — massed string voices across harmonic sections, conducted live through density sweeps and collapse events.

---

## Module roster

```
HarmonicPressure → StringMassCore × 2 → WallConductor → CollapseSat → [output]
                                                                ↑
                                              FeedbackGovernor ←┘ (loop)
                                     Drift → DENSITY CV
                                     Pulse → COLLAPSE gate
```

---

## Patch diagram

```
[HarmonicPressure]
  VOCT OUT (8ch) ──────────────────────────► [StringMassCore A] VOCT IN
                 └────────────────────────► [StringMassCore B] VOCT IN

[StringMassCore A] OUT ──► [WallConductor] CH1 IN
[StringMassCore B] OUT ──► [WallConductor] CH2 IN

[WallConductor] OUT L/R ──► [CollapseSat] IN L/R
[CollapseSat] OUT L/R ──────────────────────────► [output]

[CollapseSat] OUT L ──► [FeedbackGovernor] SEND
[FeedbackGovernor] RETURN ──► [WallConductor] CH3 IN  (feedback as 3rd section)

[Drift] SMOOTH ──► [WallConductor] DENSITY CV
[Pulse] GATE   ──► [WallConductor] COLLAPSE IN
[Pulse] OUT    ──► [CollapseSat] SC IN
```

---

## Starting parameters

### HarmonicPressure
- PITCH: 0V (C4 root)
- PARTIAL: 1, COUNT: 8 (partials 1–8)
- SPREAD: 0.2
- TUNING: JUST

### StringMassCore A — outer wall
- MASS: 8, SPREAD: 0.3, TIMBRE: 0.5
- MODE: HARM, SECTION: 2

### StringMassCore B — inner cluster (detune offset)
- MASS: 6, SPREAD: 0.15, TIMBRE: 0.3
- MODE: HARM, SECTION: 1

### WallConductor
- DENSITY: 0.25 (start with CH1 only)
- PRESSURE: 0.25, WIDTH: 0.85
- FEEDBACK: 0.15, RECOVERY: 0.65

### Drift (modulating DENSITY)
- RATE: 0.12, WANDER: 0.35, SLEW: 0.4
- Connect SMOOTH → WallConductor DENSITY CV (atten: +0.3)

### CollapseSat
- DRIVE: 0.25, BUZZ: EVEN, RECOVERY: 0.6

### FeedbackGovernor
- AMOUNT: 0.3, TONE: 0.45, DECAY: 0.4

### Pulse (performance clock)
- Kick pattern (steps 0, 4, 8, 12 on), DECAY: 0.4, METAL: 0.6
- Clock at ≈80 BPM. GATE → WallConductor COLLAPSE IN

---

## Performance arc

| Phase | Action |
|---|---|
| Open | Everything at starting params. Drift slowly raises DENSITY. Wall builds from CH1 alone. |
| Build | As DENSITY crosses 0.5, CH2 enters. Slowly raise FeedbackGovernor AMOUNT to 0.4. |
| Peak | DENSITY → 1.0 (all sections). Push WallConductor PRESSURE to 0.5. CollapseSat DRIVE up. |
| Collapse event | Press WallConductor COLLAPSE — hold 2s, release. Wall rises back over 3s. |
| Second collapse | Pulse at 4/4 kicks driving COLLAPSE gate. Rhythmic drop/rise pattern for 16 bars. |
| Fade | Slowly pull DENSITY back to 0.25. Kill FeedbackGovernor (KILL button). |

---

## Notes

- Keep FeedbackGovernor AMOUNT below 0.45 before pushing PRESSURE above 0.5.
- If feedback runaway starts, press KILL on FeedbackGovernor.
- SPREAD on HarmonicPressure controls overall "out-of-tune" character — 0.1 is tense; 0.4 is loose.
