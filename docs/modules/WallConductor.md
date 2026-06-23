# Wall Conductor — 22HP

Section-based performance mixer and conductor. The live performance control surface for the Amplified Futures stack — modelled on a conductor orchestrating massed signal sections. DENSITY sweeps channels in, PRESSURE saturates, WIDTH spreads, FEEDBACK loops, COLLAPSE drops everything.

---

## Signal flow

```text
CH1–4 IN ──► [DENSITY gate: gain = clamp(DENSITY×4 − i, 0, 1)]
                     │
              [constant-power pan spread, scaled by WIDTH]
                     │
feedbackL/R ──► [× FEEDBACK] ──────────────────────────────►──┐
                                                               │
              sumL/sumR ──► PRESSURE tanh ──► × collapseEnv ──┘──► OUT L / OUT R

COLLAPSE BTN / COLLAPSE IN ──► collapseEnv → 0 instantly
RECOVERY ──────────────────► collapseEnv rises over 50 ms–10 s

V/OCT IN ──────────────────────────────────────────────────► V/OCT THRU
```

DENSITY formula per channel i (0-indexed): `gain = clamp(DENSITY × 4 − i, 0, 1)`

At DENSITY 0.25: only CH1 fully open. At 0.5: CH1+2. At 0.75: CH1–3. At 1.0: all four.

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| DENSITY | 0 | 0–1 | 1.0 | Sweeps channels in sequentially from CH1 → CH4 |
| PRESSURE | 1 | 0–1 | 0.3 | Drive into tanh saturation: 0 = clean, 1 = heavy clip |
| WIDTH | 2 | 0–1 | 0.7 | Stereo spread: 0 = mono centre, 1 = full L/R |
| FEEDBACK | 3 | 0–1 | 0 | 1-sample output→input feedback. Keep below 0.5 in normal use |
| RECOVERY | 4 | 0–1 | 0.3 | COLLAPSE recovery time: 0 = 50 ms, 1 = 10 s |
| COLLAPSE | 5 | Button | — | Momentary: drops output to zero, rises on release per RECOVERY |
| DENSITY ATTEN | 6 | −1 to +1 | 0 | Attenuverter for DENSITY CV |
| PRESSURE ATTEN | 7 | −1 to +1 | 0 | Attenuverter for PRESSURE CV |
| WIDTH ATTEN | 8 | −1 to +1 | 0 | Attenuverter for WIDTH CV |
| FEEDBACK ATTEN | 9 | −1 to +1 | 0 | Attenuverter for FEEDBACK CV |
| RECOVERY ATTEN | 10 | −1 to +1 | 0 | Attenuverter for RECOVERY CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| CH1 IN | Input | Audio | Section input 1 (mono or poly summed) |
| CH2 IN | Input | Audio | Section input 2 |
| CH3 IN | Input | Audio | Section input 3 |
| CH4 IN | Input | Audio | Section input 4 |
| DENSITY CV | Input | CV | CV for DENSITY (scaled by DENSITY ATTEN) |
| PRESSURE CV | Input | CV | CV for PRESSURE |
| WIDTH CV | Input | CV | CV for WIDTH |
| FEEDBACK CV | Input | CV | CV for FEEDBACK |
| RECOVERY CV | Input | CV | CV for RECOVERY |
| COLLAPSE IN | Input | Gate | High = collapsed, low = recovering |
| V/OCT IN | Input | CV | Pass-through |
| OUT L | Output | Audio | Stereo left master |
| OUT R | Output | Audio | Stereo right master |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | DENSITY | CC 1 (mod wheel — primary performance control) |
| 1 | PRESSURE | CC 11 (expression) |
| 2 | WIDTH | CC 10 (pan) |
| 3 | FEEDBACK | CC 14 |
| 4 | RECOVERY | CC 15 |

COLLAPSE button (index 5) can be mapped to a momentary pad: CC value 127 triggers, 0 releases. With RECOVERY set high, hold the pad to sustain the collapse.

---

## Recommended configurations

