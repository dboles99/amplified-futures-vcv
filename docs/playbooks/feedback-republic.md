# Playbook: Feedback Republic

**Concept:** Cross-feedback system between two oscillator sources via Send module, governed by FeedbackGovernor. The C-bus becomes a third voice. Drift animates tone and decay. Kill events as performance gestures.

---

## Module roster

```
DroneClone A ──► Send ──► DroneClone B
     ↑              ↓
     └── FeedbackGovernor ←┘ (C-bus governed)
Drift → TONE CV
Pulse → KILL gate
```

---

## Patch diagram

```
[DroneClone A] OUT ──► [Send] IN A
[DroneClone B] OUT ──► [Send] IN B

[Send] OUT A ──► [DroneClone A] RTN IN   ← A gets B back + C-bus return
[Send] OUT B ──► [DroneClone B] RTN IN   ← B gets A forward

[Send] OUT C ──► [FeedbackGovernor] SEND
[FeedbackGovernor] RETURN ──► [Send] IN B  ← C-bus governed + returned into B path

[Drift] SMOOTH ──► [FeedbackGovernor] TONE CV (atten: +0.6)
[Drift] STEP   ──► [FeedbackGovernor] DECAY CV (atten: +0.3)
[Pulse] GATE   ──► [FeedbackGovernor] KILL GATE

[Send] OUT A + OUT B ──► [WallConductor] CH1 + CH2 IN
[WallConductor] OUT L/R ──► [output]
```

---

## Starting parameters

### DroneClone A — source voice
- MASS: 4, TENSION: 0.35, JAWARI: 0.2, WEIGHT: 0.15, DRIFT: 0.2
- DECAY: 0.5, CHOKE AMT: 0.8

### DroneClone B — target/return voice
- MASS: 4, TENSION: 0.4, JAWARI: 0.35, SHIMMER: 0.25, DRIFT: 0.25
- (slightly different character for asymmetry)

### Send
- A→B: 0.4, B→A: 0.3
- A→C: 0.5, C→A: 0.4  ← C-bus active

### FeedbackGovernor
- AMOUNT: 0.35, TONE: 0.5, DECAY: 0.35

### Drift (governing tone/decay)
- RATE: 0.18, WANDER: 0.4, SLEW: 0.3

### Pulse (kill clock)
- Steps 0 and 8 active (half-bar). GATE → Governor KILL
- Clock: slow, ≈40 BPM

### WallConductor
- DENSITY: 0.6, PRESSURE: 0.2, WIDTH: 0.7

---

## Performance arc

| Phase | Action |
|---|---|
| Establish | Both DroneClones running. Send in Cross Blend (A→B=0.4, B→A=0.3). C-bus off. |
| Introduce C-bus | Slowly raise Send A→C to 0.4, C→A to 0.3. Third character emerges from feedback. |
| Governor active | FeedbackGovernor AMOUNT up to 0.35. Tone drift begins. |
| Rhythmic kills | Start Pulse clock. Every half-bar the C-bus zeros. Listen to the silence shape. |
| Build tension | Increase Send A→C to 0.7. More C-bus presence. Push Governor AMOUNT to 0.5. |
| Collapse | Raise WallConductor PRESSURE to 0.6. Hit COLLAPSE. Let it breathe back in. |
| Resolution | Kill Pulse. Slowly reduce A→C to 0. Feedback drains. Back to clean cross-blend. |

---

## Notes

- The C-bus has 1-sample delay — safe for feedback loops with no risk of instantaneous runaway.
- FeedbackGovernor DECAY should stay below 0.5 unless intentionally building toward silence.
- The Drift STEP → DECAY CV atten should be low (0.3) — too much causes erratic fading.
- If the system gets too loud, use Governor KILL (manual button) rather than pulling AMOUNT — cleaner transient.
