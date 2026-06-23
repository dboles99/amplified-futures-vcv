# Sitar Grid — 42HP

Modal string-resonance sequencer. Three independent sequencing brains — PITCH (raga-quantised pitch), RES (timbral resonance), RIFF (articulation) — driving a Karplus-Strong string engine with jawari nonlinear bridge, 8-voice sympathetic resonator bank, and chikari drone string. JHALA breakdown state machine for rapid arpeggio cascade climaxes.

---

## Signal flow

```text
CLOCK IN ──► [step counter]
RESET IN ──► [reset all sequencers to step 1]

PITCH brain (8 steps, own direction/length):
    step value → quantised to RAGA scale → ROOT offset → PITCH CV OUT + GATE OUT

RES brain (8 steps, own clock division, own length):
    step value → resonance amount → RES CV OUT

RIFF brain (8 steps, own length):
    step value → articulation type (Strike/Bend/Roll/Mute/Drone/Rest/Ornament/Return)
              → RIFF TRIG OUT

──► Karplus-Strong main string (DAMPING, BRIGHTNESS, MEEND glide)
        └─► Jawari nonlinear bridge (JAWARI, JAWARI EDGE, JAWARI CHAOS)
                └─► 8-voice sympathetic resonator bank (SYMP_DECAY, SYMP_SPREAD, SYMP_FEEDBACK)
                        └─► Chikari drone string (CHIKARI)

JHALA state machine: IDLE → BUILD → ACCEL → JHALA → LAND
BD_GATE_INPUT ──► triggers BUILD phase
LOCK_GATE_INPUT ──► freezes pitch sequencer (loop current step)

MAIN L / MAIN R ──► stereo output (KS + Jawari + Chikari)
DRONE OUT ──► chikari drone string mono
SYMP OUT ──► sympathetic resonator bank mono
PITCH CV OUT ──► current pitch (V/OCT)
GATE OUT ──► pitch step gate
RIFF TRIG OUT ──► riff articulation trigger
RES CV OUT ──► resonance step CV
```

---

## Controls table

### Pitch sequencer

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| PITCH STEP 1–8 | 0–7 | 0–1 | spread 0–1 | Scale degree position (quantised to RAGA) |
| PITCH LEN | 8 | 1–8 | 8 | Active step count |
| PITCH DIR | 9 | 0–1 | 0 | 0 = forward, 0.5 = pendulum, 1 = random |

### Resonance sequencer

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| RES STEP 1–8 | 10–17 | 0–1 | 0.5 | Resonance value per step — output as RES CV |
| RES LEN | 18 | 1–8 | 5 | Active step count |
| RES DIV | 19 | 1–4 | 2 | Clock division — RES steps at 1/DIV of pitch clock rate |

### Riff sequencer

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| RIFF STEP 1–8 | 20–27 | 0–7 | 0 | Articulation index: 0=Strike, 1=Bend, 2=Roll, 3=Mute, 4=Drone, 5=Rest, 6=Ornament, 7=Return |
| RIFF LEN | 28 | 1–8 | 8 | Active step count |

### Global

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| ROOT | 29 | −4 to +4 V/oct | 0 | Root pitch offset |
| RAGA | 30 | 0–5 | 1 | 0=Bilawal, 1=Yaman, 2=Bhairav, 3=Bhairavi, 4=Kafi, 5=Khamaj |
| PHRASE LEN | 31 | 1–32 | 8 | Phrase length (master cycle count) |

### Sound engine

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| DAMPING | 32 | 0–1 | 0.3 | KS string damping — 0 = bright/long, 1 = dark/short |
| BRIGHTNESS | 33 | 0–1 | 0.6 | KS pluck filter alpha — controls initial spectral content |
| MEEND | 34 | 0–1 | 0.2 | Pitch glide (meend/bend) speed between steps |

### Jawari bridge

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| JAWARI | 35 | 0–1 | 0.35 | Jawari buzz amount — nonlinear even-harmonic rattle |
| JAWARI EDGE | 36 | 0–1 | 0.50 | Jawari edge brightness — spectral edge of the buzz |
| JAWARI CHAOS | 37 | 0–1 | 0.10 | Jawari flutter/chaos — stochastic instability in the bridge |

### Sympathetic strings

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| SYMP DECAY | 38 | 0–1 | 0.70 | Sympathetic resonator decay time |
| SYMP SPREAD | 39 | 0–1 | 0.50 | Tuning spread of the 8 sympathetic resonators |
| SYMP FEEDBACK | 40 | 0–1 | 0.60 | Comb filter feedback in the resonator bank |

