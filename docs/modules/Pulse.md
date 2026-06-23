# Pulse — 12HP

16-step no-wave step percussion sequencer. 4×4 toggle grid, white noise synthesis with HIT level, DECAY time, METAL filter, and CRACK transient burst. TRG clock in, audio out. Inspired by primitive drum machine aesthetics and no-wave percussive attack.

---

## Signal flow

```text
TRG IN ──► [rising edge → step counter advances]
                │
                ▼ active step?
           [noise burst synthesis]
                ├─ HIT:   peak amplitude (0–1)
                ├─ DECAY: exponential envelope 8–500 ms
                ├─ METAL: 1-pole LP 360→80 Hz (0=open, 1=dark)
                └─ CRACK: 4 ms sharp transient on top of noise
                │
                ▼
           OUT ──► audio

V/OCT IN ──────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| STEP 1–16 | 0–15 | Toggle | off | 4×4 grid. Active steps fire on clock |
| HIT | 16 | 0–1 | 0.75 | Peak amplitude of triggered burst |
| DECAY | 17 | 0–1 | 0.30 | 0 = 8 ms (tight click), 1 = 500 ms (long thud) |
| METAL | 18 | 0–1 | 0.20 | LP filter on noise: 0 = open (hi-hat), 1 = dark (kick/thud) |
| CRACK | 19 | 0–1 | 0 | 4 ms sharp transient burst on top of noise — attack click |
| HIT ATTEN | 20 | −1 to +1 | 0 | Attenuverter for HIT CV |
| DECAY ATTEN | 21 | −1 to +1 | 0 | Attenuverter for DECAY CV |
| METAL ATTEN | 22 | −1 to +1 | 0 | Attenuverter for METAL CV |
| CRACK ATTEN | 23 | −1 to +1 | 0 | Attenuverter for CRACK CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| TRG IN | Input | Gate / clock | Rising edge advances to next step and fires if active |
| HIT CV | Input | CV | CV for HIT (scaled by HIT ATTEN) |
| DECAY CV | Input | CV | CV for DECAY |
| METAL CV | Input | CV | CV for METAL |
| CRACK CV | Input | CV | CV for CRACK |
| V/OCT IN | Input | CV | Pass-through |
| OUT | Output | Audio | Percussive noise audio |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 16 | HIT | CC 14 |
| 17 | DECAY | CC 15 |
| 18 | METAL | CC 16 |
| 19 | CRACK | CC 17 |

Individual step toggles (indices 0–15) can be mapped if your MIDI controller has 16 pads. Map step 1 → CC 36, step 2 → CC 37, etc. (General MIDI drum pad convention).

---

## Recommended configurations

**Hi-Hat** — METAL 0, DECAY 0.08, CRACK 0.7, HIT 0.8. Steps 1, 3, 5, 7, 9, 11, 13, 15 active (every other step). Clock at 16th notes. Open noise with sharp attack click.

**Floor Thud** — METAL 0.8, DECAY 0.55, CRACK 0.15, HIT 1.0. Steps 1 and 9 active. Clock at quarter notes. Low, slow decay thud to anchor the drone.

**Noise Bed** — METAL 0.4, DECAY 1.0, CRACK 0, HIT 0.4. All 16 steps active. Very slow clock (0.3 Hz). Produces a near-continuous noise texture with very slow amplitude envelope cycling.

**Stochastic Percussion** — METAL 0.3, DECAY 0.2, CRACK 0.5, HIT 0.7. 6–8 steps active in irregular pattern. Clock from Drift GATE output at RATE 0.5–2 Hz. Unpredictable, sparse percussion events.

---

## Basic setup — sound in 60 seconds

1. Add Pulse to your patch.
2. Add a clock source (e.g. VCV Clock at 120 BPM). Patch clock output → Pulse TRG IN.
3. Patch Pulse OUT → your audio output or Choke CH input.
4. Toggle steps 1, 5, 9, 13 on (quarter-note pattern).
5. Set METAL to 0.5, DECAY to 0.3, CRACK to 0.3, HIT to 0.8.
6. You have a basic percussive pulse. Adjust METAL lower for hi-hat, higher for kick.
7. Toggle additional steps to build the pattern.

---

## How-tos

### Dual Pulse layering

- Add two Pulse instances. Clock both from the same source.
- Pulse A: METAL 0, DECAY 0.08, CRACK 0.8 — hi-hat texture.
- Pulse B: METAL 0.85, DECAY 0.6, CRACK 0.1 — floor thud.
- Route to separate Choke channels (CH3 and CH4) for panning.
- Offset the grid patterns so they interlock rather than overlap.

### Stochastic velocity via Drift

- Patch Drift STEP OUT → Pulse HIT CV. Set HIT ATTEN to +0.3.
- Each step fires at a different amplitude because Drift STEP is unslewed and random.
- Set Drift RATE to match your clock speed for one new value per step.
- Use Drift GATE → Pulse TRG IN instead of a regular clock for fully probabilistic timing.

### Noise texture as drone bed

- Set all 16 steps active. Clock at 0.5 Hz (one step every 2 seconds).
- METAL 0.3, DECAY 1.0, HIT 0.5, CRACK 0.
- The overlapping decay envelopes create a continuous noise texture that slowly evolves.
- Patch OUT → CollapseSat for periodic harmonic injection into the noise.

### Rhythmic COLLAPSE trigger

- Patch Pulse GATE OUT (or OUT > 0 V threshold via comparator) → WallConductor COLLAPSE IN.
- Programme steps 1 and 9 active. On every COLLAPSE trigger the wall briefly drops then recovers.
- WallConductor RECOVERY at 0.4 gives a 1 s rise — percussive breath on downbeats.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| GRID (work-clock) | The 4×4 step grid is the literal work-clock — rigid periodic structure |
| DAMAGE (managed-collapse) | CRACK pushed high — the attack click is a repeated structural strike |
| PRESSURE (rent-pressure) | Fast clock + all steps active — relentless percussive accumulation |
| FAILURE (managed-collapse) | Very slow clock (0.05 Hz), all steps on — tempo collapses toward stasis |
| COLLECTIVE (collective-refusal) | Pulse GATE driving multiple module events simultaneously — collective trigger |

---

## Known pairings

| Module | Role |
| --- | --- |
| Drift | GATE → TRG IN for stochastic clock; STEP → HIT CV for random velocity |
| Choke | Pulse OUT as a channel source; GATE → MUTE CVs for rhythmic gating |
| WallConductor | GATE → COLLAPSE IN for beat-synced wall drops |
| CollapseSat | SC IN from Pulse OUT for sidechain drive pumping |
| HarmonicPressure | Pulse GATE can trigger external envelope to modulate PITCH |
