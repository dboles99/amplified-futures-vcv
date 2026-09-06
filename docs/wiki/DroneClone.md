# DroneClone — 26 HP

![DroneClone in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/DroneClone.png)

Eight-voice amplified string wall, and the centrepiece of the Amplified Futures signal chain. Each polyphonic channel gets its own bank of eight detuned voices, so sixteen channels of V/OCT drive 128 simultaneous oscillators. TENSION and JAWARI shape the harmonic character between glassy and buzzing; CHOKE collapses the whole wall on a gate, and RTN takes a feedback return so the wall can feed itself.

---

## Sound in 60 seconds

1. Add DroneClone. Patch any V/OCT source into **V/OCT IN**, and **OUT** to your audio interface.
2. Set **MASS** to 4 and **TENSION** to 0.3. Leave everything else at zero.
3. Hold a note. You hear a four-voice wall.
4. Raise **JAWARI** to 0.15 — a subtle rattle appears on top of the fundamental.
5. Press the **CHOKE** button. The wall collapses. **CHOKE AMT** sets how far.

---

## Signal flow

~~~text
V/OCT IN (poly) ──► per-channel × per-voice oscillator bank
                     ├─ FUNDAMENTAL: ±2 octave offset
                     ├─ SPREAD:  per-voice detune around the fundamental
                     ├─ MASS:    active voice count 1–8
                     ├─ TENSION: odd-harmonic saw content
                     ├─ WEIGHT:  sub-octave body mix
                     ├─ SHIMMER: upper partials, 4th–8th
                     ├─ JAWARI:  even-harmonic rattle
                     ├─ DRIFT:   per-voice slow phase wander
                     └─ DECAY:   voice envelope decay

RTN IN ─────────────────────────────► mixed into the oscillator input
CHOKE BTN / CHOKE IN (gate) ────────► amplitude collapse, depth = CHOKE AMT

sum per channel ──► 1/√8 normalise ──► tanh ──► OUT (poly)
V/OCT IN ────────────────────────────────────────────► V/OCT THRU
~~~

---

## Controls

![DroneClone panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/DroneClone.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| FUNDAMENTAL | −2 to +2 Oct | 0 | Pitch offset applied over V/OCT IN |
| SPREAD | 0–1 | 0 | Per-voice detune spread around the fundamental |
| MASS | 1–8 | 4 | Active voices per channel — the density control |
| TENSION | 0–1 | 0 | Odd-harmonic saw content; the edge |
| WEIGHT | 0–1 | 0 | Sub-octave body mix |
| SHIMMER | 0–1 | 0 | Upper partial brightness, 4th–8th harmonics |
| JAWARI | 0–1 | 0 | Even-harmonic rattle — asymmetric buzz, after the sitar bridge |
| DRIFT | 0–1 | 0 | Per-voice slow phase wander rate |
| DECAY | 0–1 | 0.3 | Voice envelope decay; shapes how the wall sustains |
| CHOKE AMT | 0–1 | 0.5 | How far the CHOKE collapse goes |
| CHOKE | momentary | — | Button. Collapses the wall while held |

Every knob except CHOKE has an attenuverter (−1 to +1) and a CV input.

### TENSION against JAWARI

The two harmonic controls are the ones worth learning. TENSION adds odd harmonics, JAWARI even ones, and the pair covers most of the module's range:

| TENSION | JAWARI | Character |
|---|---|---|
| 0.0 | 0.0 | Pure sine wall — glassy, clean |
| 0.4 | 0.0 | Saw-edged, no-wave guitar |
| 0.0 | 0.3 | Sitar-like buzz |
| 0.4 | 0.2 | The canonical Amplified Futures wall |
| 0.8 | 0.5 | Aggressive harmonic chaos |

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| V/OCT IN | Input | Polyphonic pitch; sets the channel count |
| RTN | Input | Feedback return, mixed into the oscillator input |
| CHOKE | Input | Gate — 10 V collapses all voices immediately |
| CV ×10 | Input | One per knob, each via its attenuverter |
| OUT | Output | Polyphonic mixed audio, 1/√8 normalised then soft-clipped |
| V/OCT THRU | Output | Pass-through at the same channel count |

---

## Patch recipes

**No-wave guitar wall.** TENSION 0.4, JAWARI 0.2, MASS 4, SHIMMER 0, WEIGHT 0.1, DRIFT 0.15, into [[Wall-Conductor]]. The canonical sound.

**Sitar swarm.** JAWARI 0.6, SHIMMER 0.3, TENSION 0.1, MASS 6, DRIFT 0.3, fed from [[Harmonic-Pressure]] at eight partials. Add [[Drift]] SMOOTH → JAWARI CV for a buzz that breathes.

**Sub wall.** WEIGHT 0.5, TENSION 0.1, MASS 3, JAWARI 0, SHIMMER 0, FUNDAMENTAL −1 Oct. Sits beneath a second DroneClone running the mids.

**Maximum density.** MASS 8, TENSION 0.6, SHIMMER 0.4, JAWARI 0.3, DRIFT 0.4. Full-spectrum maximalism — put [[Feedback-Governor]] in the RTN path or it runs away.

---

## Known pairings

| Module | Routing |
|---|---|
| [[Wall-Conductor]] | Primary section input |
| [[Collapse-Saturator]] | Post-wall saturation and collapse |
| [[Harmonic-Pressure]] | V/OCT for harmonic-series chord stacks |
| [[Drift]] | DRIFT or JAWARI CV for externally paced wander |
| [[Send]] | RTN bus for cross-feedback between two DroneClones |
| [[Feedback-Governor]] | OUT → SEND → RTN, a controlled loop |

---

## Factory presets

Two presets from the pitch research. `SPREAD` is quoted here as the spacing
between **adjacent** voices - `knob x 1200 / 7` - because adjacent voices are
what you hear beating. The extremes of the stack are 1200 cents apart at
SPREAD 1.0, which is a different note rather than a beat. The module's
right-click menu reports the resulting beat rate at the current pitch.

**Vinyl Wow** — SPREAD 0.0505, i.e. **8.656 cents** between neighbours. That
is not a taste value: it is `1731.234 x 0.5 / 100`, the pitch deviation of a
pressing whose spindle hole is 0.5 mm off centre, read at 100 mm groove
radius. The matching wow *rate* is 0.556 Hz, once per revolution at 33 1/3 rpm.

  Honest limitation: DroneClone has no drift-*rate* control - its drift speeds
  are fixed in the module - so 0.556 Hz cannot be dialled in. The preset sets
  the depth correctly and approximates the rate. Read it as "wow of the right
  size", not a realisation of the figure.

**Disco Strings** — SPREAD 0.1021, i.e. **17.5 cents** between neighbours,
the midpoint of the 10-25 cent range measured as F0 dispersion across a large
string section, with all eight voices running. Detune wide enough to read as a
section rather than a chorus pedal.

## See also

[[DroneCore]] · [[String-Mass-Core]] · [[Send]] · [[Feedback-Governor]] · [[Wall-Conductor]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/DroneClone.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/DroneClone.md)
