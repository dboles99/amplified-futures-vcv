# Collapse Saturator — 12 HP

![Collapse Saturator in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/CollapseSat.png)

Stereo drive and saturation with a collapse gate. DRIVE sets pre-gain from ×1 to ×10, BUZZ picks the character of the clipping, and COLLAPSE is the performance control — a gate that ramps drive to maximum in a millisecond and then eases back to where you left it over RECOVERY.

COLLAPSE adds distortion rather than removing it. It is an event, not a limiter.

---

## Sound in 60 seconds

1. Add Collapse Saturator at the end of a chain. Patch **IN L**/**IN R** in and **OUT L**/**OUT R** to your interface.
2. DRIVE starts at 30%, BUZZ on ODD. The signal is already gently driven.
3. Raise **DRIVE** towards 100%. Pre-gain climbs to ×10 and the tanh takes over.
4. Switch **BUZZ** to EVEN, then FULL. Odd-harmonic warmth, then asymmetric tape colour, then hard-clipped fuzz.
5. Send a gate into **COLLAPSE**. Drive slams to maximum, then recovers over the time **RECOVERY** sets — up to about two seconds.

---

## Signal flow

~~~text
SIDECHAIN IN ──► |level| / 5 × 0.5 ──┐
                                      ├──► effective DRIVE
DRIVE knob ──────────────────────────┘         │
                                                │
COLLAPSE gate ──► env: 1 ms attack,             │
                  RECOVERY release  ────────────┤
                  (pushes drive to full)        │
                                                ▼
IN L ──►┐                              preGain = 1 + drive × 9   (×1 … ×10)
IN R ──►┤                                       │
        └──────────────────────────────► × preGain
                                                │
                        BUZZ ──► ODD  : symmetric tanh
                                 EVEN : asymmetric, tape-like
                                 FULL : hard clip
                                                │
                                        OUT L / OUT R
V/OCT IN ─────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![Collapse Saturator panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/CollapseSat.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| DRIVE | 0–100% | 30% | Pre-gain into the saturator, ×1 at zero to ×10 at full |
| BUZZ | ODD / EVEN / FULL | ODD | Saturation character. Three-position switch |
| RECOVERY | 0–100% | 30% | How long drive takes to fall back after COLLAPSE: 10 ms to about 2 s |

DRIVE and RECOVERY have an attenuverter (−1 to +1) and a CV input. BUZZ is a switch.

### The three BUZZ characters

| Setting | Curve | Sounds like |
|---|---|---|
| **ODD** | Symmetric tanh | Tube-like. Odd harmonics, compresses gracefully |
| **EVEN** | Asymmetric | Tape-like. Even harmonics, warmer and less symmetrical |
| **FULL** | Hard clip | Fuzz. The full harmonic spectrum, no politeness |

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| IN L / IN R | Input | Stereo audio in |
| DRIVE CV / RECOVERY CV | Input | Via their attenuverters |
| COLLAPSE | Input | Gate — 1 V or above ramps drive to maximum in 1 ms |
| SIDECHAIN | Input | Adds drive in proportion to its own level, up to +50% |
| OUT L / OUT R | Output | Stereo audio out |
| V/OCT IN → THRU | In / Out | Pass-through |

The SIDECHAIN input is the least obvious control on the panel and the most useful. It is not a compressor sidechain — it *raises* drive as its input gets louder. Patch [[Pulse]] into it and every percussion hit briefly distorts whatever is passing through.

---

## Patch recipes

**Transient colour.** [[Pulse]] OUT → SIDECHAIN while a drone passes through IN L/R. The drone distorts on each hit and cleans up between them, without the percussion itself being audible in the output.

**Collapse as punctuation.** [[Drift]] GATE → COLLAPSE, RECOVERY 60%. Occasional slams into full drive that ease back over a second or so.

**Tape warmth.** BUZZ EVEN, DRIVE 40%, nothing patched to COLLAPSE. Asymmetric saturation as a permanent output stage.

**Parallel fuzz.** [[Mass-Driver]] AUX L/R → IN L/R, BUZZ FULL, DRIVE 90%. The clean pre-PRESSURE mix gets hard-clipped separately and can be blended back against Mass Driver's own OUT.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Wall-Conductor]] | L/R OUT → IN for edge past what PRESSURE gives |
| [[Mass-Driver]] | AUX L/R → IN for a second, differently-shaped distortion path |
| [[Pulse]] | OUT → SIDECHAIN for rhythmic drive without rhythmic content |
| [[Drift]] | GATE → COLLAPSE for unpredictable distortion events |
| [[DroneClone]] | OUT → IN; EVEN mode suits the string wall particularly well |

---

## See also

[[Feedback-Governor]] · [[Wall-Conductor]] · [[Mass-Driver]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/CollapseSat.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/CollapseSat.md)
