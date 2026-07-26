# Quad VCA — 12HP

Four-channel VCA and mixer. Each channel has a level control and CV input, with the inputs normalled down the chain so it can behave as a compact mixer too. `VcaCore` is unit-tested offline.

---

## Controls

| Control | Range | Notes |
|---|---|---|
| LEVEL 1–4 | 0–1 | Per-channel level |
| PRESSURE | 0–1 | Saturation amount on the summed output |
| RESPONSE | Linear / Exponential | Shared response curve |

## Ports

| Port | Direction | Notes |
|---|---|---|
| IN 1–4 | Input | Audio channels |
| CV 1–4 | Input | Level modulation |
| OUT 1–4 | Output | Individual channel outputs |
| MIX | Output | Saturated mix output |

**Full parameter spec:** [`docs/modules/QuadVCA.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/QuadVCA.md)

See also: [[Module-Reference]] · [[Home]] · [[Installation]]
