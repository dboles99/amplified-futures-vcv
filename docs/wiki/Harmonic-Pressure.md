# Harmonic Pressure — 14 HP

![Harmonic Pressure in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/HarmonicPressure.png)

Harmonic series pitch CV generator, and the tuning source the rest of the system is built around. It emits a polyphonic V/OCT where every channel is one partial of the harmonic series above a root — no quantiser, no scale table, just the ratios themselves. Patch it into String Mass Core or DroneClone and the mass tunes itself.

---

## Sound in 60 seconds

1. Add Harmonic Pressure and [[String-Mass-Core]]. Patch **V/OCT OUT** → String Mass Core **V/OCT IN**, and its OUT to your interface.
2. Harmonic Pressure starts at PARTIAL 1, COUNT 8, TUNING JUST — eight partials of the harmonic series.
3. You hear a stack built on pure ratios. Nothing is tempered.
4. Raise **PARTIAL** to 4. The stack starts on the fourth harmonic instead of the fundamental — the same series, higher and tighter.
5. Turn **TUNING** to EQUAL. Every partial snaps to the nearest 12-TET semitone and the stack loses its lock. Turn it back.

---

## Signal flow

~~~text
V/OCT IN (root) ──► + PITCH offset (±2 Oct)
                          │
                          ├─ partial n = PARTIAL … PARTIAL+COUNT−1
                          │  voct(n) = root + log2(n)
                          │
                          ├─ TUNING JUST:  exact ratios, untouched
                          ├─ TUNING EQUAL: each partial rounded to
                          │                 the nearest 12-TET semitone
                          └─ TUNING MICRO: JI plus a deterministic
                                            per-partial offset
                          │
                     + SPREAD ensemble detune
                          │
                          ▼
              V/OCT OUT — COUNT polyphonic channels
~~~

---

## Controls

![Harmonic Pressure panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/HarmonicPressure.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| PITCH | −2 to +2 Oct | 0 | Octave offset applied to the root before the series is built |
| SPREAD | 0–100% | 0 | Per-partial ensemble detuning. In MICRO it sets the offset magnitude |
| PARTIAL | 1–16 | 1 | Which harmonic the series starts on. Snaps |
| COUNT | 1–16 | 8 | How many partials, and therefore how many output channels. Snaps |
| TUNING | JUST / EQUAL / MICRO | JUST | How the partials are tuned. Snaps |

PITCH and SPREAD have an attenuverter (−1 to +1) and a CV input. PARTIAL, COUNT and TUNING are knob-only.

### The three tuning modes

| Mode | What it does |
|---|---|
| **JUST** | Exact harmonic-series ratios — pure just intonation. The partials lock and beat against nothing |
| **EQUAL** | Each partial rounded to the nearest 12-TET semitone. Use when the stack has to sit with equal-tempered material |
| **MICRO** | Just intonation plus a deterministic per-partial offset, sized by SPREAD. Simulates an ensemble that is nearly, but not quite, in tune |

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| V/OCT IN | Input | Root pitch. The series is built above this |
| PITCH CV | Input | Octave offset, via its attenuverter |
| SPREAD CV | Input | Ensemble detune amount, via its attenuverter |
| V/OCT OUT | Output | Polyphonic — one channel per partial, COUNT channels wide |

---

## Patch recipes

**The intended pairing.** COUNT 8, PARTIAL 1, TUNING JUST into [[String-Mass-Core]] at MODE HARM, MASS 8. Eight partials, each carrying eight voices. This is the patch the whole system was designed around.

**Upper-partial shimmer.** PARTIAL 8, COUNT 8, TUNING JUST. Starting high in the series gives closely-spaced intervals — a shimmer band rather than a chord. Feed [[DroneCore]] for a cheap, bright stack.

**Detuned ensemble.** TUNING MICRO, SPREAD 40%, COUNT 6. Each partial sits slightly off its true ratio, and the offsets are deterministic, so the detuning is stable rather than drifting.

**Moving root.** [[Drift]] STEP output → PITCH CV at attenuverter +0.5. The whole harmonic series transposes in steps while keeping its internal tuning intact.

---

## Known pairings

| Module | Routing |
|---|---|
| [[String-Mass-Core]] | V/OCT OUT → V/OCT IN. The primary destination |
| [[DroneClone]] | V/OCT OUT → V/OCT IN for harmonic-series chord walls |
| [[DroneCore]] | V/OCT OUT → V/OCT IN; each channel gets its own detuned pair |
| [[Drift]] | STEP or SMOOTH → PITCH CV to move the root |

---

## See also

[[String-Mass-Core]] · [[DroneClone]] · [[DroneCore]] · [[Music-Theory]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/HarmonicPressure.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/HarmonicPressure.md)
