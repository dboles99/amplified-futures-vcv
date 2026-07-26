# Street Grid Clock — 12 HP (AF-02)

![Street Grid Clock panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/StreetGridClock.png)

The Branca series had no clock. A patch built only from these modules could not generate its own time — you had to borrow one from another plugin. Street Grid Clock closes that gap: RATE, SWING and BROWNOUT, four divided outputs, and an external clock input for when you would rather be the follower.

BROWNOUT is the one that is not standard issue. It models a mains grid dipping under load — the clock sags and recovers, always slowing and never speeding up, so the patch drags behind the beat rather than running ahead of it.

> **Note:** this page currently shows the flat panel export. A Rack-rendered screenshot will replace it once one has been taken, bringing it in line with the other module pages.

---

## Sound in 60 seconds

1. Add Street Grid Clock and [[Pulse]]. Patch **CLK** → Pulse **TRG**, and Pulse OUT to your interface.
2. Press **RUN**. Light a few steps on Pulse. You have rhythm with nothing borrowed.
3. Turn **RATE** up. Tempo climbs exponentially — the dial runs 20 BPM to 300, with the middle of the range around 75.
4. Raise **SWING** to about 50%. Odd-numbered pulses land late.
5. Raise **BROWNOUT**. Every couple of seconds the grid sags and pulls back up.
6. Patch **/4** into [[Sitar-Grid]] CLOCK. Two related tempos, one source.

---

## Signal flow

~~~text
RATE ──► bpm = 20 × 15^rate          (20 → 300 BPM, exponential)
              │
EXT CLK IN ──► when patched, external edges drive the phase instead
              │
RUN ──────────► gates the whole clock
              │
BROWNOUT ──► random sag, about one every two seconds at full depth
             recovery ≈250 ms · deepest sag runs at 60% of nominal
             at exactly 0 the RNG is never consulted
              │
         phase accumulator
              │
SWING ──► odd pulses arrive late by SWING × 0.5 of the period
              │
     ┌────────┬────┴───┬────────┐
     ▼        ▼        ▼        ▼
   CLK       /2       /4       /8

RESET button / RESET IN ──► phase to zero ──► RESET OUT
~~~

---

## Controls

| Control | Range | Default | What it does |
|---|---|---|---|
| RATE | 20–300 BPM | ≈59 BPM | Exponential. Most of the usable range sits in the lower half of the dial |
| SWING | 0–75% | 0 | Delays odd-numbered pulses by up to half a period |
| BROWNOUT | 0–100% | 0 | Sag frequency and depth. At zero the clock is exactly steady |
| RUN | toggle | — | Starts and stops |
| RESET | momentary | — | Phase to zero, and fires RESET OUT |

RATE, SWING and BROWNOUT each have an attenuverter (−1 to +1) and a CV input.

### Why BROWNOUT only ever slows down

A sag that ran *fast* would be a wobble, and would push the patch ahead of the beat. Dragging behind it is the musical behaviour, so the effect is deliberately one-directional.

At exactly zero the random number generator is not consulted and the sag state is forced to zero, making the timing bit-identical to a clean clock. Not "nearly steady" — exactly steady. A clock you cannot trust at zero is not a clock.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| RATE / SWING / BROWNOUT CV | Input | One per knob, each via its attenuverter |
| EXT CLK | Input | External clock. When patched it drives the phase instead of RATE |
| RESET | Input | Trigger — returns the phase to zero |
| CLK | Output | The main pulse |
| /2 · /4 · /8 | Output | Divided pulses, for polymetric layering |
| RESET | Output | Fires on reset, for chaining to other sequencers |

---

## Patch recipes

**The patch master.** CLK → [[Pulse]] TRG, /2 → [[Swarm-Core]] TRIG, /4 → [[Sitar-Grid]] CLOCK, /8 → [[Wall-Conductor]] COLLAPSE. Four related tempos from one module — percussion, texture, melody and structure, all locked.

**Dragging grid.** BROWNOUT 40%, SWING 0. The pulse is steady until it is not. Because sags only slow, the patch feels like it is being held back rather than falling apart.

**Swung against straight.** SWING 55% on CLK into Pulse, while /4 stays straight into [[Sitar-Grid]]. The percussion swings and the melodic line does not.

**Locked randomness.** CLK → [[Drift]] SYNC. Drift's random walk stops floating free and steps on the grid instead.

**Following, not leading.** A DAW clock or another module into EXT CLK. Street Grid Clock becomes a divider and swing processor for someone else's time.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Pulse]] | CLK → TRG. The primary destination |
| [[Swarm-Core]] | /2 → TRIG so insect calls sit against the percussion |
| [[Sitar-Grid]] | CLOCK, with RESET → RESET for phrase alignment |
| [[Drift]] | CLK → SYNC to lock the random walk to the grid |
| [[Wall-Conductor]] | /8 → COLLAPSE for structural events on a long cycle |

---

## See also

[[Pulse]] · [[Drift]] · [[Sitar-Grid]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/StreetGridClock.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/StreetGridClock.md)
