# Playbook: Harmonic Pressure Session

**Concept:** HarmonicPressure drives StringMassCore in full harmonic mode. PARTIAL and COUNT are performance controls — sweeping up the harmonic series live. Drift modulates root pitch. WallConductor governs final output.

---

## Module roster

```
Drift → HarmonicPressure → StringMassCore → WallConductor → CollapseSat → [output]
```

---

## Patch diagram

```
[Drift] SMOOTH ──► [HarmonicPressure] PITCH CV (atten: +0.15)  ← subtle root wander

[HarmonicPressure] VOCT OUT (poly) ──► [StringMassCore] VOCT IN

[StringMassCore] OUT ──► [WallConductor] CH1 IN
[StringMassCore] VOCT THRU ──► [WallConductor] VOCT IN (thru chain)

[WallConductor] OUT L/R ──► [CollapseSat] IN L/R
[CollapseSat] OUT L/R ──► [output]
```

---

## Starting parameters

### HarmonicPressure
- PITCH: 0V (C4)
- PARTIAL: 1, COUNT: 4
- SPREAD: 0.15
- TUNING: JUST

### StringMassCore
- MASS: 8, SPREAD: 0.25, TIMBRE: 0.4
- MODE: HARM, SECTION: 2

### Drift (root pitch animation)
- RATE: 0.07, WANDER: 0.2, SLEW: 0.5
- SMOOTH → HarmonicPressure PITCH CV (atten: +0.15)

### WallConductor
- DENSITY: 1.0 (full — single source), PRESSURE: 0.2
- WIDTH: 0.8, FEEDBACK: 0, RECOVERY: 0.5

### CollapseSat
- DRIVE: 0.2, BUZZ: EVEN, RECOVERY: 0.5

---

## Performance controls (live manipulation)

### HarmonicPressure PARTIAL knob
Moving PARTIAL is the primary performance gesture:
- PARTIAL 1 = fundamental gravity (feels stable)
- PARTIAL 3 = skips octave and 2nd — immediately tense
- PARTIAL 7–9 = high harmonic tension, microtonal territory
- PARTIAL 14–16 = extreme upper series — almost noise-like

### HarmonicPressure COUNT knob
- COUNT 2–3 = sparse, open, focused
- COUNT 8 = full wall character
- COUNT 16 = maximum density at high PARTIAL = overwhelming

### StringMassCore MODE switch
- HARM → JUST: shift from section-spread to JI chord. Dramatic tonal change.
- JUST → MICRO: adds spectral microtonality — "out of tune" tension.

---

## Performance arc

| Phase | Duration | Action |
|---|---|---|
| Ground | 2–3 min | PARTIAL 1, COUNT 4. Pure low harmonic presence. Let Drift move root. |
| Ascent | 3–5 min | Slowly move PARTIAL up: 1 → 3 → 5 → 7. Each step raises tension. COUNT stays at 4. |
| Expansion | 2 min | At PARTIAL 7, expand COUNT to 8. More harmonic density at this register. |
| Mode shift | 1 min | Switch StringMassCore MODE: HARM → JUST. JI chord locks. Different kind of tension. |
| Push | 2 min | Increase WallConductor PRESSURE to 0.5. Raise CollapseSat DRIVE to 0.4. |
| Peak collapse | — | Hit CollapseSat COLLAPSE. Hold 3s. Release. Long RECOVERY (0.7). |
| Return | 2 min | Pull PARTIAL back to 1 over 2 min. COUNT back to 4. |
| Fade | — | WallConductor DENSITY slow pull to 0. |

---

## Notes

- SPREAD on HarmonicPressure at 0.15 means the ensemble has natural detuning without losing pitch identity.
- MODE: MICRO in StringMassCore adds extra individual-voice vibrato — combine with PARTIAL 8+ for maximum spectral complexity.
- Root pitch wander (Drift atten +0.15) is subtle by design — too much and the harmonic series loses its anchor.
