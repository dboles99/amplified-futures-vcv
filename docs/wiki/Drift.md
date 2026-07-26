# Drift — 12HP

![Drift panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Drift.png)

Slow random modulation source. Random walk oscillator with slew smoothing — generates the slow, organic pitch and timbre drift characteristic of the Amplified Futures sound. Three simultaneous outputs: SMOOTH (slewed), STEP (raw), GATE (pulse on each step).

---

## Signal flow

```
SYNC IN ──► [forces immediate step]
             │
RATE ──────► [phase accumulator at 0.01–10 Hz]
             │ (each cycle: target += WANDER × random)
             │
SLEW ──────► [1-pole LP on target]
             │
             ├──► SMOOTH OUT  (slewed value × 5V, ±5V)
             ├──► STEP OUT    (raw target × 5V, ±5V)
             └──► GATE OUT    (10V, 5ms pulse on each step)

V/OCT IN ──────────────────────────────────────► V/OCT THRU
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| RATE | 0.01–10 Hz | Exponential — 0 = glacial (100s/cycle), 1 = fast (0.1s) |
| WANDER | 0–1 | Random walk step size — 0 = no movement, 1 = full ±1 jumps |
| SLEW | 0–1 | Smoothing LP cutoff — 0 = very smooth (0.1Hz), 1 = instant (1kHz) |

All knobs have attenuverter + CV input.

---

## Outputs

| Output | Character | Best use |
|---|---|---|
| SMOOTH | Gently slewed, continuous curve | PITCH, DETUNE, DENSITY, SPREAD — anything that needs gradual change |
| STEP | Immediate jumps at each step | TIMBRE, TENSION — abrupt timbral changes |
| GATE | 10V 5ms pulse on step | Clock input, COLLAPSE trigger, KILL gate |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| SYNC IN | Input | Rising edge forces immediate step (resets phase) |
| SMOOTH OUT | Output | Slewed value ±5V |
| STEP OUT | Output | Raw unslewed value ±5V |
| GATE OUT | Output | 10V 5ms pulse on every new step |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Patch tips

- **SMOOTH → DETUNE CV** on DroneCore: slow pitch drift ±6–12¢. RATE 0.1Hz, WANDER 0.3, SLEW 0.2.
- **SMOOTH → JAWARI CV** on DroneClone: breathing buzz. Keep atten below 0.3 to avoid full modulation swing.
- **GATE → TRG IN** on Pulse: Drift-clocked percussion. Very slow RATE = sparse hits.
- **STEP → any CV**: harder, more stochastic jumps — good for TIMBRE or TENSION.
- **Multiple Drift modules** at different rates — use prime-ratio rates (e.g. 0.05Hz, 0.13Hz, 0.37Hz) to avoid synchronisation. The wander never loops.
- **SYNC IN from Pulse** GATE: force steps to happen on beat, but randomise what the step value is.

## Rate reference

| RATE knob | Approximate frequency | Approximate period |
|---|---|---|
| 0.0 | 0.01 Hz | ~100 seconds per step |
| 0.1 | ~0.02 Hz | ~50 seconds |
| 0.25 | ~0.07 Hz | ~14 seconds |
| 0.5 | ~0.3 Hz | ~3 seconds |
| 0.75 | ~1.5 Hz | ~0.7 seconds |
| 1.0 | 10 Hz | 0.1 seconds |

For modulating slow drones: RATE 0.05–0.2 Hz. For irregular clock: RATE 0.3–1 Hz.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneCore]] | DETUNE/TIMBRE CV |
| [[DroneClone]] | JAWARI/DRIFT/TENSION CV |
| [[Pulse]] | TRG IN for stochastic clock |
| [[Wall-Conductor]] | DENSITY CV for slow channel sweeps |
| [[Collapse-Saturator]] | GATE output → COLLAPSE IN for periodic events |
| [[Harmonic-Pressure]] | SMOOTH → PITCH CV for root wander |

---

## See also

[[DroneCore]] · [[DroneClone]] · [[Pulse]] · [[Wall-Conductor]]
