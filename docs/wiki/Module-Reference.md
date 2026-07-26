# Module Reference

Twelve modules. Every module has CV + attenuverter on every knob and V/OCT pass-through.

---

## Oscillators

### DroneCore — 8HP
![DroneCore](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/DroneCore.png)

Two-voice detuned oscillator core. PITCH, DETUNE (0–100¢), TIMBRE (sine to harmonic stack). Polyphonic.

→ **[[DroneCore]]** for full documentation

---

### DroneClone — 22HP
![DroneClone](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/DroneClone.png)

8-voice amplified string wall. MASS, TENSION, SHIMMER, JAWARI (sitar buzz), WEIGHT, DRIFT. CHOKE gate + RTN feedback input. Up to 128 simultaneous oscillators.

→ **[[DroneClone]]** for full documentation

---

### String Mass Core — 16HP
![StringMassCore](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/StringMassCore.png)

16-voice harmonic mass oscillator. Four modes: UNIS, HARM (odd-harmonic sections), JUST (Ptolemaic JI ratios), MICRO (spectral microtonality). 1/√N amplitude normalised.

→ **[[String-Mass-Core]]** for full documentation

---

## Pitch & Modulation

### Harmonic Pressure — 14HP
![HarmonicPressure](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/HarmonicPressure.png)

Harmonic series pitch CV generator. PITCH root, PARTIAL (starting harmonic), COUNT (output channels). JUST / EQUAL / MICRO tuning modes. Polyphonic V/OCT output.

→ **[[Harmonic-Pressure]]** for full documentation

---

### Drift — 12HP
![Drift](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Drift.png)

Slow random modulation source. RATE (0.01–10 Hz), WANDER (step size), SLEW (smoothing). Three simultaneous outputs: SMOOTH (slewed), STEP (raw), GATE (pulse on step).

→ **[[Drift]]** for full documentation

---

### Pulse — 12HP
![Pulse](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Pulse.png)

16-step no-wave percussion. 4×4 toggle grid, white noise synthesis. HIT level, DECAY time (8–500ms), METAL (LP filter), CRACK (transient burst).

→ **[[Pulse]]** for full documentation

---

## Routing & Mixing

### Send — 12HP
![Send](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Send.png)

2×2 cross-send feedback routing matrix. A→B send, B→A return, A→C / C→A internal C-bus (one-sample delayed). Polyphonic.

→ **[[Send]]** for full documentation

---

### Choke — 14HP
![Choke](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Choke.png)

4-channel performance mixer. Per-channel GAIN + TONE, fixed auto-pan spread, MUTE buttons. MAIN master with soft saturation. Stereo L/R out.

→ **[[Choke]]** for full documentation

---

### Wall Conductor — 22HP
![WallConductor](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/WallConductor.png)

Section-based performance conductor. DENSITY sweeps 4 channels in progressively. PRESSURE saturates. WIDTH spreads stereo. FEEDBACK loop. COLLAPSE gate with shaped RECOVERY.

→ **[[Wall-Conductor]]** for full documentation

---

### Mass Driver — 32HP (AF-01)
![MassDriver](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/MassDriver.png)

16-channel no-wave mixer. Per-channel GAIN + MUTE. DENSITY sweep, PRESSURE saturation, WIDTH stereo, FEEDBACK loop, COLLAPSE gate. Five outputs: OUT L/R, AUX L/R, SUM.

→ **[[Mass-Driver]]** for full documentation

---

## Effects

### Collapse Saturator — 12HP
![CollapseSat](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/CollapseSat.png)

Stereo drive/saturation. Three harmonic modes: ODD (tanh), EVEN (tape-like), FULL (hard clip). COLLAPSE gate instantly maxes drive with shaped RECOVERY. Sidechain input.

→ **[[Collapse-Saturator]]** for full documentation

---

### Feedback Governor — 12HP
![FeedbackGovernor](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/FeedbackGovernor.png)

Controlled feedback send/return. AMOUNT, TONE LP filter (100Hz–20kHz), DECAY per-pass attenuation. KILL button/gate zeros path instantly. DC blocker + ±10V safety limiter.

→ **[[Feedback-Governor]]** for full documentation

---

## Signal flow overview

```
[HarmonicPressure] ──► V/OCT poly ──► [StringMassCore] ──► [WallConductor] ──► [CollapseSat] ──► OUT
                                       [DroneCore]     ──►        ▲
                                       [DroneClone]    ──►        │
                                            ↕                     │
                                         [Send]              [FeedbackGovernor] (loop)
                                       [Drift] ──► CV ──► (any knob)
                                       [Pulse] ──► gates ──► COLLAPSE, MUTE, KILL
                                       [Choke] ──► section sub-mixing
                                       [MassDriver] ──► 16-channel alternative to WallConductor
```
