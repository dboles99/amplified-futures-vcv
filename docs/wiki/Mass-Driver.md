# Mass Driver — 32HP (AF-01)

![Mass Driver panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/MassDriver.png)

16-channel no-wave signal mixer. Per-channel GAIN and MUTE. Master DENSITY sweep, PRESSURE saturation, WIDTH stereo spread, FEEDBACK loop, COLLAPSE gate. Five outputs: OUT L/R, AUX L/R, SUM. The centrepiece of the AF-01 module series.

---

## Signal flow

```
CH 1–16 IN
    │
    ├─ MUTE (per-channel toggle)
    │
    ├─ GAIN (per-channel 0–2×)
    │
    ├─ DENSITY sweep (fades channels 1→16 in sequence)
    │
    ├─ WIDTH pan spread (linear L→R across 16 channels)
    │
    ├────────────────────────────────┐
    │                                │
    AUX L/R (pre-pressure stereo)   SUM (mono pre-pressure)
    │
    ├─ PRESSURE saturation (tanh)
    │
    ├─ FEEDBACK loop (1-sample, capped 0.92)
    │
    ├─ COLLAPSE gate (instant drop, 1.5s recovery)
    │
    └─ OUT L / OUT R
```

---

## Controls

| Control | Range | Function |
|---|---|---|
| GAIN 1–16 | 0–2× | Per-channel level |
| MUTE 1–16 | Toggle | Per-channel mute (state saved) |
| DENSITY | 0–1 | Sweeps channels 1→16 in progressively |
| PRESSURE | 0–1 | tanh saturation drive (0=clean, 1=hard clip) |
| WIDTH | 0–1 | Stereo spread — 0=mono centre, 1=channels hard L↔R |
| MASS | 0–1 | Master output level (0–5V scale) |
| FEEDBACK | 0–1 | Feedback loop amount (capped at 0.92× per pass) |
| COLLAPSE | Button/gate | Instantly drops output to zero; 1.5s recovery |

---

## DENSITY sweep behaviour

DENSITY sweeps channels in from 1 to 16. Each channel's contribution is `clamp(DENSITY×16 − i, 0, 1)` where i is the 0-indexed channel number.

| DENSITY | Channels fully active |
|---|---|
| 0.0 | None |
| 0.25 | CH 1–4 |
| 0.50 | CH 1–8 |
| 0.75 | CH 1–12 |
| 1.0 | All 16 |

At 0.25 only channels 1–4 are at full level; at 1.0 all 16 channels are fully open. This gives a smooth mass-building gesture from a single fader.

---

## AUX outputs

AUX L/R taps the signal after DENSITY and WIDTH but **before** PRESSURE and COLLAPSE. Use for:
- Parallel compression (process AUX separately and blend back)
- Clean reference send to reverb or delay
- Pre-saturation recording
- Sidechaining CollapseSat from a clean copy of the mix

---

## COLLAPSE

Pressing or gating COLLAPSE instantly zeros the output. The signal recovers with a 1.5-second exponential return to full level. At high FEEDBACK values, COLLAPSE also clears the feedback state, preventing runaway on re-entry.

---

## Ports

| Port | Type | Function |
|---|---|---|
| CH 1–16 | Audio in | Per-channel signal inputs |
| DENSITY CV | CV in | Modulates DENSITY knob |
| PRESSURE CV | CV in | Modulates PRESSURE knob |
| WIDTH CV | CV in | Modulates WIDTH knob |
| MASS CV | CV in | Modulates MASS knob |
| FEEDBACK CV | CV in | Modulates FEEDBACK knob |
| COLLAPSE | Gate in | Triggers COLLAPSE when high |
| V/OCT | Thru | V/OCT passthrough (unprocessed) |
| OUT L / OUT R | Audio out | Full processed stereo |
| AUX L / AUX R | Audio out | Pre-pressure stereo |
| SUM | Audio out | Mono pre-pressure sum |

---

## Patch tips

- **Mass build**: Start DENSITY at 0, PRESSURE at 0. Slowly raise DENSITY to bring in channel mass. Then push PRESSURE for harmonic density.
- **Feedback wall**: Set FEEDBACK to 0.6–0.75. Keep MASS below 0.5 to prevent runaway. Use COLLAPSE to reset.
- **Section separation**: Route different source types (oscillators, string masses, percussion) to channel groups. DENSITY sweeps them all in as one gesture.
- **32 channels**: Run two Mass Drivers in series — first handles CH 1–16, AUX feeds second MassDriver for a 32-channel system with two independent COLLAPSE events.

---

## Known pairings

| Module | Role |
|---|---|
| [[String-Mass-Core]] × 4 | 16 voices of harmonic mass per channel bank |
| [[Harmonic-Pressure]] | Polyphonic V/OCT → StringMassCore → Mass Driver |
| [[Wall-Conductor]] | Feeds Mass Driver AUX for a second-stage DENSITY+COLLAPSE layer |
| [[Feedback-Governor]] | External feedback loop in parallel with Mass Driver FEEDBACK |
| [[Collapse-Saturator]] | Post-process OUT L/R for additional harmonic character |
| [[Drift]] | DENSITY CV for slow autonomous mass evolution |

---

## See also

[[Wall-Conductor]] · [[String-Mass-Core]] · [[Feedback-Governor]] · [[Playbooks]]
