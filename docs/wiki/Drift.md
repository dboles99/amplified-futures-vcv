# Drift — 12 HP

![Drift in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Drift.png)

Slow random modulation source — a random walk with slew smoothing, and the module that keeps everything else from sitting still. It emits three views of the same walk simultaneously: SMOOTH for continuous movement, STEP for the raw jumps, and GATE for a trigger on each new step. Drift is rarely the point of a patch; it is what makes the rest of the patch sound alive.

---

## Sound in 60 seconds

1. Add Drift alongside any module with a CV input — [[DroneCore]]'s TIMBRE is a good first target.
2. Patch **SMOOTH** → TIMBRE CV. Set that attenuverter to about +0.4.
3. Drift starts at RATE 35%, which is roughly 1 Hz. The timbre already moves.
4. Turn **RATE** down towards 0. At 0% a step lasts a minute and a half.
5. Turn **SLEW** down. SMOOTH stops gliding and starts jumping, converging on what STEP already outputs.

---

## Signal flow

~~~text
        phase accumulator at RATE Hz (0.01 → 10 Hz, exponential)
                          │
        on wrap ──► new random target, step size scaled by WANDER
                          │
                          ├──────────────► STEP OUT   (±5 V, raw)
                          ├─ 1-pole LP ──► SMOOTH OUT (±5 V, slewed)
                          │   SLEW: 1000 Hz (instant) → 0.1 Hz (glacial)
                          └──────────────► GATE OUT   (10 V, 5 ms pulse)

SYNC IN (trigger) ──► reset the phase accumulator
V/OCT IN ────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![Drift panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Drift.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| RATE | 0–100% | 35% | Step rate, exponential: 0% is 0.01 Hz, 100% is 10 Hz |
| WANDER | 0–100% | 50% | How far each new step may move from the last |
| SLEW | 0–100% | 50% | Smoothing on SMOOTH: 0% is near-instant, 100% is around 0.1 Hz |

Every knob has an attenuverter (−1 to +1) and a CV input.

The RATE curve is exponential, so the useful range is not where you might expect: 0% is one step every hundred seconds, 50% is about 0.3 Hz, and the top of the dial is audio-adjacent. For structural drift under a drone, stay below 30%.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| SYNC | Input | Trigger — resets the phase accumulator, for locking Drift to a clock |
| RATE / WANDER / SLEW CV | Input | One per knob, each via its attenuverter |
| SMOOTH | Output | ±5 V, slewed. The everyday output |
| STEP | Output | ±5 V, raw stepped values with no smoothing |
| GATE | Output | 10 V, 5 ms pulse on each new step |
| V/OCT IN → THRU | In / Out | Pass-through, so Drift can sit inline in a pitch chain |

---

## Patch recipes

**Breathing timbre.** SMOOTH → [[DroneCore]] TIMBRE CV or [[DroneClone]] JAWARI CV, attenuverter +0.4, RATE 20%. Harmonic content that fades in and out over ten-second cycles.

**Stepped transposition.** STEP → [[Harmonic-Pressure]] PITCH CV, attenuverter +0.5, RATE 10%. The root moves in discrete jumps while the series above it stays internally in tune.

**Stochastic percussion.** GATE → [[Pulse]] TRG, RATE 60%, WANDER high. Irregular triggering that never settles into a bar. Add [[Swarm-Core]] on the same gate for insect calls at the same irregular spacing.

**Clocked drift.** A steady clock → SYNC, RATE set so a step lasts roughly one bar. The randomness stays, but it lands on the grid.

**Structural pressure.** SMOOTH → [[Wall-Conductor]] DENSITY CV at low RATE. The wall thickens and thins over minutes without anyone touching it.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneCore]] | SMOOTH → TIMBRE CV for slow harmonic wander |
| [[DroneClone]] | SMOOTH → JAWARI or DRIFT CV for a buzz that breathes |
| [[String-Mass-Core]] | SMOOTH → SPREAD CV for chorus without a chorus module |
| [[Wall-Conductor]] | SMOOTH → DENSITY CV for slow structural change |
| [[Pulse]] | GATE → TRG for irregular triggering |
| [[Harmonic-Pressure]] | STEP → PITCH CV to move the root in jumps |

---

## See also

[[Pulse]] · [[Wall-Conductor]] · [[DroneCore]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/Drift.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/Drift.md)
