# Choke — 14HP

4-channel mixer built as a performance instrument. Fixed auto-pan spread, per-channel GAIN and TONE, MUTE buttons, MAIN master with soft saturation. Stereo L/R output. The immediate mixing surface for the Amplified Futures signal chain.

---

## Signal flow

```text
CH1 IN ──► GAIN (0–1.5×) ──► TONE (LP blend) ──► [MUTE] ──► pan L       ──►┐
CH2 IN ──► GAIN           ──► TONE             ──► [MUTE] ──► pan L-C     ──►┤
CH3 IN ──► GAIN           ──► TONE             ──► [MUTE] ──► pan R-C     ──►┤ MAIN ──► tanh ──► L OUT
CH4 IN ──► GAIN           ──► TONE             ──► [MUTE] ──► pan R       ──►┘                   R OUT

Fixed pan positions: CH1 = 0.0 (full L), CH2 = 0.33, CH3 = 0.67, CH4 = 1.0 (full R)
V/OCT IN ──────────────────────────────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| GAIN 1 | 0 | 0–1.5× | 0.75 | Channel 1 level. Unity at 0.75 (noon) |
| GAIN 2 | 1 | 0–1.5× | 0.75 | Channel 2 level |
| GAIN 3 | 2 | 0–1.5× | 0.75 | Channel 3 level |
| GAIN 4 | 3 | 0–1.5× | 0.75 | Channel 4 level |
| TONE 1 | 4 | 0–1 | 0.7 | LP blend: 0 = dark (≈400 Hz), 1 = full open |
| TONE 2 | 5 | 0–1 | 0.7 | Channel 2 tone |
| TONE 3 | 6 | 0–1 | 0.7 | Channel 3 tone |
| TONE 4 | 7 | 0–1 | 0.7 | Channel 4 tone |
| MUTE 1 | 8 | Toggle | off | Silences CH1, preserves pan position |
| MUTE 2 | 9 | Toggle | off | Silences CH2 |
| MUTE 3 | 10 | Toggle | off | Silences CH3 |
| MUTE 4 | 11 | Toggle | off | Silences CH4 |
| MAIN | 12 | 0–1.5 | 0.8 | Master output level, followed by soft tanh saturation |
| GAIN 1 ATTEN | 13 | −1 to +1 | 0 | Attenuverter for GAIN 1 CV |
| GAIN 2 ATTEN | 14 | −1 to +1 | 0 | Attenuverter for GAIN 2 CV |
| GAIN 3 ATTEN | 15 | −1 to +1 | 0 | Attenuverter for GAIN 3 CV |
| GAIN 4 ATTEN | 16 | −1 to +1 | 0 | Attenuverter for GAIN 4 CV |
| TONE 1 ATTEN | 17 | −1 to +1 | 0 | Attenuverter for TONE 1 CV |
| TONE 2 ATTEN | 18 | −1 to +1 | 0 | Attenuverter for TONE 2 CV |
| TONE 3 ATTEN | 19 | −1 to +1 | 0 | Attenuverter for TONE 3 CV |
| TONE 4 ATTEN | 20 | −1 to +1 | 0 | Attenuverter for TONE 4 CV |
| MAIN ATTEN | 21 | −1 to +1 | 0 | Attenuverter for MAIN CV |

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| CH1–4 IN | Input | Audio / poly | Per-channel audio. Polyphonic signals are summed to mono |
| GAIN 1–4 CV | Input | CV | CV for each channel GAIN (scaled by GAIN ATTEN) |
| TONE 1–4 CV | Input | CV | CV for each channel TONE |
| MAIN CV | Input | CV | CV for MAIN master level |
| V/OCT IN | Input | CV | Pass-through |
| L OUT | Output | Audio | Stereo left master |
| R OUT | Output | Audio | Stereo right master |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | GAIN 1 | CC 14 |
| 1 | GAIN 2 | CC 15 |
| 2 | GAIN 3 | CC 16 |
| 3 | GAIN 4 | CC 17 |
| 4 | TONE 1 | CC 18 |
| 5 | TONE 2 | CC 19 |
| 6 | TONE 3 | CC 20 |
| 7 | TONE 4 | CC 21 |
| 12 | MAIN | CC 7 (standard volume) |

MUTE buttons (indices 8–11) can be toggled via CC: CC value ≥ 64 = toggle on rising edge. Map each to a pad or button on a MIDI controller.

---

## Recommended configurations

**Quad Wall Mix** — Four DroneClone instances or StringMassCore voices into CH1–4. GAIN all at 0.75, TONE all at 0.7, MAIN 0.8. The fixed pan spread gives immediate stereo width without any external panning.

**Frequency Stack** — CH1 = sub layer (TONE 0.3, GAIN 0.9), CH2 = mid wall (TONE 0.7, GAIN 0.75), CH3 = high shimmer (TONE 1.0, GAIN 0.5), CH4 = percussion (TONE 0.8, GAIN 1.0). Use TONE as a per-channel filter rather than a global EQ.

**Saturation Pump** — GAIN on one or two channels at 1.3–1.5×, MAIN at 0.6. The per-channel gain clips into the MAIN tanh, giving saturation character on those channels while keeping overall level controlled.

**Rhythmic Mute Grid** — All channels at moderate gain and tone. Patch Pulse GATE outputs → CH MUTE CV inputs to create rhythmic on/off patterns against the drone layers.

---

## Basic setup — sound in 60 seconds

1. Add Choke to your patch.
2. Patch DroneClone OUT → Choke CH1 IN.
3. Patch Choke L OUT and R OUT → your audio interface.
4. All GAIN knobs at noon (0.75), TONE at 0.7, MAIN at 0.8.
5. You have audio. CH1 is panned hard left — patch a second source to CH4 for immediate stereo width.
6. Raise GAIN on CH1 past 1.0 to push the MAIN saturation. Listen to the character change.
7. Toggle MUTE 1 to hear the channel silence while pan position is preserved.

---

## How-tos

### TONE as per-channel filter

- Set CH1 TONE to 0.2 (dark, telephone texture) while CH2–4 stay at 0.7.
- This creates a mid-cut effect on one channel without any external EQ module.
- Automate CH1 TONE via Drift SMOOTH → TONE 1 CV for slow filter sweeps.
- TONE ATTEN at +0.4 gives a ±2-octave sweep range from a ±5 V Drift output.

### Mute as rhythmic gate

- Patch Pulse OUT → a comparator or Schmitt trigger → MUTE 1 CV.
- Alternatively use Pulse GATE directly if it is 10 V gate compatible.
- Programme Pulse with a 4-step pattern: steps 1 and 3 on. Channel 1 mutes on every off-beat.
- Use different Pulse patterns on MUTE 1 and MUTE 2 for polyrhythmic gating.

### Gain-into-saturation technique

- Push CH2 GAIN to 1.4. Set MAIN to 0.5.
- CH2 signal clips hard at the MAIN tanh but overall output is controlled.
- Add a second source at CH4 GAIN 0.5 — the clean signal contrasts with the saturated CH2.
- This replicates the effect of a driven channel on a cheap no-input mixer.

### Fixed pan as composition tool

- Route the brightest / most active signal to CH2 or CH3 (centre-ish positions).
- Route sub and noise layers to CH1 and CH4 (extremes).
- The fixed positions force deliberate signal placement — use this as a constraint rather than fighting it.
- For dynamic stereo: patch WIDTH CV on WallConductor downstream rather than moving Choke pans.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| GRID (work-clock) | Fixed pan positions as rigid spatial grid — no deviation from assigned positions |
| PRESSURE (rent-pressure) | GAIN pushed above 1.0 on multiple channels — compressive accumulation into MAIN tanh |
| DAMAGE (managed-collapse) | Rhythmic MUTE CV from Pulse — structural interruption of the mix |
| VOICE (static-witness) | One channel isolated (others muted) — singular voice audible in the field |
| CARE (mutual-aid) | Low GAIN across all channels, MAIN 0.5, TONE 0.5 — controlled, non-dominant mix presence |

---

## Known pairings

| Module | Role |
| --- | --- |
| DroneCore / DroneClone | Primary channel sources |
| StringMassCore | Polyphonic mass voices into one or two channels |
| Pulse | GATE outputs → MUTE CVs for rhythmic gating |
| WallConductor | Downstream: receives Choke L/R for DENSITY and COLLAPSE |
| CollapseSat | After Choke for additional saturation shaping |
| Drift | SMOOTH → TONE or GAIN CV for slow per-channel modulation |
