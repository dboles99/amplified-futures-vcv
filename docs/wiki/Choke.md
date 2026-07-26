# Choke — 14HP

![Choke panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Choke.png)

4-channel mixer built as an instrument. Fixed auto-pan spread, per-channel GAIN and TONE, MUTE buttons, MAIN master with soft saturation. Stereo L/R output. The performance mixing surface for smaller Amplified Futures signal chains.

---

## Signal flow

```
CH1 IN ──► GAIN ──► TONE (LP) ──► [MUTE] ──► pan L       ──►┐
CH2 IN ──► GAIN ──► TONE (LP) ──► [MUTE] ──► pan L–C     ──►┤
CH3 IN ──► GAIN ──► TONE (LP) ──► [MUTE] ──► pan R–C     ──►┤ MAIN ──► tanh ──► L OUT
CH4 IN ──► GAIN ──► TONE (LP) ──► [MUTE] ──► pan R       ──►┤                   R OUT
```

**Fixed pan positions:** CH1 = full L, CH2 = L–centre, CH3 = R–centre, CH4 = full R.

---

## Controls

| Control | Per-channel | Notes |
|---|---|---|
| GAIN | 0–2× | Unity at 12 o'clock |
| TONE | 0–1 | Low-pass filter — 0 = dark (≈400Hz), 1 = full open |
| MUTE | Toggle | Silences channel, preserves pan position |
| MAIN | 0–1 | Master output level, followed by soft tanh saturation |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| CH1–4 IN | Input | Per-channel audio (mono or poly summed) |
| L OUT / R OUT | Output | Stereo master |
| MAIN OUT | Output | Summed mono (pre-pan) |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Fixed panning

The fixed pan spread is intentional design — the four channels map to four spatial positions with no overlap. This lets you layer sources at different DETUNE values and have them naturally separate in the stereo field without needing a pan module.

For dynamic stereo control, use **[[Wall-Conductor]]** WIDTH downstream.

---

## Patch tips

- **TONE as instrument**: pull CH2 TONE to 0.2 for a mid-cut telephone texture while CH1/3/4 stay open — creates depth without EQ modules.
- **GAIN above unity + MAIN low**: push individual channel gains to 1.5–2× for saturation colour on that channel while keeping overall level controlled.
- **MUTE as performance**: patch Pulse GATE outs to MUTE CV inputs for rhythmic gating of drone layers.
- **The Drone Bed patch** uses 4 DroneCore instances, each into a Choke channel, with Drift modulating their DETUNE and TIMBRE. See [[Playbooks]].

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneCore]] / [[DroneClone]] | Primary channel sources |
| [[Pulse]] | GATE → MUTE CVs for rhythmic choke |
| [[Collapse-Saturator]] | Post-mix saturation |
| [[Wall-Conductor]] | Choke feeds WallConductor CH inputs |

---

## See also

[[Wall-Conductor]] · [[Pulse]] · [[DroneCore]] · [[Playbooks]]
