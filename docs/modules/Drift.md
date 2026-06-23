# Drift — 12HP

Slow random modulation source. Random walk oscillator with slew smoothing — generates the slow, organic pitch and timbre drift characteristic of the Amplified Futures sound. Three outputs: SMOOTH (slewed), STEP (raw), GATE (pulse on each step).

---

## Signal flow

```text
SYNC IN ──► [rising edge forces immediate step, resets phase]
             │
RATE ──────► [phase accumulator 0.01–10 Hz, exponential]
             │ (each new cycle: target += WANDER × random[-1,1], clamped ±1)
             │
SLEW ──────► [1-pole LP toward target, cutoff 0.1–1000 Hz]
             │
             ├──► SMOOTH OUT  (slewed value × 5 V, range ±5 V)
             ├──► STEP OUT    (raw target × 5 V, unslewed, ±5 V)
             └──► GATE OUT    (10 V, 5 ms pulse on each new step)

V/OCT IN ──────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| RATE | 0 | 0–1 | 0.35 | Step rate, exponential: 0 = 0.01 Hz (100 s/cycle), 1 = 10 Hz |
| WANDER | 1 | 0–1 | 0.5 | Random walk step size per step: 0 = frozen, 1 = full ±1 jumps |
| SLEW | 2 | 0–1 | 0.5 | Smoothing: 0 = very smooth (0.1 Hz LP), 1 = instant (1 kHz LP) |
| RATE ATTEN | 3 | −1 to +1 | 0 | Attenuverter for RATE CV |
| WANDER ATTEN | 4 | −1 to +1 | 0 | Attenuverter for WANDER CV |
| SLEW ATTEN | 5 | −1 to +1 | 0 | Attenuverter for SLEW CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| RATE CV | Input | CV | CV for RATE (scaled by RATE ATTEN) |
| WANDER CV | Input | CV | CV for WANDER |
| SLEW CV | Input | CV | CV for SLEW |
| SYNC IN | Input | Gate | Rising edge forces immediate step and resets phase accumulator |
| V/OCT IN | Input | CV | Pass-through |
| SMOOTH OUT | Output | CV | Slewed random value ±5 V — use for smooth pitch/filter modulation |
| STEP OUT | Output | CV | Raw unslewed step value ±5 V — harder, more stochastic jumps |
| GATE OUT | Output | Gate | 10 V 5 ms pulse on every new step |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | RATE | CC 14 |
| 1 | WANDER | CC 15 |
| 2 | SLEW | CC 16 |

Use CC 14 mapped to RATE for live tempo modulation of the wander speed. Automating WANDER (CC 15) from 0 to 1 turns a frozen drift into a wild random walk in real time.

---

## Recommended configurations

**Glacial Pitch Drift** — RATE 0.1 (≈ 0.05 Hz), WANDER 0.25, SLEW 0.1. SMOOTH OUT → DroneCore DETUNE CV. Pitch beating changes over minutes. Barely perceptible motion, but never static.

**Breathing Timbre** — RATE 0.25 (≈ 0.2 Hz), WANDER 0.5, SLEW 0.3. SMOOTH OUT → DroneClone JAWARI CV (JAWARI ATTEN +0.3). The buzz character rises and falls slowly.

**Stochastic Clock** — RATE 0.6 (≈ 1–2 Hz), WANDER 1.0, SLEW 0.9 (almost instant). GATE OUT → Pulse TRG IN. Each gate fires at irregular intervals from 0.05–1 second. Irregular, human-feeling percussion timing.

**Compound Drift** — Three Drift instances at RATE 0.12, 0.19, 0.31 Hz (approximate prime ratios). Each modulates a different parameter. The three rates never synchronise — the patch never repeats.

---

## Basic setup — sound in 60 seconds

1. Add Drift to your patch.
2. Patch Drift SMOOTH OUT → DroneCore DETUNE CV. Set DroneCore DETUNE ATTEN to +0.4.
3. Set Drift RATE to 0.25, WANDER 0.4, SLEW 0.2.
4. DroneCore DETUNE will now slowly wander up and down over a ±40¢ range.
5. Patch Drift GATE OUT → Pulse TRG IN for synced-ish percussion.
6. A second Drift at RATE 0.08 for a slower layer adds compound movement.

---

## How-tos

### Prime-ratio multi-drift

- Add 3 Drift instances. Set RATE knobs to positions corresponding to ≈ 0.05 Hz, 0.08 Hz, 0.13 Hz.
- Route each SMOOTH to different targets: DETUNE, JAWARI, SHIMMER.
- Because the rates are in approximate prime ratios (5:8:13), the three wanders never align.
- The patch never settles into a repeating pattern.

### Sync to clock for stepped randomness

- Patch any clock gate → Drift SYNC IN.
- On each clock pulse, Drift takes an immediate new random step.
- WANDER 0.6 with SLEW 0.9 (instant) makes each clock beat a sharp random jump.
- Use STEP OUT → PITCH CV for pitched stochastic melody.

### Feedback rate modulation

- Drift SMOOTH OUT → FeedbackGovernor TONE CV (TONE ATTEN +0.5).
- RATE 0.15, WANDER 0.5, SLEW 0.2.
- The feedback path slowly darkens and brightens, changing the character of the feedback tail over time.

### Autonomous density sweep

- Drift SMOOTH OUT → WallConductor DENSITY CV (DENSITY ATTEN +0.6).
- RATE 0.03 (very slow), WANDER 0.3, SLEW 0.05 (very smooth).
- The mix slowly expands and contracts over 30–60 second arcs without any manual intervention.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| SPACE (static-witness) | Very slow RATE + low WANDER — barely perceptible drift, the patch as witness to its own motion |
| PRESSURE (rent-pressure) | RATE and WANDER both high — relentless unpredictable pressure on every connected parameter |
| CARE (mutual-aid) | SMOOTH → TONE on FeedbackGovernor — the drift softens and warms the feedback loop |
| COLLECTIVE (collective-refusal) | Multiple Drift instances at prime rates driving different parameters — autonomous collective motion |
| FAILURE (managed-collapse) | GATE OUT driving collapse events — random structural failures in the signal chain |

---

## Known pairings

| Module | Role |
| --- | --- |
| DroneCore | SMOOTH → DETUNE or TIMBRE CV for slow oscillator drift |
| DroneClone | SMOOTH → JAWARI, DRIFT, or TENSION CV |
| Pulse | GATE → TRG IN for stochastic clock; STEP → HIT CV for random velocity |
| WallConductor | SMOOTH → DENSITY CV for autonomous section sweeps |
| FeedbackGovernor | SMOOTH → TONE CV for feedback tone animation |
| CollapseSat | GATE → COLLAPSE IN for periodic timed collapse events |
