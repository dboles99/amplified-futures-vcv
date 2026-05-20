# Amplified Futures — Playbooks

Named patch configurations for live performance. Each playbook describes a signal routing, starting parameter values, and a performance arc.

---

## Available playbooks

| Playbook | Modules | Character |
|---|---|---|
| [The Wall](the-wall.md) | HarmonicPressure → StringMassCore × 2 → WallConductor → CollapseSat + FeedbackGovernor loop | Full massed-voice orchestra. Density sweeps and collapse events. |
| [Drone Bed](drone-bed.md) | DroneCore × 4 → Choke, Drift × 2 | Pure sustain. 4 detuned layers with Drift-animated timbre. No clock. |
| [Feedback Republic](feedback-republic.md) | DroneClone × 2 ↔ Send ↔ FeedbackGovernor, Drift, Pulse | Cross-feedback system. C-bus governed loop. Kill events as rhythm. |
| [Harmonic Pressure Session](harmonic-pressure-session.md) | HarmonicPressure → StringMassCore → WallConductor → CollapseSat | PARTIAL as primary performance control. Ascend the harmonic series live. |
| [Percussion Slab](percussion-slab.md) | Pulse → Choke, DroneCore × 2, StringMassCore, Drift | Rhythmic structure driving drone muting. Irregular Drift clock for texture dissolution. |

---

## Reading a playbook

Each playbook has:
- **Module roster**: which modules are needed
- **Patch diagram**: how to connect them (ASCII routing)
- **Starting parameters**: where to set everything before you play
- **Performance arc**: a suggested sequence of gestures from open to close
- **Notes**: gotchas, safety limits, and unconventional behaviours

---

## General performance principles

- **Start sparse.** One source, one processor. Layer in.
- **Collapse is a gesture, not a failure.** Use COLLAPSE events intentionally — they are the climax, not the reset.
- **Feedback has a safe limit.** FeedbackGovernor AMOUNT above 0.5 with DECAY below 0.3 = runaway. KILL button is your abort.
- **Drift rates in prime-number ratios** (0.07, 0.11, 0.17 Hz) prevent synchronisation and give each modulation source independent character.
- **WallConductor DENSITY** is the main performance fader — more expressive than individual volume control.