### JHALA breakdown engine

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| BD INT | 41 | 0–1 | 0 | Breakdown intensity (speed/density of JHALA arpeggio) |
| BD ACCEL | 42 | 0–1 | 0.5 | Breakdown acceleration rate (IDLE→BUILD→ACCEL ramp speed) |
| SA GRAVITY | 43 | 0–1 | 0.6 | Tonic (Sa) pull during LAND phase — how strongly it resolves |
| CHIKARI | 44 | 0–1 | 0.3 | Chikari drone string density (rhythmic accent frequency) |
| ORNAMENT | 45 | 0–1 | 0.3 | Ornament probability per step (gamak, meend, andolan) |
| BD LAND | 46 | 0–1 | 0.7 | Sam (downbeat) landing strength at end of JHALA |

---

## Raga scales

| RAGA index | Name | Scale degrees (semitones) |
| --- | --- | --- |
| 0 | Bilawal (major) | 0, 2, 4, 5, 7, 9, 11, 12 |
| 1 | Yaman (Lydian) | 0, 2, 4, 6, 7, 9, 11, 12 |
| 2 | Bhairav | 0, 1, 4, 5, 7, 8, 11, 12 |
| 3 | Bhairavi | 0, 1, 3, 5, 7, 8, 10, 12 |
| 4 | Kafi | 0, 2, 3, 5, 7, 9, 10, 12 |
| 5 | Khamaj | 0, 2, 4, 5, 7, 9, 10, 12 |

---

## JHALA breakdown state machine

```text
BD_GATE_INPUT (rising edge) ──► IDLE → BUILD
    BUILD: intensity accumulates over BD_ACCEL time
    → ACCEL: arpeggio rate increases
    → JHALA: rapid arpeggio cascade at full BD_INT speed
    → LAND: SA_GRAVITY pulls pitch to tonic, BD_LAND sets impact strength
    → IDLE: reset, cycle complete
```

BD_LIGHT LEDs: off = IDLE, slow flash = BUILD, fast flash = ACCEL/JHALA, solid = LAND.

---

## Articulation types (RIFF sequencer)

| Index | Name | Effect |
| --- | --- | --- |
| 0 | Strike | Standard pluck at full velocity |
| 1 | Bend | Pluck with meend pitch bend up |
| 2 | Roll | Rapid re-pluck (tremolo) |
| 3 | Mute | Pluck with fast DAMPING increase |
| 4 | Drone | Hold current pitch, suppress re-trigger |
| 5 | Rest | Silence (no pluck) |
| 6 | Ornament | Apply ornament per ORNAMENT density setting |
| 7 | Return | Return to previous pitch before bending back |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| V/OCT IN | Input | CV | External pitch override (bypasses pitch sequencer) |
| CLOCK IN | Input | Gate | Master clock — each rising edge advances all sequencers |
| RESET IN | Input | Gate | Resets all three sequencers to step 1 |
| BD GATE IN | Input | Gate | Triggers JHALA breakdown state machine |
| LOCK GATE IN | Input | Gate | High = pitch sequencer frozen on current step |
| ROOT CV IN | Input | CV | CV offset for ROOT parameter |
| JAWARI CV IN | Input | CV | CV for JAWARI buzz amount |
| MAIN L | Output | Audio | Stereo left (KS + Jawari + Chikari mixed) |
| MAIN R | Output | Audio | Stereo right |
| DRONE OUT | Output | Audio | Chikari drone string mono |
| SYMP OUT | Output | Audio | Sympathetic resonator bank mono |
| PITCH CV OUT | Output | CV | Current quantised pitch (V/OCT) |
| GATE OUT | Output | Gate | Pitch step gate (fires on each pluck) |
| RIFF TRIG OUT | Output | Gate | Riff articulation trigger |
| RES CV OUT | Output | CV | Resonance sequence CV (0–10 V) |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 29 | ROOT | CC 14 |
| 30 | RAGA | CC 15 (0–21 = Bilawal, 22–42 = Yaman, 43–63 = Bhairav, 64–84 = Bhairavi, 85–105 = Kafi, 106–127 = Khamaj) |
| 32 | DAMPING | CC 16 |
| 33 | BRIGHTNESS | CC 17 |
| 34 | MEEND | CC 18 |
| 35 | JAWARI | CC 19 |
| 41 | BD INT | CC 20 |
| 44 | CHIKARI | CC 21 |

---

## Recommended configurations

**Raga Exploration** — RAGA Yaman, ROOT 0, PITCH LEN 6, PITCH DIR forward. JAWARI 0.35, DAMPING 0.3, MEEND 0.3. SYMP DECAY 0.7, SYMP FEEDBACK 0.6. Clock at 80–100 BPM. Standard raga-style melodic development with sympathetic resonance.

