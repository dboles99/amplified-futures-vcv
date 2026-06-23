# DroneClone — 22HP

8-voice amplified string wall. The centrepiece of the Amplified Futures signal chain — inspired by massed electric guitar density. Up to 16 polyphonic channels × 8 internal voices = 128 simultaneous oscillators. MASS/TENSION/SHIMMER/JAWARI/WEIGHT/DRIFT control the character of the wall. CHOKE and RTN handle performance collapse and feedback return.

---

## Signal flow

```text
V/OCT IN (poly) ──► per-channel × per-voice oscillator bank
                     ├─ MASS:    active voice count 1–8
                     ├─ TENSION: odd-harmonic saw content
                     ├─ SHIMMER: upper partials 4th–8th
                     ├─ JAWARI:  even-harmonic rattle/buzz
                     ├─ WEIGHT:  sub-octave body mix
                     └─ DRIFT:   per-voice slow phase wander

RTN IN ──────────────────────────────► mixed into oscillator input

CHOKE BTN / CHOKE IN (gate) ──► instant amplitude collapse

sum per channel ──► 1/√8 normalise ──► OUT (poly)
V/OCT IN ──────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| FUNDAMENTAL | 0 | −2 to +2 Oct | 0 | Pitch offset over V/OCT IN |
| SPREAD | 1 | 0–1 | 0 | Per-voice detune spread around fundamental |
| MASS | 2 | 1–8 | 4 | Active voices per channel — higher = denser |
| TENSION | 3 | 0–1 | 0 | Odd-harmonic saw content (edge) |
| WEIGHT | 4 | 0–1 | 0 | Sub-octave body mix |
| SHIMMER | 5 | 0–1 | 0 | Upper partial brightness (4th–8th harmonics) |
| JAWARI | 6 | 0–1 | 0 | Even-harmonic rattle — asymmetric buzz like sitar jawari |
| DRIFT | 7 | 0–1 | 0 | Per-voice slow phase wander rate |
| DECAY | 8 | 0–1 | 0.3 | Voice envelope decay — shapes wall density |
| CHOKE AMT | 9 | 0–1 | 0.5 | Choke collapse depth |
| CHOKE BTN | 10 | — | — | Momentary button — triggers wall collapse |
| FUNDAMENTAL ATTEN | 11 | −1 to +1 | 0 | Attenuverter for FUNDAMENTAL CV |
| SPREAD ATTEN | 12 | −1 to +1 | 0 | Attenuverter for SPREAD CV |
| MASS ATTEN | 13 | −1 to +1 | 0 | Attenuverter for MASS CV |
| TENSION ATTEN | 14 | −1 to +1 | 0 | Attenuverter for TENSION CV |
| WEIGHT ATTEN | 15 | −1 to +1 | 0 | Attenuverter for WEIGHT CV |
| SHIMMER ATTEN | 16 | −1 to +1 | 0 | Attenuverter for SHIMMER CV |
| JAWARI ATTEN | 17 | −1 to +1 | 0 | Attenuverter for JAWARI CV |
| DRIFT ATTEN | 18 | −1 to +1 | 0 | Attenuverter for DRIFT CV |
| DECAY ATTEN | 19 | −1 to +1 | 0 | Attenuverter for DECAY CV |
| CHOKE AMT ATTEN | 20 | −1 to +1 | 0 | Attenuverter for CHOKE AMT CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| V/OCT IN | Input | CV / poly | Polyphonic pitch input — sets channel count |
| MASS CV | Input | CV | CV for MASS (InputId index 2, scaled by MASS ATTEN) |
| TENSION CV | Input | CV | CV for TENSION |
| CHOKE IN | Input | Gate | 10 V high collapses all voices immediately |
| RTN IN | Input | Audio | Feedback return — patch from downstream output |
| FUNDAMENTAL CV | Input | CV | CV for FUNDAMENTAL offset |
| SPREAD CV | Input | CV | CV for SPREAD |
| WEIGHT CV | Input | CV | CV for WEIGHT |
| SHIMMER CV | Input | CV | CV for SHIMMER |
| JAWARI CV | Input | CV | CV for JAWARI |
| DRIFT CV | Input | CV | CV for DRIFT |
| DECAY CV | Input | CV | CV for DECAY |
| CHOKE AMT CV | Input | CV | CV for CHOKE AMT |
| OUT | Output | Audio / poly | 8-voice mixed audio, polyphonic |
| V/OCT THRU | Output | CV / poly | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | FUNDAMENTAL | CC 14 |
| 2 | MASS | CC 17 |
| 3 | TENSION | CC 18 |
| 4 | WEIGHT | CC 19 |
| 5 | SHIMMER | CC 20 |
| 6 | JAWARI | CC 21 |
| 7 | DRIFT | CC 22 |
| 8 | DECAY | CC 23 |

Right-click any knob → "Map" in VCV Rack to assign directly. CHOKE BTN (index 10) can be mapped to a momentary CC (CC value 127 = press).

---

## Recommended configurations

**No-Wave Guitar Wall** — TENSION 0.4, JAWARI 0.2, MASS 4, SHIMMER 0, WEIGHT 0.1, DRIFT 0.15. Feed into WallConductor. The canonical Amplified Futures guitar wall.

**Sitar Swarm** — JAWARI 0.6, SHIMMER 0.3, TENSION 0.1, MASS 6, DRIFT 0.3. Feed from HarmonicPressure (8 partials). Add Drift SMOOTH → JAWARI CV for breathing buzz character.

**Sub Wall** — WEIGHT 0.5, TENSION 0.1, MASS 3, JAWARI 0, SHIMMER 0. FUNDAMENTAL −1 Oct. Use as a sub-bass layer beneath a second DroneClone running the mids.

**Maximum Density** — MASS 8, TENSION 0.6, SHIMMER 0.4, JAWARI 0.3, DRIFT 0.4. For full-spectrum noise-rock maximalism. Requires FeedbackGovernor in the RTN path to prevent runaway.

---

## Basic setup — sound in 60 seconds

1. Add DroneClone to your patch.
2. Patch a V/OCT source → DroneClone V/OCT IN.
3. Patch DroneClone OUT → your audio output or WallConductor CH1.
4. Set MASS to 4 (noon). Set TENSION to 0.3. Leave all other knobs at 0.
5. Send a held note. You hear the 4-voice wall.
6. Raise JAWARI to 0.15 for subtle buzz character.
7. Press CHOKE button to experience the collapse. Adjust CHOKE AMT for depth.

---

## How-tos

### RTN feedback loop

- Patch DroneClone OUT → FeedbackGovernor SEND.
- Patch FeedbackGovernor RETURN → DroneClone RTN.
- FeedbackGovernor: AMOUNT 0.3, TONE 0.7, DECAY 0.2.
- The feedback gradually darkens — push TENSION up to increase resonance.
- Hit KILL on FeedbackGovernor for instant feedback silence on downbeats.

### CHOKE as performance event

- Patch WallConductor COLLAPSE gate out → DroneClone CHOKE IN.
- Or patch Pulse GATE → CHOKE IN for rhythmic wall stutters.
- CHOKE AMT at 0.8 gives nearly complete collapse. At 0.3, a subtle dip.

### JAWARI breathing

- Patch Drift SMOOTH → JAWARI CV. Set JAWARI ATTEN to +0.25.
- Set Drift RATE 0.07 Hz, WANDER 0.5, SLEW 0.2.
- The buzz character breathes slowly. Keep base JAWARI at 0.3 so the CV pulls it above and below.

### Two DroneClone cross-feedback

- DroneClone A OUT → Send A IN. DroneClone B OUT → Send B IN.
- Send A OUT → DroneClone A RTN. Send B OUT → DroneClone B RTN.
- Set Send A→B to 0.3, B→A to 0.3. Each clone's output bleeds into the other's return.
- Different TENSION and JAWARI settings on each create timbral interference.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| PRESSURE (rent-pressure) | TENSION + JAWARI pushed to maximum — unrelenting harmonic accumulation |
| COLLECTIVE (collective-refusal) | MASS 8, all voices active, DRIFT high — massed collective motion without individual distinction |
| DAMAGE (managed-collapse) | CHOKE IN triggered by Pulse — rhythmic structural damage to the wall |
| CARE (mutual-aid) | MASS 2–3, DRIFT 0.1, TENSION low — minimal voiced presence, room to breathe |
| FAILURE (managed-collapse) | RTN feedback pushed past AMOUNT 0.6 — loop destabilises toward self-oscillation |

Macro assignments: TENSION → PRESSURE macro. MASS → COLLECTIVE macro. JAWARI → DAMAGE macro.

---

## Known pairings

| Module | Role |
| --- | --- |
| WallConductor | Section-based mixing of multiple DroneClone instances |
| FeedbackGovernor | Governed feedback in the RTN path |
| HarmonicPressure | V/OCT source for harmonic series chord stacks |
| CollapseSat | Post-mix saturation and collapse events |
| Drift | SMOOTH → DRIFT or JAWARI CV for organic wander |
| Send | Cross-feedback routing between two DroneClone instances |
