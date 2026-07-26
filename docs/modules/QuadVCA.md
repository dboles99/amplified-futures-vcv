# Quad VCA — 12HP

Four-channel VCA and mixer. Each channel has a level control and CV input, with the inputs normalled down the chain so the module can work as a compact mixer as well as a VCA bank. PRESSURE adds saturation on the summed output. VcaCore is unit-tested offline.

---

## Signal flow

```text
IN 1 ──► LEVEL 1 ──► OUT 1
IN 2 ──► LEVEL 2 ──► OUT 2
IN 3 ──► LEVEL 3 ──► OUT 3
IN 4 ──► LEVEL 4 ──► OUT 4

CV 1–4 modulate each level
patched inputs travel down the chain if later channels are empty

sum of unpatched outs ──► PRESSURE saturator ──► MIX
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| LEVEL 1 | 0 | 0–1 | 1 | Channel 1 level |
| LEVEL 2 | 1 | 0–1 | 1 | Channel 2 level |
| LEVEL 3 | 2 | 0–1 | 1 | Channel 3 level |
| LEVEL 4 | 3 | 0–1 | 1 | Channel 4 level |
| PRESSURE | 4 | 0–1 | 0 | Sum saturation amount |
| RESPONSE | 5 | Linear / Exponential | Linear | Shared response curve |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| IN 1 | Input | Audio | Channel 1 input |
| IN 2 | Input | Audio | Channel 2 input |
| IN 3 | Input | Audio | Channel 3 input |
| IN 4 | Input | Audio | Channel 4 input |
| CV 1 | Input | CV | Level modulation for channel 1 |
| CV 2 | Input | CV | Level modulation for channel 2 |
| CV 3 | Input | CV | Level modulation for channel 3 |
| CV 4 | Input | CV | Level modulation for channel 4 |
| OUT 1 | Output | Audio | Channel 1 output |
| OUT 2 | Output | Audio | Channel 2 output |
| OUT 3 | Output | Audio | Channel 3 output |
| OUT 4 | Output | Audio | Channel 4 output |
| MIX | Output | Audio | Saturated mix output |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | LEVEL 1 | CC 14 |
| 1 | LEVEL 2 | CC 15 |
| 2 | LEVEL 3 | CC 16 |
| 3 | LEVEL 4 | CC 17 |
| 4 | PRESSURE | CC 18 |
| 5 | RESPONSE | CC 19 |

---

## Recommended configurations

**Four VCA bank** — LEVEL 1–4 at 1.0, PRESSURE 0. The clean quad VCA case.

**Mini mixer** — Patch one source into IN 1, leave the rest empty, then use the chain normalling to create a four-channel split with separate gains.

**Saturated blend** — Feed four related sources, turn PRESSURE up to taste, and use MIX as the master sum.

---

## Basic setup

1. Add Quad VCA to your patch.
2. Patch sources to IN 1–4.
3. Patch CV to the channel you want to animate.
4. Take the individual OUT jacks where you want channel isolation.
5. Use MIX when you want the summed, pressure-controlled output.

