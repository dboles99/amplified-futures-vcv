# Collapse Saturator — 12HP

Stereo drive/saturation with collapse gate. Three harmonic modes (ODD/EVEN/FULL), COLLAPSE gate that instantly maxes drive and shifts mode toward hard clip, shaped RECOVERY, and a sidechain input for external envelope boost. The final shaping stage before output.

---

## Signal flow

```text
IN L / IN R ──► DRIVE pre-gain (×1–×10) ──► BUZZ saturation mode
                                              ├─ ODD:  tanh(x)                  — symmetric, odd harmonics
                                              ├─ EVEN: tanh(x+0.35)−tanh(0.35) — DC-free asymmetric, tape-like
                                              └─ FULL: clamp(x, −1, 1)          — brutal hard clip

SC IN ──► sidechain drive boost (+0–50% additive, proportional to SC signal level)

COLLAPSE BTN / COLLAPSE IN ──► collapseEnv → 1 instantly (maxes effective drive)
                               during collapse: ODD/EVEN blend toward FULL character
RECOVERY ──────────────────► collapseEnv decays: 0 = 10 ms snap back, 1 = 2 s slow

OUT L / OUT R ──► stereo saturated output
V/OCT IN ──────────────────────────────────────────────────────────────────► V/OCT THRU
```

---

## Controls table

| Param | Index | Range | Default | Notes |
| --- | --- | --- | --- | --- |
| DRIVE | 0 | 0–1 | 0.3 | Pre-gain: 0 = ×1 (clean), 1 = ×10 (heavy) |
| DRIVE ATTEN | 1 | −1 to +1 | 0 | Attenuverter for DRIVE CV |
| BUZZ | 2 | 0–2 | 0 | 0 = ODD, 1 = EVEN, 2 = FULL. Snap-enabled switch |
| RECOVERY | 3 | 0–1 | 0.3 | Post-collapse recovery: 0 = 10 ms, 1 = 2 s |
| RECOVERY ATTEN | 4 | −1 to +1 | 0 | Attenuverter for RECOVERY CV |
| LEVEL | 5 | 0–1 | 0.5 | Output level after the drive stage. Default reproduces pre-2.3.0 behaviour |
| MIX | 6 | 0–1 | 1 | Dry/wet between the input and the saturated signal. 1 = fully saturated |
| SC AMT | 7 | 0–1 | 1 | How strongly the sidechain input drives DRIVE |

---

## BUZZ modes

| Mode | Character | Best for |
| --- | --- | --- |
| ODD | Symmetric tanh — adds 3rd, 5th, 7th harmonics. Tube-like | General saturation, wall drives |
| EVEN | Asymmetric tape-like warmth — adds 2nd harmonics, DC-free via bias subtract | Subtle density, string walls |
| FULL | Hard clip — brutal square-wave edges at high DRIVE | Noise maximalism, extreme collapse |

During COLLAPSE: ODD and EVEN modes blend progressively toward FULL as collapseEnv rises. At peak collapse, all modes are effectively hard clip.

---

## Ports table

| Port | Direction | Type | Notes |
| --- | --- | --- | --- |
| IN L | Input | Audio | Left stereo input |
| IN R | Input | Audio | Right stereo input |
| SC IN | Input | Audio / CV | Sidechain — boosts DRIVE proportional to signal level |
| COLLAPSE IN | Input | Gate | High = collapse engaged |
| DRIVE CV | Input | CV | CV for DRIVE (scaled by DRIVE ATTEN) |
| RECOVERY CV | Input | CV | CV for RECOVERY |
| V/OCT IN | Input | CV | Pass-through |
| OUT L | Output | Audio | Left stereo output |
| OUT R | Output | Audio | Right stereo output |
| V/OCT THRU | Output | CV | Pass-through |

---

## MIDI CC automation

| Param index | Parameter | Recommended CC |
| --- | --- | --- |
| 0 | DRIVE | CC 11 (expression — primary performance control) |
| 2 | BUZZ | CC 14 (0–42 = ODD, 43–84 = EVEN, 85–127 = FULL) |
| 3 | RECOVERY | CC 15 |

