# Drift — 12HP

Slow random modulation source. Random walk oscillator with slew smoothing — generates the slow, organic pitch and timbre drift characteristic of the Amplified Futures sound. Three outputs: SMOOTH (slewed), STEP (raw), GATE (pulse on each step).

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
- **Multiple Drift modules** at different rates (e.g. 0.05Hz, 0.13Hz, 0.37Hz — use prime-ratio rates) to avoid synchronisation. The wander never loops.
- **SYNC IN from Pulse** GATE: force steps to happen on beat, but randomise what the step value is.

---

## Known pairings

- **→ DroneCore** DETUNE/TIMBRE CV for slow oscillator drift
- **→ DroneClone** JAWARI/DRIFT/TENSION CV
- **→ Pulse** TRG IN for stochastic clock
- **→ WallConductor** DENSITY CV for slow channel sweeps
- **→ CollapseSat** COLLAPSE gate (GATE output) for periodic collapse events
