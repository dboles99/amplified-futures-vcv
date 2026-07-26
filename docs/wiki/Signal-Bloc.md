# Signal Bloc — 10 HP

CV glue: two attenuverter/offset channels, a precision three-input adder, and a buffered 1-to-3 mult. No character knob, by design — this is the module you reach for when you want to know exactly what happened to a voltage. Every section is polyphonic. `BlocCore` is unit-tested offline.

---

## Controls

| Control | Range | Notes |
|---|---|---|
| ATT 1–2 | -1 – +1 | Per-channel attenuvert, unity by default |
| OFF 1–2 | -5 – +5 V | Per-channel offset, zero by default |

## Ports

| Port | Direction | Notes |
|---|---|---|
| IN 1–2 | Input | Channel inputs; unpatched leaves a constant-voltage source |
| SUM A / B / C | Input | Adder inputs, read polyphonically |
| MULT | Input | Mult source |
| OUT 1–2 | Output | `IN x ATT + OFF` |
| SUM | Output | `A + B + C`, clamped to ±12 V |
| MULT 1–3 | Output | Buffered copies of MULT |

## Use it

1. **Scale or invert** — source into IN 1, ATT 1 sets depth (negative inverts), OFF 1 sets centre.
2. **Offset by hand** — leave IN 1 unpatched and OFF 1 alone drives OUT 1 as a constant.
3. **Sum three sources** — SUM A/B/C into SUM. Exact below the rails.
4. **Distribute one source** — MULT into MULT 1–3 without loading the original.

The three sections are independent; use any of them on their own.

## See also

[[Quad-VCA]] · [[Drift]] · [[Harmonic-Pressure]]

**Full parameter spec:** [`docs/modules/SignalBloc.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/SignalBloc.md)

See also: [[Module-Reference]] · [[Home]] · [[Installation]]
