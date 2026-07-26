# Send — 12HP

![Send panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Send.png)

2×2 cross-send feedback routing matrix. A→B send, B→A return, A→C / C→A internal feedback bus (C-bus). The C-bus is one-sample delayed for safe self-oscillation without DC runaway. Polyphonic.

---

## Signal flow

```
A IN ──►──── A→B SEND ────────────────────────► B OUT
        └──► A→C DEPTH ──► [C bus, 1-sample delay] ──► C OUT
                                                     ▲
B IN ──►──── B→A RETURN ─────────────────────────────┤
        └──────────────────────────────────────────── C→A RETURN ──► A OUT
```

A OUT = A IN + (B→A return) + (C→A return)
B OUT = B IN + (A→B send)
C OUT = C-bus (one sample delayed)

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

## The C-bus

The C-bus is a one-sample-delayed internal feedback path. This delay breaks the algebraic loop that would otherwise cause instantaneous runaway or NaN values in the audio path. It makes the C-bus safe for self-patching: feed A→C and C→A simultaneously at moderate levels for controlled resonance without instability.

Build up the C-bus slowly:
1. Start with all sends at 0
2. Raise A→B SEND to 0.4 — cross-blend between A and B
3. Introduce A→C to 0.3 — C-bus begins accumulating
4. Raise C→A to 0.3 — C-bus feeds back into A
5. Monitor amplitude — above 0.6/0.6 on both C settings the loop resonates

---

## Patch tips

- **Basic drone feedback**: DroneClone OUT → A IN; A OUT → DroneClone RTN; A→C and C→A to taste. The C-bus delay prevents DC runaway.
- **Cross-feedback between two oscillators**: DroneCore A → Send A IN; DroneCore B → Send B IN. A→B SEND blends B's character into A's output and vice versa.
- **FeedbackGovernor on C OUT**: route C OUT → FeedbackGovernor SEND → FeedbackGovernor RETURN → B IN. Adds filtering and per-pass decay to the C-bus — the governed feedback loop.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | RTN bus for cross-feedback |
| [[Feedback-Governor]] | C OUT → SEND for governed feedback tail |
| [[Wall-Conductor]] | Send/return for section bleed effects |

---

## See also

[[Feedback-Governor]] · [[DroneClone]] · [[Playbooks]]
