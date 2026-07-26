# DroneCore — 8 HP

![DroneCore in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/DroneCore.png)

Two-voice detuned oscillator core, and the base voice of the Amplified Futures stack. A pair of sine-to-harmonic oscillators run symmetrically detuned around a shared pitch centre, so a single DETUNE knob widens or narrows the beating between them. At 8 HP it is the cheapest way to add mass to a patch — stack several, or run one polyphonically from Harmonic Pressure.

---

## Sound in 60 seconds

1. Add DroneCore. Patch **OUT** to your audio interface.
2. It sounds immediately — PITCH defaults to 220 Hz, a pure sine.
3. Turn **TIMBRE** up. The 2nd, 3rd and 4th harmonics fade in; the tone hardens into a third-bridge stack.
4. Turn **DETUNE** to about 30¢. The two voices separate and begin to beat.
5. Push **DETUNE** past 80¢ for interference rather than warmth.

---

## Signal flow

~~~text
V/OCT IN ──► [pitch + detune] ──► Voice A (+detune/2) ──►┐
                                  Voice B (−detune/2) ──►┤ mix ──► OUT
                                                          │
                              [TIMBRE] blends 2nd/3rd/4th harmonics
V/OCT IN ────────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![DroneCore panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/DroneCore.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| PITCH | 82–1319 Hz | 220 Hz | Base frequency, roughly A2–E6 |
| DETUNE | 0–100¢ | 12¢ | Symmetric split — each voice moves ±DETUNE/2 |
| TIMBRE | 0–1 | 0 | 0 is a pure sine; 1 adds the 2nd, 3rd and 4th harmonics |

Every knob has an attenuverter (−1 to +1) and a CV input (±5 V bipolar, scaled by the attenuverter).

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic pitch input; sets the channel count |
| MOD | Input | CV for PITCH, via its attenuverter |
| CV ×2 | Input | CV for DETUNE and TIMBRE |
| OUT | Output | Mixed dual-voice audio, polyphonic |
| V/OCT THRU | Output | Pass-through at the same channel count |

---

## Patch recipes

**Orchestral spread.** Four DroneCores in unison at DETUNE 6¢, 12¢, 18¢ and 24¢, summed through [[Choke]]. Warmth and width without reaching for DroneClone.

**Breathing harmonics.** [[Drift]] SMOOTH output into TIMBRE CV, attenuverter at +0.4, Drift RATE around 0.2 Hz. The harmonic stack fades in and out over roughly ten-second cycles.

**Polyphonic chord stack.** [[Harmonic-Pressure]] V/OCT OUT (COUNT 8, MODE JUST) into V/OCT IN. Each channel gets its own detuned pair — sixteen oscillators from one 8 HP module.

**Interference bed.** One DroneCore at DETUNE 95¢, TIMBRE 0, into [[Feedback-Governor]] at DECAY 0.6. The beating feeds itself and never quite settles.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | DroneCore an octave below as sub-fundamental |
| [[Choke]] | Per-voice gain and muting across a DroneCore stack |
| [[Drift]] | SMOOTH → TIMBRE CV for slow harmonic wander |
| [[Harmonic-Pressure]] | V/OCT OUT → V/OCT IN for microtonal partial stacks |

---

## See also

[[DroneClone]] · [[String-Mass-Core]] · [[Drift]] · [[Harmonic-Pressure]] · [[Music-Theory]]

**Full parameter spec:** [`docs/modules/DroneCore.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/DroneCore.md)