**Minimalist Drone Melody** — PITCH LEN 3, all other steps rest (RIFF = 5). JAWARI 0.5, CHIKARI 0.4. One or two active pitch steps circling around the tonic. Chikari drone provides rhythmic pulse underneath. Very slow clock (40 BPM).

**Breakdown Performance** — Set up a 6-step PITCH sequence in Bhairav. MAP BD INT to CC 20. Trigger JHALA via BD GATE IN on a pad. At BD INT 0.8, SA GRAVITY 0.7, BD LAND 0.9 — a rapid arpeggio cascade lands hard on the tonic. Use as a climax gesture.

**Timbral Sequencing** — RES brain with RES DIV 4 (steps every 4 pitch clocks). Each RES step sends a different CV value to RES CV OUT. Patch RES CV OUT → JAWARI CV IN. The jawari buzz follows a separate slow sequence from the pitch — timbral counterpoint.

---

## Basic setup — sound in 60 seconds

1. Add SitarGrid to your patch.
2. Patch a clock source (120 BPM, 16th notes recommended) → CLOCK IN.
3. Patch MAIN L and MAIN R → your audio interface.
4. Set RAGA to Yaman (1), ROOT to 0, PITCH LEN to 8.
5. Set JAWARI to 0.35, DAMPING to 0.3, BRIGHTNESS to 0.6.
6. Send clock. You hear the 8-step raga-quantised sequence with Karplus-Strong plucks.
7. Toggle a few RIFF steps to Strike vs Rest to create rhythmic space.

---

## How-tos

### Three-brain polyrhythm

- Set PITCH LEN to 6, RES LEN to 5, RIFF LEN to 7.
- RES DIV to 3 (resonance changes every 3 pitch clocks).
- These three independent lengths create a polyrhythmic cycle that takes 6 × 5 × 7 = 210 steps to fully repeat.
- Patch RES CV OUT → JAWARI CV IN for timbral animation tied to the RES sequence.

### JHALA in performance

- Route BD GATE IN to a manual gate button or Pulse step 1.
- Set BD INT to 0.6, BD ACCEL to 0.5, SA GRAVITY 0.7, BD LAND 0.8.
- Trigger BD GATE IN at the peak of a performance section.
- The sequencer cascades through rapid arpeggios and lands on the tonic.
- BD LAND 0.9 makes the tonic landing emphatic. SA GRAVITY 0.3 makes it tentative.

### LOCK GATE for tonic pedal

- Patch a manual gate → LOCK GATE IN.
- When held, the pitch sequencer freezes on the current step.
- Use during the JHALA LAND phase to sustain the tonic while sympathetics ring.
- Release to resume the sequence.

### Sympathetics as spatial layer

- Patch SYMP OUT → a separate mixer channel or reverb input.
- Set SYMP SPREAD 0.7, SYMP DECAY 0.8, SYMP FEEDBACK 0.55.
- SYMP OUT carries only the 8 sympathetic comb filters — no direct KS signal.
- Route to a wet-only reverb send. The sympathetics create spatial resonance separate from the dry string attack.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| GRID (work-clock) | The three sequencer brains as rigid rhythmic grids — each running its own length and division |
| PRESSURE (rent-pressure) | JAWARI + BD INT building — harmonic pressure accumulated through the raga structure |
| COLLECTIVE (collective-refusal) | 8 sympathetic resonators + chikari drone — collective resonant response to the main string |
| VOICE (static-witness) | PITCH LEN 1 + RIFF = Drone — single sustained pitch, the instrument bearing witness |
| CARE (mutual-aid) | MEEND high + SA GRAVITY high — the melody tends toward the tonic, seeks resolution |
| MANAGED COLLAPSE | JHALA breakdown — the cascade and landing as structured collapse/recovery |

---

## Known pairings

| Module | Role |
| --- | --- |
| Drift | SMOOTH → JAWARI CV or ROOT CV for slow timbral and root drift |
| WallConductor | MAIN L/R into section input — SitarGrid as one section of a larger wall |
| DroneClone | Run in parallel; SitarGrid DRONE OUT → DroneClone RTN for resonance injection |
| CollapseSat | MAIN L/R → CollapseSat for saturation shaping of the KS output |
| FeedbackGovernor | SYMP OUT → SEND for a governed sympathetic feedback tail |
| HarmonicPressure | PITCH CV OUT → HarmonicPressure V/OCT IN — raga pitch drives harmonic series |