**Slow Build** — Start DENSITY at 0, PRESSURE 0.2, WIDTH 0.6, FEEDBACK 0. Automate DENSITY with mod wheel from 0 → 1 over 4 minutes. Each section enters one at a time. Then push PRESSURE to 0.6 for harmonic density.

**Feedback Wall** — DENSITY 1.0, FEEDBACK 0.3, PRESSURE 0.4, WIDTH 0.7, RECOVERY 0.5. The 1-sample feedback loop creates a subtle room-like buildup. Press COLLAPSE for dramatic drops with a 2 s recovery.

**Stereo Breathing** — Drift SMOOTH → WIDTH CV (WIDTH ATTEN +0.4). RATE 0.05 Hz, WANDER 0.3. The stereo field slowly expands and contracts over 20 s arcs. DENSITY fixed at 1.0.

**Orchestral Drop** — RECOVERY 0.85 (≈ 5 s rise). Hold COLLAPSE for 2 s, release. The wall rebuilds over 5 s from silence — a shaped re-entry after a performance event.

---

## Basic setup — sound in 60 seconds

1. Add WallConductor to your patch.
2. Patch DroneClone OUT → CH1 IN. Patch a second source → CH2 IN.
3. Patch WallConductor OUT L and OUT R → your audio interface.
4. Set DENSITY to 1.0, PRESSURE 0.3, WIDTH 0.7, FEEDBACK 0, RECOVERY 0.3.
5. You have a mixed stereo signal from both channels.
6. Map DENSITY to your mod wheel. Pull it to 0 — only CH1 remains.
7. Press COLLAPSE. Release. Adjust RECOVERY for the rebuild time.

---

## How-tos

### DENSITY as primary performance control

- Map DENSITY to MIDI CC 1 (mod wheel).
- At DENSITY 0: only CH1 is audible — your primary drone layer.
- At DENSITY 0.5: CH1 + CH2 enter — second section joins.
- At DENSITY 1.0: all four sections at full level.
- This single gesture controls the orchestral mass.

### Stacked conductors

- WallConductor A handles sections 1–4 (four DroneClone instances).
- WallConductor B handles sections 5–8 (four StringMassCore voices).
- Both COLLAPSE INs share one gate from Pulse step 1 — simultaneous drop.
- Independent DENSITY controls allow each layer stack to build separately.

### Feedback buildup and collapse

- Set FEEDBACK to 0.25, PRESSURE 0.4. Play for 1–2 minutes.
- The one-sample loop accumulates harmonic density gradually.
- When the texture is maximally dense, press COLLAPSE.
- Set RECOVERY 0.7 for a 3–4 s shaped re-entry.

### Autonomous density arc

- Drift SMOOTH OUT → DENSITY CV (DENSITY ATTEN +0.7).
- Drift RATE 0.03, WANDER 0.25, SLEW 0.03 (very smooth).
- The mix expands and contracts over 30–60 second arcs without intervention.
- Combine with Drift GATE → COLLAPSE IN for random collapse events.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| COLLECTIVE (collective-refusal) | DENSITY at 1.0 — all sections active, collective mass |
| PRESSURE (rent-pressure) | PRESSURE knob + FEEDBACK building — unrelenting accumulation |
| MANAGED COLLAPSE | COLLAPSE button — the defining collapse gesture |
| CARE (mutual-aid) | DENSITY 0.25 (one section only), low PRESSURE, long RECOVERY — restrained, recoverable presence |
| FAILURE (managed-collapse) | FEEDBACK pushed toward 0.6–0.7 — the loop approaches instability |

Macro assignments: DENSITY → COLLECTIVE macro. PRESSURE → PRESSURE macro. COLLAPSE → FAILURE macro trigger.

---

## Known pairings

| Module | Role |
| --- | --- |
| DroneClone ×2–4 | Section inputs — the canonical use |
| StringMassCore | Harmonic mass voices as additional sections |
| Pulse | GATE → COLLAPSE IN for beat-synced drops |
| CollapseSat | Post-mix: WallConductor OUT → CollapseSat IN |
| Drift | SMOOTH → DENSITY CV for autonomous section sweeps |
| HarmonicPressure | V/OCT → DENSITY CV via attenuverter for pitch-tracked density |
