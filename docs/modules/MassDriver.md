# Mass Driver (AF-01)

**32HP** | Mixer / Performance

16-channel no-wave signal mixer. Per-channel GAIN and MUTE. Master DENSITY sweep, PRESSURE saturation, WIDTH stereo spread, FEEDBACK loop, COLLAPSE gate. Five outputs: OUT L/R, AUX L/R, SUM.

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
    ├─────────────────────────────────┐
    │                                 │
    AUX L/R (pre-pressure stereo)    SUM (mono pre-pressure)
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
| DENSITY | 0–1 | Sweeps channels in — at 0 only ch1 active, at 1 all 16 |
| DENSITY CV | ±1 atten | CV over DENSITY |
| PRESSURE | 0–1 | tanh saturation drive (0=clean, 1=hard clip) |
| PRESSURE CV | ±1 atten | CV over PRESSURE |
| WIDTH | 0–1 | Stereo spread — 0=mono centre, 1=channels hard L↔R |
| WIDTH CV | ±1 atten | CV over WIDTH |
| MASS | 0–1 | Master output level (0–5V scale) |
| MASS CV | ±1 atten | CV over MASS |
| FEEDBACK | 0–1 | Feedback loop amount (capped at 0.92× per pass) |
| FEEDBACK CV | ±1 atten | CV over FEEDBACK |
| COLLAPSE | Button/gate | Instantly drops output to zero; 1.5s recovery |

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
| OUT L / OUT R | Audio out | Full processed stereo (post-pressure, post-collapse) |
| AUX L / AUX R | Audio out | Pre-pressure stereo (use for parallel compression or dry send) |
| SUM | Audio out | Mono pre-pressure sum |
| V/OCT | Thru out | V/OCT passthrough |

---

## DENSITY sweep behaviour

DENSITY sweeps channels in from 1 to 16. Each channel's contribution is `clamp(DENSITY×16 − i, 0, 1)` where i is the 0-indexed channel number. At DENSITY=0.25 only channels 1–4 are at full level; at DENSITY=1.0 all 16 channels are fully open. This gives a smooth mass-building gesture from a single fader.

---

## COLLAPSE

Pressing or gating COLLAPSE instantly zeros the output. The signal recovers with a 1.5-second exponential return to full level. Use as a performance gesture — the climax, not a reset. At high FEEDBACK values, COLLAPSE also clears the feedback state, preventing runaway on re-entry.

---

## AUX outputs

AUX L/R taps the signal after DENSITY and WIDTH but before PRESSURE and COLLAPSE. Use for:
- Parallel compression (process AUX separately and blend back)
- Clean reference send to a reverb or delay
- Pre-saturation recording

---

## Patch tips

- **Mass build**: Start DENSITY at 0, PRESSURE at 0. Slowly raise DENSITY to bring in channel mass. Then push PRESSURE for harmonic density.
- **Feedback wall**: Set FEEDBACK to 0.6–0.75. Keep MASS below 0.5 to prevent runaway. Use COLLAPSE to reset.
- **Section separation**: Route different source types (oscillators, string masses, percussion) to channel groups. DENSITY sweeps them in as one gesture.
- **V/OCT thru**: Patch a keyboard or sequencer V/OCT through the V/OCT in → out for pitch-tracking integration without breaking the signal chain.

---

## Known pairings

| Module | Role |
|---|---|
| StringMassCore × 4 | 16 voices of harmonic mass per channel bank |
| HarmonicPressure | Polyphonic V/OCT into StringMassCore → Mass Driver |
| WallConductor | Feeds Mass Driver AUX for a second-stage DENSITY+COLLAPSE layer |
| FeedbackGovernor | External feedback loop in parallel with Mass Driver FEEDBACK |
| CollapseSat | Post-process OUT L/R for additional harmonic character |
| Drift | CV into DENSITY for slow autonomous mass evolution |