COLLAPSE button (no CC index — it is a gate input) can be triggered from a MIDI-to-CV gate on any note or pad.

---

## Recommended configurations

**Subtle Tape Warmth** — BUZZ EVEN, DRIVE 0.2, RECOVERY 0.1. Barely perceptible saturation on a dense wall mix. Adds density without obvious clipping. SC IN from Pulse for transient colouring.

**Heavy Drive** — BUZZ ODD, DRIVE 0.65, RECOVERY 0.3. Canonical guitar-wall saturation. Works on DroneClone output for added grit. DRIVE CV from Drift SMOOTH for slow drive breathing.

**Collapse Event** — BUZZ ODD, DRIVE 0.4, RECOVERY 0.7 (≈ 1.5 s). Press COLLAPSE — the saturation explodes into hard clip, then slowly recovers. The signature Amplified Futures performance gesture.

**Sidechain Pump** — BUZZ EVEN, DRIVE 0.3. Pulse OUT → SC IN. Every percussion hit briefly boosts DRIVE by the pulse amplitude. Creates a pumping, rhythmically-aware saturation texture.

---

## Basic setup — sound in 60 seconds

1. Add CollapseSat to your patch.
2. Patch WallConductor OUT L → CollapseSat IN L. OUT R → IN R.
3. Patch CollapseSat OUT L and OUT R → your audio interface.
4. Set BUZZ to ODD, DRIVE to 0.35, RECOVERY to 0.3.
5. You have light saturation on the wall mix.
6. Press COLLAPSE. The drive spikes, clips hard, then recovers over about 0.5 s.
7. Adjust RECOVERY: 0.7 gives a 1+ s shaped recovery arc.

---

## How-tos

### Sidechain percussive pump

- Patch Pulse OUT → CollapseSat SC IN.
- Set BUZZ to EVEN, DRIVE to 0.25.
- Each time Pulse fires, the signal level from Pulse OUT boosts DRIVE briefly.
- The wall saturation pumps in rhythm with the percussion. Keep base DRIVE low so SC has room to push.

### DRIVE automation for harmonic build

- Map DRIVE to MIDI CC 11 (expression pedal).
- At CC 0: clean pass-through (DRIVE ≈ 0.05). At CC 127: heavy clipping.
- Sweep slowly over a performance section — the wall transitions from transparent to dense.
- BUZZ EVEN during the sweep preserves warmth; switch to FULL at maximum for a noise event.

### Collapse into feedback

- Patch CollapseSat OUT → FeedbackGovernor SEND.
- When COLLAPSE fires, the burst of hard-clipped signal enters the feedback path.
- FeedbackGovernor TONE 0.5, DECAY 0.3 — the clipped burst decays as a filtered tail.
- Hit FeedbackGovernor KILL to clear the burst before re-entry.

### BUZZ mode switching live

- Map BUZZ to MIDI CC 14.
- Programme transitions: CC 20 (ODD) → CC 65 (EVEN) → CC 100 (FULL) during a performance.
- FULL at the peak of a collapse section, then back to EVEN as RECOVERY completes.

---

## Commiecore integration

| Mode | Role |
| --- | --- |
| DAMAGE (managed-collapse) | COLLAPSE button — the defining damage gesture; character shifts to FULL at peak |
| PRESSURE (rent-pressure) | DRIVE swept up over time — accumulating harmonic pressure on the mix |
| FAILURE (managed-collapse) | COLLAPSE + long RECOVERY — failure event with shaped return |
| CARE (mutual-aid) | EVEN mode + DRIVE 0.1 — barely present saturation, warmth without aggression |
| COLLECTIVE (collective-refusal) | SC IN from Pulse — the collective percussion drives the saturation together |

---

## Known pairings

| Module | Role |
| --- | --- |
| WallConductor | Primary input — OUT L/R → CollapseSat IN L/R |
| Pulse | OUT → SC IN for sidechain drive pumping |
| FeedbackGovernor | CollapseSat OUT → SEND for governed post-collapse feedback tail |
| Drift | SMOOTH → DRIVE CV for slow saturation breathing |
| DroneClone | OUT → CollapseSat IN for per-instance saturation before mixing |
