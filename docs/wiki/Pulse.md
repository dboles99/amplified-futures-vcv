# Pulse — 12HP

![Pulse panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/Pulse.png)

16-step no-wave step percussion sequencer. 4×4 toggle grid, white noise synthesis with HIT level, DECAY time, METAL filter, and CRACK transient burst. TRG clock in, audio out. Inspired by primitive drum machine aesthetics and no-wave percussive attack.

---

## Signal flow

```
TRG IN ──► [step counter] ──► active step? ──► [noise burst]
                                               ├─ HIT   : amplitude
                                               ├─ DECAY : 8–500ms exponential decay
                                               ├─ METAL : LP filter (360→80Hz)
                                               └─ CRACK : 4ms transient burst (attack click)
                                               └──► OUT
```

The 4×4 grid is 16 steps in rows of 4. Each lit button = that step fires when clocked.

---

## Controls

| Control | Range | Notes |
|---|---|---|
| HIT | 0–1 | Peak amplitude of triggered burst |
| DECAY | 0–1 | 0 = 8ms (tight click), 1 = 500ms (long thud) |
| METAL | 0–1 | Low-pass filter on noise — 0 = open (hi-hat), 1 = dark (kick/thud) |
| CRACK | 0–1 | Adds 4ms sharp transient on top of noise burst — "attack click" |
| Grid (4×4) | Toggle | 16 step on/off buttons |

---

## Ports

| Port | Type | Notes |
|---|---|---|
| TRG IN | Input | Clock pulse — fires next step on rising edge |
| OUT | Output | Percussive noise audio |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Sound shaping guide

The four controls interact to produce all percussion types from this white-noise engine:

| Style | DECAY | METAL | CRACK | Character |
|---|---|---|---|---|
| Hi-hat closed | 0.0 | 0.0 | 0.3 | Short click, open noise |
| Hi-hat open | 0.3 | 0.0 | 0.1 | Longer hiss |
| Snare-like | 0.2 | 0.3 | 0.7 | Mid-filtered with crack |
| Kick/thud | 0.6 | 0.8 | 0.2 | Dark low thud |
| Noise burst | 0.8 | 0.0 | 0.0 | Long open noise hit |
| Click only | 0.0 | 0.0 | 1.0 | Pure transient, no tail |

---

## Patch tips

- **Run two Pulse modules**: one for hi-hat texture (METAL 0, DECAY 0.1), one for thud (METAL 0.7, DECAY 0.5). Offset their grids.
- **HIT CV from Drift** STEP output: stochastic velocity for humanised irregular feel.
- **Slow clock (0.5–2Hz) + full grid on**: produces a continuous noise texture with slow amplitude envelope — useful as a modulation source or noise bed.
- **GATE → MUTE CV** on Choke: rhythmic gating of drone channels on each hit.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Drift]] | GATE output as irregular clock |
| [[Choke]] | GATE → MUTE CVs for rhythmic drone gating |
| [[Wall-Conductor]] | GATE → COLLAPSE IN for beat-synced drops |
| [[Collapse-Saturator]] | OUT → SC IN for sidechain drive pumping |
| [[Feedback-Governor]] | GATE → KILL GATE for rhythmic feedback chopping |

---

## See also

[[Drift]] · [[Choke]] · [[Wall-Conductor]] · [[Playbooks]]
