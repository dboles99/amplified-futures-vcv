# DroneCore — 8HP

Two-voice detuned oscillator. The foundational pitch source of the Amplified Futures stack — a pair of sine-to-harmonic oscillators running symmetrically detuned around a shared pitch centre, designed to pair with DroneClone or run in polyphonic stacks.

---

## Signal flow

```text
V/OCT IN (poly) ──► [pitch offset by PITCH knob]
                     ├─ Voice A: freq × 2^(+DETUNE/2 / 1200)
                     └─ Voice B: freq × 2^(−DETUNE/2 / 1200)
                           │
                    [TIMBRE blend: sine + harmonics 2–4]
                           │
                    mix A+B ──► OUT (poly)

V/OCT IN ──────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| PITCH | 0 | −1.667 to +2.333 Oct | 0 V (C4) | Base frequency offset over V/OCT input |
| PITCH ATTEN | 1 | −1 to +1 | 0 | Attenuverter for PITCH MOD CV |
| DETUNE | 2 | 0–100¢ | 0¢ | Symmetric split: Voice A = +DETUNE/2, Voice B = −DETUNE/2 |
| DETUNE ATTEN | 3 | −1 to +1 | 0 | Attenuverter for DETUNE CV |
| TIMBRE | 4 | 0–1 | 0 | 0 = pure sine; 1 = 2nd+3rd+4th harmonics (third-bridge stack) |
| TIMBRE ATTEN | 5 | −1 to +1 | 0 | Attenuverter for TIMBRE CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| V/OCT IN | Input | CV / poly | Polyphonic pitch input — sets channel count |
| PITCH MOD | Input | CV | CV for PITCH (scaled by PITCH ATTEN) |
| DETUNE CV | Input | CV | CV for DETUNE (scaled by DETUNE ATTEN) |
| TIMBRE CV | Input | CV | CV for TIMBRE (scaled by TIMBRE ATTEN) |
| OUT | Output | Audio / poly | Mixed dual-voice audio, polyphonic |
| V/OCT THRU | Output | CV / poly | Pass-through, same channel count as V/OCT IN |

---

## MIDI CC automation

VCV Rack's MIDI-Map module maps CCs to parameter indices. Right-click any knob and choose "Map" to assign directly.

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | PITCH | CC 14 |
| 1 | PITCH ATTEN | — (set manually) |
| 2 | DETUNE | CC 15 |
| 3 | DETUNE ATTEN | — (set manually) |
| 4 | TIMBRE | CC 16 |
| 5 | TIMBRE ATTEN | — (set manually) |

---

## Recommended configurations

**Still Air** — Pure sine drone. PITCH 0 V, DETUNE 0¢, TIMBRE 0. Run 4 channels from HarmonicPressure. No movement. A still, humming fundamental field.

**Interference Field** — DETUNE 18¢, TIMBRE 0. Two voices beating slowly. Feed DETUNE CV from Drift SMOOTH at 0.1 Hz / WANDER 0.4 for drifting beat frequencies. Best with 2–4 polyphonic channels.

**String Wall Sub** — One DroneCore tuned an octave below a DroneClone stack. PITCH −1 Oct, DETUNE 6¢, TIMBRE 0.25. Adds body without cluttering the string zone.

**Third Bridge Shimmer** — TIMBRE 0.7–1.0, DETUNE 40¢. The harmonic content produces a buzzy, third-bridge guitar texture. Combine with CollapseSat in EVEN mode for tape-like saturation character.

---

## Basic setup — sound in 60 seconds

1. Add DroneCore to your patch.
2. Add a MIDI-CV module or any V/OCT source. Patch V/OCT → DroneCore V/OCT IN.
3. Patch DroneCore OUT → your audio interface or mixer input.
4. Set PITCH knob to 0 (12 o'clock). Set DETUNE to 12¢. Leave TIMBRE at 0.
5. Send a held note. You will hear two sine voices beating at approximately 7 Hz.
6. Slowly raise TIMBRE to add harmonic content.
7. Connect Drift SMOOTH → DETUNE CV (DETUNE ATTEN at +0.3) for slow drift.

---

## How-tos

### Poly harmonic stack

- Connect HarmonicPressure VOCT OUT (8 channels) → DroneCore V/OCT IN.
- Each of the 8 polyphonic channels becomes a separate dual-voice pair.
- DETUNE 8–16¢ keeps intonation dense but not chaotic.
- Feed into Choke CH inputs for per-layer gain and mute control.

### Slow harmonic breathing

- Patch Drift SMOOTH → TIMBRE CV. Set TIMBRE ATTEN to +0.4.
- Set Drift RATE to 0.08 Hz, WANDER 0.35, SLEW 0.15.
- TIMBRE cycles between pure sine and upper harmonics over a minute-long arc.
- Add a second Drift at 0.13 Hz with SMOOTH → DETUNE CV for compound drift.

### Feedback injection

- Patch DroneCore OUT → Send A IN. Patch Send A OUT → DroneClone RTN.
- This routes the DroneCore fundamental into the DroneClone feedback return.
- Keep Send A→C DEPTH below 0.3 initially to avoid runaway resonance.

### Stacked detuned unison

- Add 4 DroneCore instances. Set DETUNE to 6¢, 12¢, 18¢, 24¢ respectively.
- All share the same V/OCT source. Mix all four OUT signals into Choke CH1–4.
- Produces a warm orchestral spread without using DroneClone.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| PRESSURE (rent-pressure) | DETUNE pushed to 50–100¢ creates unresolvable dissonance — the tonal analogue of compressive accumulation |
| GRID (work-clock) | Sync PITCH CV to a clocked sequencer for rigid pitch-grid patterns beneath free-running DroneClone texture |
| DAMAGE (managed-collapse) | TIMBRE maxed + DETUNE 80¢ + COLLAPSE on downstream CollapseSat — the harmonic stack buckles |
| CARE (mutual-aid) | Low DETUNE (0–6¢), low TIMBRE, slow Drift on TIMBRE CV — stable, restorative fundamental presence |

Macro assignments: DETUNE → PRESSURE macro. TIMBRE → DAMAGE macro. PITCH offset → GRID macro.

---

## Known pairings

| Module | Role |
| --- | --- |
| DroneClone | DroneCore one octave below as sub-fundamental layer |
| HarmonicPressure | V/OCT source for per-partial dual-voice pairs |
| Choke | Per-channel gain and muting of stacked instances |
| Drift | SMOOTH → TIMBRE or DETUNE CV for slow modulation |
| Send | Cross-feedback with DroneClone via A→B path |
| CollapseSat | Downstream saturation of the dual-voice mix |
