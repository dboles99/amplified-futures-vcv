# Playbooks

Named patch configurations for live performance. Each playbook describes a signal routing, starting parameter values, and a performance arc — from open to close.

---

## Performance principles

- **Start sparse.** One source, one processor. Layer in.
- **Collapse is a gesture, not a failure.** Use COLLAPSE events intentionally — they are the climax, not the reset.
- **Feedback has a safe limit.** FeedbackGovernor AMOUNT above 0.5 with DECAY below 0.3 = runaway. KILL button is your abort.
- **Drift rates in prime-number ratios** (0.07, 0.11, 0.17 Hz) prevent synchronisation and give each modulation source independent character.
- **WallConductor DENSITY** is the main performance fader — more expressive than individual volume control.
- **Street Grid Clock** gives the patch its own time. Use it when you want the system to clock itself instead of borrowing tempo from elsewhere.

---

## Available playbooks

| Playbook | Modules | Character |
|---|---|---|
| [The Wall](#the-wall) | StreetGridClock → Pulse → HarmonicPressure → StringMassCore × 2 → WallConductor → CollapseSat + FeedbackGovernor | Full massed-voice orchestra. Density sweeps and self-clocked collapse events. |
| [Drone Bed](#drone-bed) | DroneCore × 4 → Choke, Drift × 2 | Pure sustain. 4 detuned layers with Drift-animated timbre. No clock. |
| [Feedback Republic](#feedback-republic) | DroneClone × 2 ↔ Send ↔ FeedbackGovernor, Drift, Pulse | Cross-feedback system. C-bus governed loop. Kill events as rhythm. |
| [Harmonic Pressure Session](#harmonic-pressure-session) | HarmonicPressure → StringMassCore → WallConductor → CollapseSat | PARTIAL as primary performance control. Ascend the harmonic series live. |
| [Percussion Slab](#percussion-slab) | StreetGridClock → Pulse → Choke, DroneCore × 2, StringMassCore, Drift | Rhythmic structure driving drone muting. Clocked slab with Drift for texture dissolution. |

---

## The Wall

**Concept:** Full massed-voice orchestra — string voices across harmonic sections, conducted live through density sweeps and collapse events.

### Module routing

```
[StreetGridClock] CLK ─────────────────────► [Pulse] TRG
[StreetGridClock] /8 ──────────────────────► [WallConductor] COLLAPSE IN
[StreetGridClock] RESET ────────────────────► [Pulse] RESET

[HarmonicPressure]
  VOCT OUT (8ch) ─────────────────────► [StringMassCore A] VOCT IN
               └───────────────────────► [StringMassCore B] VOCT IN

[StringMassCore A] OUT ──► [WallConductor] CH1 IN
[StringMassCore B] OUT ──► [WallConductor] CH2 IN

[WallConductor] OUT L/R ──► [CollapseSat] IN L/R
[CollapseSat] OUT L/R ───────────────────────────► [output]

[CollapseSat] OUT L ──► [FeedbackGovernor] SEND
[FeedbackGovernor] RETURN ──► [WallConductor] CH3 IN  (feedback as 3rd section)

[Drift] SMOOTH ──► [WallConductor] DENSITY CV
[Pulse] GATE   ──► [WallConductor] COLLAPSE IN
[Pulse] OUT    ──► [CollapseSat] SC IN
```

### Starting parameters

**HarmonicPressure:** PITCH 0V, PARTIAL 1, COUNT 8, SPREAD 0.2, TUNING JUST

**StringMassCore A** (outer wall): MASS 8, SPREAD 0.3, TIMBRE 0.5, MODE HARM

**StringMassCore B** (inner cluster): MASS 6, SPREAD 0.15, TIMBRE 0.3, MODE HARM

**WallConductor:** DENSITY 0.25, PRESSURE 0.25, WIDTH 0.85, FEEDBACK 0.15, RECOVERY 0.65

**StreetGridClock:** RATE 0.4, SWING 0, BROWNOUT 0, RUN on

**Drift:** RATE 0.12, WANDER 0.35, SLEW 0.4 — SMOOTH → WallConductor DENSITY CV (atten +0.3)

**CollapseSat:** DRIVE 0.25, BUZZ EVEN, RECOVERY 0.6

**FeedbackGovernor:** AMOUNT 0.3, TONE 0.45, DECAY 0.4

**Pulse:** Steps 0, 4, 8, 12 active. DECAY 0.4, METAL 0.6. GATE → WallConductor COLLAPSE

### Performance arc

| Phase | Action |
|---|---|
| Open | Drift slowly raises DENSITY. Wall builds from CH1 alone. |
| Build | As DENSITY crosses 0.5, CH2 enters. Slowly raise FeedbackGovernor AMOUNT to 0.4. |
| Peak | DENSITY → 1.0 (all sections). Push WallConductor PRESSURE to 0.5. CollapseSat DRIVE up. |
| Collapse event | Press WallConductor COLLAPSE — hold 2s, release. Wall rises back over 3s. |
| Second collapse | StreetGridClock /8 drives COLLAPSE gate. Rhythmic drop/rise for 16 bars. |
| Fade | Slowly pull DENSITY back to 0.25. Kill FeedbackGovernor (KILL button). |

> **Safety:** Keep FeedbackGovernor AMOUNT below 0.45 before pushing PRESSURE above 0.5. If feedback runaway starts, press KILL.

---

## Drone Bed

**Concept:** 4 DroneCore instances at different detune amounts routed through Choke. Drift modulates TIMBRE and DETUNE for slow harmonic breathing. No percussion. Pure sustain.

### Module routing

```
[Drift A] SMOOTH ──► [DroneCore 1] DETUNE CV (atten: +0.4)
               └──► [DroneCore 2] DETUNE CV (atten: +0.3)
[Drift A] STEP   ──► [DroneCore 3] DETUNE CV (atten: +0.25)

[Drift B] SMOOTH ──► [DroneCore 1] TIMBRE CV (atten: +0.5)
               └──► [DroneCore 2] TIMBRE CV (atten: −0.3)  ← inverted for contrast
[Drift B] STEP   ──► [DroneCore 4] TIMBRE CV (atten: +0.4)

[DroneCore 1] OUT ──► [Choke] CH1 IN
[DroneCore 2] OUT ──► [Choke] CH2 IN
[DroneCore 3] OUT ──► [Choke] CH3 IN
[DroneCore 4] OUT ──► [Choke] CH4 IN

[Choke] OUT L/R ──► [output]
```

All 4 DroneCores share same V/OCT source or run free at PITCH 0V.

### Starting parameters

**DroneCore 1** (foundation): PITCH 0V, DETUNE 8¢, TIMBRE 0.1

**DroneCore 2** (octave layer): PITCH +1V, DETUNE 14¢, TIMBRE 0.2

**DroneCore 3** (wide detune): PITCH 0V, DETUNE 22¢, TIMBRE 0.0

**DroneCore 4** (harmonic layer): PITCH 0V, DETUNE 35¢, TIMBRE 0.6

**Drift A** (DETUNE mod): RATE 0.08, WANDER 0.3, SLEW 0.35

**Drift B** (TIMBRE mod): RATE 0.13, WANDER 0.25, SLEW 0.5

**Choke:** All GAIN 0.65, all TONE 0.75, MAIN 0.7

### Performance arc

| Phase | Action |
|---|---|
| Open | CH1 only (mute CH2–4). Let Drift A establish detune drift. |
| Layer 1→2 | Unmute CH2. Watch beating patterns emerge. |
| Layer 1–3 | Unmute CH3. Three detune layers — complex beating. |
| Full stack | Unmute CH4. Dense, evolving wash. |
| Thin | Solo CH4 (mute 1–3) for harmonic-heavy single layer. |
| Rate push | Increase Drift A RATE to 0.4 — faster stochastic detune jumps. |
| Fade | Pull Choke MAIN slowly to 0. |

> **Note:** Different Drift rates (0.08 vs 0.13) prevent synchronisation. Inverting TIMBRE CV atten on DroneCore 2 means it goes darker as others go brighter — creates motion within static pitch. This patch works entirely without a clock source.

---

## Feedback Republic

**Concept:** Cross-feedback system between two oscillator sources via Send module, governed by FeedbackGovernor. The C-bus becomes a third voice. Drift animates tone and decay. Kill events as performance gestures.

### Module routing

```
[DroneClone A] OUT ──► [Send] IN A
[DroneClone B] OUT ──► [Send] IN B

[Send] OUT A ──► [DroneClone A] RTN IN   ← A gets B back + C-bus return
[Send] OUT B ──► [DroneClone B] RTN IN   ← B gets A forward

[Send] OUT C ──► [FeedbackGovernor] SEND
[FeedbackGovernor] RETURN ──► [Send] IN B  ← C-bus governed, returned into B path

[Drift] SMOOTH ──► [FeedbackGovernor] TONE CV (atten: +0.6)
[Drift] STEP   ──► [FeedbackGovernor] DECAY CV (atten: +0.3)
[Pulse] GATE   ──► [FeedbackGovernor] KILL GATE

[Send] OUT A + OUT B ──► [WallConductor] CH1 + CH2
[WallConductor] OUT L/R ──► [output]
```

### Starting parameters

**DroneClone A:** MASS 4, TENSION 0.35, JAWARI 0.2, WEIGHT 0.15, DRIFT 0.2

**DroneClone B:** MASS 4, TENSION 0.4, JAWARI 0.35, SHIMMER 0.25, DRIFT 0.25

**Send:** A→B 0.4, B→A 0.3, A→C 0.5, C→A 0.4

**FeedbackGovernor:** AMOUNT 0.35, TONE 0.5, DECAY 0.35

**Drift:** RATE 0.18, WANDER 0.4, SLEW 0.3

**Pulse:** Steps 0 and 8 active (half-bar). Clock ~40 BPM.

**WallConductor:** DENSITY 0.6, PRESSURE 0.2, WIDTH 0.7

### Performance arc

| Phase | Action |
|---|---|
| Establish | Both DroneClones running. Cross blend only, C-bus off. |
| Introduce C-bus | Raise A→C to 0.4, C→A to 0.3. Third character emerges. |
| Governor active | FeedbackGovernor AMOUNT up to 0.35. Tone drift begins. |
| Rhythmic kills | Start Pulse. Every half-bar the C-bus zeros. |
| Build tension | Increase A→C to 0.7. More C-bus. Push Governor AMOUNT to 0.5. |
| Collapse | Raise WallConductor PRESSURE to 0.6. Hit COLLAPSE. |
| Resolution | Kill Pulse. Slowly reduce A→C to 0. Feedback drains. |

---

## Harmonic Pressure Session

**Concept:** HarmonicPressure drives StringMassCore in full harmonic mode. PARTIAL and COUNT are the primary performance controls — sweeping up the harmonic series live.

### Module routing

```
[Drift] SMOOTH ──► [HarmonicPressure] PITCH CV (atten: +0.15)

[HarmonicPressure] VOCT OUT (poly) ──► [StringMassCore] VOCT IN

[StringMassCore] OUT ──► [WallConductor] CH1 IN

[WallConductor] OUT L/R ──► [CollapseSat] IN L/R
[CollapseSat] OUT L/R ──► [output]
```

### Starting parameters

**HarmonicPressure:** PITCH 0V, PARTIAL 1, COUNT 4, SPREAD 0.15, TUNING JUST

**StringMassCore:** MASS 8, SPREAD 0.25, TIMBRE 0.4, MODE HARM

**Drift:** RATE 0.07, WANDER 0.2, SLEW 0.5 — SMOOTH → HP PITCH CV (atten +0.15)

**WallConductor:** DENSITY 1.0, PRESSURE 0.2, WIDTH 0.8, FEEDBACK 0

**CollapseSat:** DRIVE 0.2, BUZZ EVEN, RECOVERY 0.5

### Live controls

**PARTIAL knob** is the primary performance gesture:

| PARTIAL | Tonal character |
|---|---|
| 1 | Fundamental gravity — stable |
| 3 | Skips octave/2nd — immediately tense |
| 7–9 | High harmonic tension, microtonal territory |
| 14–16 | Extreme upper series — almost noise |

**COUNT knob:** 2–3 = sparse and open; 8 = full wall; 16 = overwhelming at high PARTIAL.

### Performance arc

| Phase | Duration | Action |
|---|---|---|
| Ground | 2–3 min | PARTIAL 1, COUNT 4. Pure low harmonic presence. |
| Ascent | 3–5 min | Move PARTIAL up: 1 → 3 → 5 → 7. Each step raises tension. |
| Expansion | 2 min | At PARTIAL 7, expand COUNT to 8. |
| Mode shift | 1 min | Switch StringMassCore MODE: HARM → JUST. JI chord locks. |
| Push | 2 min | WallConductor PRESSURE to 0.5. CollapseSat DRIVE to 0.4. |
| Peak collapse | — | Hit CollapseSat COLLAPSE. Hold 3s. Long RECOVERY (0.7). |
| Return | 2 min | Pull PARTIAL back to 1 over 2 min. COUNT back to 4. |
| Fade | — | WallConductor DENSITY slow pull to 0. |

---

## Percussion Slab

**Concept:** Pulse drives rhythmic structure — triggers StringMassCore as textural percussion, with Choke controlling per-hit gain and Drift adding stochastic velocity. WallConductor COLLAPSE tied to a specific beat for periodic drops.

### Module routing

```
[StreetGridClock] CLK ──► [Pulse] TRG IN
[StreetGridClock] /8 ──► [WallConductor] COLLAPSE IN

[Pulse] GATE ──► [Choke] MUTE CV CH1  (mutes drone A on hit)
[Pulse] GATE ──► [Choke] MUTE CV CH2  (inverted atten −1 → opens CH2 on hit)
[Pulse] OUT  ──► [Choke] CH3 IN       (percussion hit as audio)

[Drift A] STEP ──► [Pulse] HIT CV (atten +0.4)  ← stochastic velocity
[Drift B] GATE ──► [Pulse] TRG IN               ← Drift as irregular clock

[HarmonicPressure] VOCT OUT ──► [StringMassCore] VOCT IN
[StringMassCore] OUT ──► [Choke] CH4 IN

[DroneCore A] OUT ──► [Choke] CH1 IN
[DroneCore B] OUT ──► [Choke] CH2 IN

[Choke] OUT L/R ──► [WallConductor] CH1 IN
[WallConductor] OUT L/R ──► [output]
```

### Starting parameters

**StreetGridClock:** RATE 0.42, SWING 0.15, BROWNOUT 0, RUN on

**Pulse:** Steps 0, 4, 8, 12. HIT 0.7, DECAY 0.45, METAL 0.65, CRACK 0.35

**Drift A** (velocity): RATE 0.55, WANDER 0.85, SLEW 0.0

**Drift B** (clock): RATE 0.4, WANDER 0.6, SLEW 0.0

**HarmonicPressure:** PARTIAL 3, COUNT 4, TUNING EQUAL, SPREAD 0.05

**StringMassCore:** MASS 4, SPREAD 0.1, TIMBRE 0.6, MODE HARM

**DroneCore A:** PITCH 0V, DETUNE 10¢, TIMBRE 0.2

**DroneCore B:** PITCH −1V (sub octave), DETUNE 6¢, TIMBRE 0.0

**Choke:** CH1 GAIN 0.5, CH2 GAIN 0.4, CH3 GAIN 0.8, CH4 GAIN 0.35, all TONE 0.7, MAIN 0.6

**WallConductor:** DENSITY 1.0, PRESSURE 0.3, WIDTH 0.7, RECOVERY 0.15

### Performance arc

| Phase | Action |
|---|---|
| Skeleton | Pulse only, no drone layers. Kick pattern, METAL-heavy. |
| Add sub | Unmute Choke CH2 (DroneCore B at −1V). Low body. |
| Add texture | Unmute Choke CH4 (StringMassCore). Harmonic mass on every beat. |
| Full mix | All channels. Drift velocity making hits irregular. |
| WallConductor drops | StreetGridClock /8 → WallConductor COLLAPSE. Bar 1 drops. |
| Sparse clock | Keep StreetGridClock running, but switch Pulse TRG to Drift B GATE. Rhythm dissolves into texture. |
| Noise flood | Push Pulse METAL to 0.0 and DECAY to 0.8 — continuous noise bed. |

---

## See also

[[Wall-Conductor]] · [[DroneClone]] · [[Harmonic-Pressure]] · [[Drift]] · [[Pulse]] · [[Feedback-Governor]]
