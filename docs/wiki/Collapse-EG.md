# Collapse EG — 8 HP

Compact attack/decay envelope with misfire and loop. Useful whenever the patch needs a standalone envelope for gates, triggers or stutters. `EnvCore` is unit-tested offline.

---

## Controls

| Control | Range | Notes |
|---|---|---|
| ATTACK | 0–1 | Exponential attack sweep |
| DECAY | 0–1 | Exponential decay sweep |
| CURVE | -1 to +1 | Log / linear / exponential shape |
| MISFIRE | 0–1 | 0 = textbook AD, higher values drop more triggers |
| LOOP | Off / On | Re-fires automatically when the cycle ends |

## Ports

| Port | Direction | Notes |
|---|---|---|
| GATE | Input | Gate trigger |
| TRIG | Input | Pulse trigger |
| ENV | Output | Main envelope, 0–10 V |
| INV | Output | Inverted envelope |
| EOC | Output | End-of-cycle pulse |

**Full parameter spec:** [`docs/modules/CollapseEG.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/CollapseEG.md)

See also: [[Module-Reference]] · [[Home]] · [[Installation]]
