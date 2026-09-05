# Module Reference

Nineteen modules. Every CV-able knob uses the satellite attenuverter + jack layout, with Swarm Core DECAY as the documented exception. V/OCT pass-through stays consistent.

---

## Oscillators

### DroneCore — 8HP
![DroneCore](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/DroneCore.png)

Two-voice detuned oscillator core. PITCH, DETUNE (0–100¢), TIMBRE (sine to harmonic stack). Polyphonic.

→ **[[DroneCore]]** for full documentation

---

### DroneClone — 26HP
![DroneClone](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/DroneClone.png)

8-voice amplified string wall. MASS, TENSION, SHIMMER, JAWARI (sitar buzz), WEIGHT, DRIFT. CHOKE gate + RTN feedback input. Up to 128 simultaneous oscillators.

→ **[[DroneClone]]** for full documentation

---

### String Mass Core — 16HP
![StringMassCore](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/StringMassCore.png)

16-voice harmonic mass oscillator. Four modes: UNIS, HARM (odd-harmonic sections), JUST (Ptolemaic JI ratios), MICRO (spectral microtonality). 1/√N amplitude normalised.

→ **[[String-Mass-Core]]** for full documentation

---

## Pitch & Modulation

### Harmonic Pressure — 14HP
![HarmonicPressure](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/HarmonicPressure.png)

Harmonic series pitch CV generator. PITCH root, PARTIAL (starting harmonic), COUNT (output channels). JUST / EQUAL / DRIFT tuning modes. Polyphonic V/OCT output.

→ **[[Harmonic-Pressure]]** for full documentation

---

### Drift — 12HP
![Drift](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Drift.png)

Slow random modulation source. RATE (0.01–10 Hz), WANDER (step size), SLEW (smoothing). Three simultaneous outputs: SMOOTH (slewed), STEP (raw), GATE (pulse on step).

→ **[[Drift]]** for full documentation

---

## Clock & Sequencing

### Street Grid Clock — 12HP (AF-02)
![Street Grid Clock](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/StreetGridClock.png)

Master clock — the series previously had none. RATE (20–300 BPM, exponential), SWING (delays odd pulses by up to half a period), BROWNOUT (mains-sag metaphor: dips under load and recovers, only ever slowing). CLK, /2, /4, /8 and RESET outputs, plus EXT CLK input for slaving. At BROWNOUT 0 the timing is bit-identical to a clean clock.

→ **[[Street-Grid-Clock]]** for full documentation

---

### Pulse — 12HP
![Pulse](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Pulse.png)

16-step no-wave percussion. 4×4 toggle grid, white noise synthesis. HIT level, DECAY time (8–500ms), METAL (body filter and grit), CRACK (4ms transient burst).

→ **[[Pulse]]** for full documentation

---

### Ratchet — 8HP (AF-03)

Trigger burst generator. Measures the incoming trigger period, then subdivides it into 1–8 repeats. SPREAD accelerates or decelerates the burst, PROB thins it, and END fires once when the burst closes. `RatchetCore` is unit-tested offline.

→ **[[Ratchet]]** for full documentation

---

### Sitar Grid — 42HP
![Sitar Grid](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/SitarGrid.png)

Modal string-resonance sequencer. Three independent sequencing brains — PITCH (raga-quantised), RES (timbral, own clock division), RIFF (articulation) — driving a Karplus-Strong string through a jawari nonlinear bridge, with an 8-voice sympathetic bank and a chikari drone. JHALA breakdown state machine. Six ragas. Separate MAIN, DRONE and SYMP outputs.

→ **[[Sitar-Grid]]** for full documentation

---

## Sampling

### Swarm Core — 18HP
![Swarm Core](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/SwarmCore.png)

Bio-acoustic insect sample engine over the InsectSet32 bank (CC-BY 4.0). Specimen mode plays one pitched voice per trigger; Swarm mode fires up to 8, scattered in time and detuned across a fixed stereo spread. Note the 2–5 second bank load on first patch load.

