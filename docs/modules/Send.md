# Send — 12HP

2×2 cross-send feedback routing matrix. A→B send, B→A return, A→C/C→A internal feedback bus. The C-bus is one-sample delayed for safe self-oscillation. Polyphonic.

---

## Signal flow

```
A IN ──►──── A→B SEND ────────────────────────► B OUT
        └──► A→C DEPTH ──► [C bus, 1-sample delay] ──► C OUT
                                                     ▲
B IN ──►──── B→A RETURN ─────────────────────────────┤
        └──────────────────────────────────────────── C→A RETURN ──► A OUT
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| A→B SEND | 0–1 | Level of A routed into B output |
| B→A RETURN | 0–1 | Level of B routed back into A output |
| A→C DEPTH | 0–1 | Feed depth into the internal C feedback bus |
| C→A RETURN | 0–1 | Amount of C bus fed back into A output |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| A IN | Input | Primary input (polyphonic) |
| B IN | Input | Secondary input (polyphonic) |
| A OUT | Output | A + B→A return + C→A return |
| B OUT | Output | B + A→B send |
| C OUT | Output | C bus output (one-sample delayed) |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Patch tips

- **Basic drone feedback**: DroneClone OUT → A IN; A OUT → DroneClone RTN; set B→A RETURN to 0, A→C and C→A to taste. The C-bus delay prevents DC runaway.
- **Cross-feedback between two oscillators**: DroneCore A → Send A IN; DroneCore B → Send B IN. A→B SEND blends B's character into A's output and vice versa.
- **Build up slowly**: start all sends at 0, raise A→B SEND first. Introduce C-bus last — C→A above 0.5 with A→C above 0.5 causes resonance.
- Combine with **FeedbackGovernor** on the C OUT for filtered, decaying feedback tails.

---

## Known pairings

- **↔ DroneClone RTN** — primary feedback loop use case
- **→ FeedbackGovernor** on C OUT for governed feedback tail
- **↔ WallConductor** — send/return for section bleed effects
