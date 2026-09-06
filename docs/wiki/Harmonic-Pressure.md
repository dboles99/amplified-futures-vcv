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
                          └─ TUNING DRIFT: exact ratios, moving —
                                            SPREAD/RATE/COHERENCE
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
| SPREAD | 0–100% | 0 | Per-partial detuning. In JUST and EQUAL a static offset; in DRIFT the depth, in cents, of the movement |
| PARTIAL | 1–16 | 1 | Which harmonic the series starts on. Snaps |
| COUNT | 1–16 | 8 | How many partials, and therefore how many output channels. Snaps |
| TUNING | JUST / EQUAL / DRIFT | JUST | How the partials are tuned. Snaps |
| DRIFT RATE | 0–4 Hz | 0 | How fast the drift moves. **DRIFT mode only** |
| DRIFT COH | 0–1 | 1 | 0 = the whole stack transposes together; 1 = partials drift independently. **DRIFT mode only** |

PITCH and SPREAD have an attenuverter (−1 to +1) and a CV input. PARTIAL, COUNT and TUNING are knob-only.

### The three tuning modes

| Mode | What it does |
|---|---|
| **JUST** | Exact harmonic-series ratios — pure just intonation. The partials lock and beat against nothing |
| **EQUAL** | Each partial rounded to the nearest 12-TET semitone. Use when the stack has to sit with equal-tempered material |
| **DRIFT** | Exact ratios, but the partials move. SPREAD sets the depth in cents, DRIFT RATE how fast, and DRIFT COHERENCE whether the movement is shared — a slow collective transposition at 0, an independent per-partial chorus at 1 |

> **Changed in 2.3.0.** Mode 2 used to be MICRO: just intonation plus a *static*
> deterministic offset. It is now DRIFT, and the offset moves. The parameter
> keeps its range and position, so patches saved before 2.3.0 still load and
> every other control behaves identically — but a patch that used MICRO will
> now drift where it used to sit still. JUST and EQUAL are untouched, and
> DRIFT RATE and DRIFT COHERENCE have no effect in either.

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

**Detuned ensemble.** TUNING DRIFT, SPREAD 40%, COUNT 6, RATE 0.2 Hz, COHERENCE 1. Each partial wanders independently around its true ratio — the slow, never-quite-settling beating of an ensemble tuning up.

**Collective glide.** TUNING DRIFT, COHERENCE 0, RATE 0.1 Hz, SPREAD 20%. The whole stack transposes together, keeping its internal tuning exact while the root breathes.

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

## Factory presets

Three presets shipped from the pitch research. Every figure below is derived,
not chosen to look tidy, and `SPREAD` reads in cents as `knob x 20`.

**Branca Mass** — SPREAD 0.6 (12 cents), DRIFT RATE 0.1 Hz, COHERENCE 1.0.
Massed detuning that shimmers without the centre of the stack moving, which is
what full coherence buys: every partial drifts together rather than wandering
apart. Authored in **DRIFT** tuning mode rather than JUST. Drift is scoped to
DRIFT mode only, so a JUST preset carrying a drift rate would sit completely
static and the rate would be decoration.

**Chatham 7-4** — JUST, first partial 7, SPREAD 0.1 (2 cents), no drift. The
seventh partial is 969 cents above its octave, a flat minor seventh no piano
can play, and the narrow spread keeps it identifiable rather than smeared.

**Raga Drone** — JUST, SPREAD 0, no drift. Deliberately the still one: a fixed
just-intonation reference with no movement of any kind, for tuning against.

## See also

[[String-Mass-Core]] · [[DroneClone]] · [[DroneCore]] · [[Music-Theory]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/HarmonicPressure.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/HarmonicPressure.md)