→ **[[Swarm-Core]]** for full documentation

---

## Routing & Mixing

### Send — 12HP
![Send](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Send.png)

2×2 cross-send feedback routing matrix. A→B send, B→A return, A→C / C→A internal C-bus (one-sample delayed). Polyphonic.

→ **[[Send]]** for full documentation

---

### Choke — 18HP
![Choke](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/Choke.png)

4-channel performance mixer. Per-channel GAIN + TONE, fixed auto-pan spread, MUTE buttons. MAIN master with soft saturation. Stereo L/R out.

→ **[[Choke]]** for full documentation

---

### Wall Conductor — 22HP
![WallConductor](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/WallConductor.png)

Section-based performance conductor. DENSITY sweeps 4 channels in progressively. PRESSURE saturates. WIDTH spreads stereo. FEEDBACK loop. COLLAPSE gate with shaped RECOVERY.

→ **[[Wall-Conductor]]** for full documentation

---

### Mass Driver — 32HP (AF-01)
![MassDriver](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/MassDriver.png)

16-channel no-wave mixer. Per-channel GAIN + MUTE. DENSITY sweep, PRESSURE saturation, WIDTH stereo, FEEDBACK loop, COLLAPSE gate. Five outputs: OUT L/R, AUX L/R, SUM.

→ **[[Mass-Driver]]** for full documentation

---

## Effects

### Collapse Saturator — 16HP
![CollapseSat](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/CollapseSat.png)

Stereo drive/saturation. Three harmonic modes: ODD (tanh), EVEN (tape-like), FULL (hard clip). COLLAPSE gate instantly maxes drive with shaped RECOVERY. Sidechain input.

→ **[[Collapse-Saturator]]** for full documentation

---

### Feedback Governor — 12HP
![FeedbackGovernor](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/FeedbackGovernor.png)

Controlled feedback send/return. AMOUNT, TONE LP filter (100Hz–20kHz), DECAY per-pass attenuation. KILL button/gate zeros path instantly. DC blocker + ±10V safety limiter.

→ **[[Feedback-Governor]]** for full documentation

---

## Utilities

### Collapse EG — 8HP

Standalone attack/decay envelope with CURVE, MISFIRE and LOOP. A compact utility envelope for gates, triggers and stutters.

→ **[[Collapse-EG]]** for full documentation

---

### Quad VCA — 12HP

Four-channel VCA and mixer. Per-channel level controls and CV inputs, with chain normalling and a PRESSURE-saturated mix output.

→ **[[Quad-VCA]]** for full documentation

---

### Signal Bloc — 10HP (AF-06)

CV glue with no character knob: two attenuverter/offset channels, a precision three-input adder, and a buffered 1-to-3 mult. Every section is polyphonic and clamps to ±12 V. `BlocCore` is unit-tested offline.

→ **[[Signal-Bloc]]** for full documentation

---

## Signal flow overview

```text
[HarmonicPressure] ──► V/OCT poly ──► [StringMassCore] ──► [WallConductor] ──► [CollapseSat] ──► OUT
                                       [DroneCore]     ──►        ▲
                                       [DroneClone]    ──►        │
                                            ↕                     │
                                         [Send]              [FeedbackGovernor] (loop)
                                       [SitarGrid]     ──►        │   MAIN / DRONE / SYMP as three sources
                                       [SwarmCore]     ──►        │   texture layer, stereo
                                       [Drift] ──► CV ──► (any knob)
                                       [Pulse] ──► gates ──► COLLAPSE, MUTE, KILL, TRG
                                       [Choke] ──► section sub-mixing
                                       [MassDriver] ──► 16-channel alternative to WallConductor
                                                        AUX (pre-drive) ──► [CollapseSat] parallel path

[SitarGrid] PITCH CV + GATE ──► can drive [StringMassCore] / [DroneClone] instead of its own string
[SwarmCore] CV OUT ──────────► envelope of its own swarm, for driving anything
```
