# Signal Bloc — 10HP

CV glue: two attenuverter/offset channels, a precision three-input adder, and a buffered 1-to-3 mult. No character knob, by design — this is the module you reach for when you want to know exactly what happened to a voltage. Everything is polyphonic. `BlocCore` is unit-tested offline.

---

## Signal flow

```text
IN 1 ──► x ATT 1 ──► + OFF 1 ──► OUT 1
IN 2 ──► x ATT 2 ──► + OFF 2 ──► OUT 2

SUM A ─┐
SUM B ─┼──► precision adder ──► SUM
SUM C ─┘

MULT ──► buffer ─┬──► MULT 1
                 ├──► MULT 2
                 └──► MULT 3

all sections clamp to ±12 V
```

An unpatched channel input leaves that channel as a constant-voltage source, so ATT/OFF doubles as a manual offset generator.

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| ATT 1 | 0 | -1 – +1 | 1 | Channel 1 attenuvert |
| OFF 1 | 1 | -5 – +5 V | 0 V | Channel 1 offset |
| ATT 2 | 2 | -1 – +1 | 1 | Channel 2 attenuvert |
| OFF 2 | 3 | -5 – +5 V | 0 V | Channel 2 offset |

Unity and zero are the defaults: the module is transparent until you ask it not to be.

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| IN 1 | Input | CV / Audio | Channel 1 input; unpatched leaves a constant |
| IN 2 | Input | CV / Audio | Channel 2 input; unpatched leaves a constant |
| SUM A | Input | CV / Audio | Adder input A |
| SUM B | Input | CV / Audio | Adder input B |
| SUM C | Input | CV / Audio | Adder input C |
| MULT | Input | CV / Audio | Mult source |
| OUT 1 | Output | CV / Audio | `IN 1 x ATT 1 + OFF 1` |
| OUT 2 | Output | CV / Audio | `IN 2 x ATT 2 + OFF 2` |
| SUM | Output | CV / Audio | `A + B + C`, clamped |
| MULT 1 | Output | CV / Audio | Buffered copy |
| MULT 2 | Output | CV / Audio | Buffered copy |
| MULT 3 | Output | CV / Audio | Buffered copy |

The adder reads its inputs polyphonically, so a mono cable into a poly sum feeds every channel rather than only the first.

---

## Polyphony

Channel count follows the input on every section, and the arithmetic is looped per channel. On a mult and an adder polyphony genuinely falls out for free, which is why the series' "no polyphony unless it is free" rule permits it here.

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | ATT 1 | CC 20 |
| 1 | OFF 1 | CC 21 |
| 2 | ATT 2 | CC 22 |
| 3 | OFF 2 | CC 23 |

---

## Recommended configurations

**Invert a modulation source** — Patch into IN 1, set ATT 1 to -1, OFF 1 to 0. OUT 1 is the exact inversion; a signal and its inversion sum to zero.

**Bipolar to unipolar** — A ±5 V LFO into IN 1 with ATT 1 at 0.5 and OFF 1 at +2.5 V gives 0–5 V.

**Manual offset generator** — Leave IN 1 unpatched. OFF 1 alone drives OUT 1 as a constant, useful as a tuning or bias voltage.

**Three-source modulation blend** — LFO, envelope and offset into SUM A/B/C. The adder is exact below the rails, so what you patch is what you get.

**Clock or V/OCT distribution** — MULT into the buffered outputs when one source has to reach three destinations without loading.

---

## Basic setup

1. Add Signal Bloc to your patch.
2. For scaling: patch a source to IN 1, set ATT 1 for depth and OFF 1 for centre.
3. For summing: patch up to three sources to SUM A/B/C and take SUM.
4. For distribution: patch one source to MULT and take MULT 1–3.
5. Sections are independent — use any of them without patching the others.
