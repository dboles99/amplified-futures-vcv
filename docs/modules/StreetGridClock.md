# Street Grid Clock (AF-02) — 12 HP

Master clock for the Branca series. The series previously had no clock at all — a patch built only from these modules could not generate its own time, and needed a clock borrowed from another plugin. This is that module.

RATE, SWING and BROWNOUT, each with attenuverter and CV. RUN and RESET buttons. Four clock outputs at /1, /2, /4 and /8, plus a RESET output. External clock input for slaving to something else.

BROWNOUT is a mains-sag metaphor: the grid dips under load and recovers. It only ever slows the clock, never speeds it up, and always returns to nominal.

---

## Signal flow

```text
RATE knob/CV ──► bpm = 20 × 15^rate01        (20 → 300 BPM)
                          │
EXT CLK IN ──► if patched, external edges drive the phase instead
                          │
RUN (toggle) ──► gate the whole clock
                          │
BROWNOUT ──► random sag events, ~1 per 2 s at full depth
             exponential recovery, ≈250 ms time constant
             deepest sag runs at 60% of nominal
             at exactly 0 the RNG is never consulted — bit-identical
             to a clean clock
                          │
                     phase accumulator
                          │
SWING ──► odd-numbered pulses arrive late by (SWING × 0.5) of the period
                          │
        ┌─────────┬───────┴───┬──────────┐
        ▼         ▼           ▼          ▼
      CLK        /2          /4         /8

RESET button / RESET IN ──► phase to zero ──► RESET OUT
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| RATE | 0 | 0–1 | 0.4 | Exponential: `bpm = 20 × 15^rate`. 0 = 20 BPM, 0.4 ≈ 59 BPM, 1 = 300 BPM |
| SWING | 1 | 0–75% | 0 | Delays odd-numbered pulses by SWING × 0.5 of the period |
| BROWNOUT | 2 | 0–100% | 0 | Sag probability and depth. At 0, timing is exactly steady |
| RATE ATTEN | 3 | −1 to +1 | 0 | Attenuverter for RATE CV |
| SWING ATTEN | 4 | −1 to +1 | 0 | Attenuverter for SWING CV |
| BROWNOUT ATTEN | 5 | −1 to +1 | 0 | Attenuverter for BROWNOUT CV |
| RUN | 6 | Toggle | — | Starts and stops the clock |
| RESET | 7 | Momentary | — | Returns the phase to zero and fires RESET OUT |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| RATE CV | Input | CV | Scaled by RATE ATTEN |
| SWING CV | Input | CV | Scaled by SWING ATTEN |
| BROWNOUT CV | Input | CV | Scaled by BROWNOUT ATTEN |
| EXT CLK | Input | Gate | External clock. When patched, drives the phase instead of RATE |
| RESET | Input | Trigger | Returns the phase to zero |
| CLK | Output | Gate | Main clock pulse |
| /2 | Output | Gate | Half rate |
| /4 | Output | Gate | Quarter rate |
| /8 | Output | Gate | Eighth rate |
| RESET | Output | Trigger | Fires on reset, for chaining |

---

## BROWNOUT design note

The behaviour is deliberately asymmetric. A brownout that ran *fast* would be a wobble, and would push the patch ahead of the beat rather than dragging behind it, so sag only ever reduces the rate. At the deepest point the clock runs at 60% of nominal, recovering with roughly a 250 ms time constant.

At exactly zero the random number generator is not consulted at all and the sag state is forced to zero, so timing is bit-identical to a clean clock. "Nearly steady" would defeat the purpose — nobody patches a clock they cannot trust.

`ClockCore` is host-independent and unit-tested offline in `tests/test_clock_core.cpp`.

---

## Recommended configurations

**Steady grid** — RATE 0.4 (≈59 BPM), SWING 0, BROWNOUT 0. An exactly steady clock. Use as the patch master.

**Dragging grid** — BROWNOUT 40%, everything else steady. Occasional sags pull the patch behind the beat and recover, giving a mechanical unsteadiness that is not random jitter.

**Swung percussion** — SWING 55%, feeding Pulse TRG. The off-beats land late; the /4 output stays straight for anything that needs the underlying pulse.

**Polymetric layers** — CLK → Pulse, /2 → SwarmCore TRG, /4 → SitarGrid CLOCK, /8 → a WallConductor COLLAPSE gate. Four related tempos from one module.

---

## Basic setup — sound in 60 seconds

1. Add Street Grid Clock and Pulse.
2. Patch CLK → Pulse TRG. Patch Pulse OUT → your audio interface.
3. Press RUN. Light some Pulse steps. You have rhythm with nothing borrowed from another plugin.
4. Turn RATE up. The tempo climbs exponentially towards 300 BPM.
5. Raise SWING to about 50%. Off-beats fall late.
6. Raise BROWNOUT. The grid begins to sag and recover.

---

## Known pairings

| Module | Role |
| --- | --- |
| Pulse | CLK → TRG, the primary destination |
| SwarmCore | /2 → TRIG so insect calls land against the percussion |
| SitarGrid | CLOCK input, with RESET → RESET for phrase alignment |
| Drift | Clock → SYNC to lock the random walk to the grid |
| WallConductor | /8 → COLLAPSE for structural events on a long cycle |
