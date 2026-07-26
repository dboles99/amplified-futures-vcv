# Collapse Saturator — 12HP

![Collapse Saturator panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/CollapseSat.png)

Stereo drive/saturation with collapse. Three harmonic modes (ODD/EVEN/FULL), COLLAPSE gate that instantly maxes drive, shaped RECOVERY, and a sidechain input for external envelope control. The final shaping stage before output.

---

## Signal flow

```
IN L / IN R ──► DRIVE (pre-gain 0–4×) ──► BUZZ (saturation mode)
                                           ├─ ODD:  tanh(x)                 — symmetric, odd harmonics
                                           ├─ EVEN: tanh(x+0.35)−tanh(0.35) — DC-free asymmetric, tape-like
                                           └─ FULL: hard clip clamp(x,−1,1)  — brutal clipping

SC IN ──► sidechain boost on DRIVE (+0–50% additive)

COLLAPSE BTN/IN ──► instant collapseEnv → 0 (maxes effective drive, blend to hard clip)
RECOVERY ──────► envelope rise time 10ms–2s

──► OUT L / OUT R
V/OCT IN ──────► V/OCT THRU
```

---

## Controls

| Control | Range | Notes |
|---|---|---|
| DRIVE | 0–1 | Pre-gain 1–4×. Higher = more saturation character |
| BUZZ | ODD / EVEN / FULL | Harmonic character switch (3-way) |
| RECOVERY | 0–1 | Post-collapse recovery time: 0 = 10ms (snap back), 1 = 2s (slow rise) |
| COLLAPSE | Button | Momentary performance button — triggers collapse |

DRIVE has attenuverter + CV. RECOVERY has attenuverter + CV.

---

## BUZZ modes

Each mode adds different harmonics to the signal:

| Mode | Algorithm | Harmonics added | Character |
|---|---|---|---|
| ODD | `tanh(x)` | 3rd, 5th, 7th (odd series) | Symmetric drive — classic saturation, "amplifier overdrive" |
| EVEN | `tanh(x+b) − tanh(b)` (b=0.35) | 2nd harmonic, DC-free | Asymmetric tape-like warmth — adds octave-doubling character |
| FULL | `clamp(x, −1, 1)` | Hard clip — all harmonics | Brutal — square wave character at high DRIVE |

During COLLAPSE, ODD and EVEN modes blend toward hard clip as collapseEnv rises — the saturation character automatically shifts to FULL at peak collapse.

**A note on harmonics:** The ODD mode adds odd-order harmonics which are harmonically related to the original pitch (5th, 3rd above). The EVEN mode adds even-order harmonics which produce an octave doubling effect. FULL adds both in unlimited amounts.

---

## Ports

| Port | Type | Notes |
|---|---|---|
| IN L / IN R | Input | Stereo audio input |
| SC IN | Input | Sidechain — adds to DRIVE amount |
| COLLAPSE IN | Input | Gate: high = collapse engaged |
| DRIVE CV | Input | CV for DRIVE (with attenuverter) |
| RECOVERY CV | Input | CV for RECOVERY |
| OUT L / OUT R | Output | Stereo saturated output |
| V/OCT IN | Input | Pass-through |
| V/OCT THRU | Output | Pass-through |

---

## Patch tips

- **EVEN mode + DRIVE 0.3**: subtle tape saturation on dense wall mixes without obvious clipping.
- **ODD mode + DRIVE 0.6–0.8**: canonical heavy drive. Works on DroneClone wall output for added grit.
- **COLLAPSE → long RECOVERY (0.7)**: press COLLAPSE for a dramatic harmonic explosion that slowly settles — a signature performance gesture.
- **SC IN from Pulse OUT**: percussion transients briefly boost DRIVE for rhythmic saturation pumping.
- **Chain after WallConductor**: WALL → CollapseSat → master out. The natural position in the signal chain.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Wall-Conductor]] | Primary input source |
| [[Pulse]] | OUT → SC IN for sidechain drive pumping |
| [[Feedback-Governor]] | Route output back into feedback chain |
| [[Drift]] | GATE → COLLAPSE IN for periodic timed collapse events |

---

## See also

[[Wall-Conductor]] · [[Feedback-Governor]] · [[Drift]] · [[Music-Theory]]
