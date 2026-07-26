# Collapse EG — 8HP

Attack/decay envelope with misfire and loop. The module gives the series a standalone envelope for shaping gates or arbitrary triggers. EnvCore is unit-tested offline.

---

## Signal flow

```text
GATE / TRIG ──► EnvCore trigger
                   │
            ATTACK / DECAY / CURVE
                   │
             MISFIRE (0 = textbook AD)
                   │
              LOOP (optional re-fire)
                   │
           ENV ─────────► 0–10 V envelope
           INV ─────────► inverted envelope
           EOC ─────────► end-of-cycle pulse
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| ATTACK | 0 | 0–1 | 0.1 | Exponential attack time sweep |
| DECAY | 1 | 0–1 | 0.4 | Exponential decay time sweep |
| CURVE | 2 | -1 to +1 | 0 | Shape: log, linear, exponential |
| MISFIRE | 3 | 0–1 | 0 | 0 = precise AD, 1 = maximum drop rate |
| LOOP | 4 | Off/On | Off | Re-triggers automatically when the cycle ends |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| GATE | Input | Gate | Trigger from a held gate |
| TRIG | Input | Trigger | Trigger from a short pulse |
| ENV | Output | Audio/CV | Main envelope output, 0–10 V |
| INV | Output | Audio/CV | Inverted envelope output |
| EOC | Output | Trigger | End-of-cycle pulse |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | ATTACK | CC 14 |
| 1 | DECAY | CC 15 |
| 2 | CURVE | CC 16 |
| 3 | MISFIRE | CC 17 |
| 4 | LOOP | CC 18 |

---

## Recommended configurations

**Snappy AD** — ATTACK 0.05, DECAY 0.2, CURVE 0, MISFIRE 0. The clean envelope case.

**Broken repeat** — ATTACK 0.1, DECAY 0.4, CURVE -0.2, MISFIRE 0.35, LOOP on. A slightly unreliable loop that never quite feels mechanical.

**Long fade** — ATTACK 0.25, DECAY 0.8, CURVE +0.35, MISFIRE 0. The long version for collapse tails and pad shaping.

---

## Basic setup

1. Add Collapse EG to your patch.
2. Patch a gate or trigger into TRIG.
3. Patch ENV into a VCA or filter cutoff.
4. Turn DECAY up for a longer fall time.
5. Enable LOOP if you want a self-retriggering envelope.

