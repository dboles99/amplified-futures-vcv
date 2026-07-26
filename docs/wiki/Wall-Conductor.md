# Wall Conductor — 22 HP

![Wall Conductor in VCV Rack](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/rack/WallConductor.png)

Four-channel performance mixer built around one gesture: DENSITY. Rather than four faders, a single knob sweeps the channels in and out one at a time, so an arrangement can be built and dismantled with one hand. PRESSURE drives the sum into saturation, WIDTH spreads it, FEEDBACK recirculates it, and COLLAPSE drops the whole thing instantly with a recovery time you set in advance.

The pairing of COLLAPSE and RECOVERY is the point. Collapse is immediate; the return is shaped, and can take anywhere from 50 ms to ten seconds.

---

## Sound in 60 seconds

1. Add Wall Conductor. Patch sources into **CH1**–**CH4** and **L/R OUT** to your interface.
2. DENSITY starts at 100%, so all four channels are already in. You hear everything.
3. Sweep **DENSITY** down towards zero. Channels drop out one at a time, from CH4 backwards.
4. Raise **PRESSURE**. The sum drives into saturation — a change of character, not just level.
5. Hit **COLLAPSE**. Everything ducks instantly. Release it and the sound returns over the time set by **RECOVERY** — at 30% that is about three seconds.

---

## Signal flow

~~~text
CH1–4 IN ──► per-channel gain = clamp(DENSITY × 4 − i, 0, 1)
                     │
                     ├─ pan = fixed spread × WIDTH
                     └─ constant-power: mixL += sig·cos(θ), mixR += sig·sin(θ)
                     │
        mix += previous output × FEEDBACK   [one-sample bus]
                     │
        out = 5 · tanh(mix × PRESSURE drive / 5) × collapseEnv
                     │                                  ▲
                     ▼                                  │
                 L OUT / R OUT          COLLAPSE ── instant duck,
                                        RECOVERY ── rise over 50 ms – 10 s

V/OCT IN ─────────────────────────────────────────► V/OCT THRU
~~~

DENSITY works as a staged sweep: `gain = clamp(DENSITY × 4 − i, 0, 1)` for channel *i*, so each quarter of the knob's travel brings in one more channel and crossfades it rather than switching it.

---

## Controls

![Wall Conductor panel](https://raw.githubusercontent.com/dboles99/amplified-futures-vcv/master/docs/panels/WallConductor.png)

| Control | Range | Default | What it does |
|---|---|---|---|
| DENSITY | 0–100% | 100% | Sweeps CH1–4 in and out, one at a time, with a crossfade at each stage |
| PRESSURE | 0–100% | 30% | Drive into tanh saturation on the summed signal |
| WIDTH | 0–100% | 70% | Stereo spread, constant-power. 0 is mono |
| FEEDBACK | 0–100% | 0% | One-sample output-to-input recirculation |
| RECOVERY | 0–100% | 30% | How long the sound takes to return after COLLAPSE: 50 ms to 10 s |
| COLLAPSE | button + gate | — | Instant duck to silence. Held while the button is down or the gate is high |

All five knobs have an attenuverter (−1 to +1) and a CV input. COLLAPSE has a gate input instead.

---

## Ports

| Port | Direction | Notes |
|---|---|---|
| CH1–4 | Input | Channel audio |
| DENSITY / PRESSURE / WIDTH / FEEDBACK / RECOVERY CV | Input | One per knob, each via its attenuverter |
| COLLAPSE | Input | Gate — 1 V or above collapses, same as holding the button |
| L OUT / R OUT | Output | Stereo master |
| V/OCT IN → THRU | In / Out | Pass-through |

---

## Patch recipes

**Building an arrangement live.** Four different sources on CH1–4, DENSITY at 0. Sweep it up over a minute and the piece assembles itself in a fixed order. Sweep back down to end.

**Collapse as punctuation.** RECOVERY 10%, roughly a second. [[Pulse]] gate → COLLAPSE. The wall stutters on the grid and recovers fast enough to stay rhythmic.

**Long breath.** RECOVERY 90%, near ten seconds. A single COLLAPSE press becomes a structural event — near-silence, then a slow return. Use once, not repeatedly.

**Slow structural drift.** [[Drift]] SMOOTH → DENSITY CV at a low RATE and attenuverter +0.3. Channels fade in and out over minutes with nobody touching the knob.

**Controlled recirculation.** FEEDBACK 40%, PRESSURE 50%. The output re-enters the mix and saturates further each pass. Above about 60% it becomes its own instrument.

---

## Known pairings

| Module | Routing |
|---|---|
| [[DroneClone]] | The primary channel source |
| [[String-Mass-Core]] | OUT → a channel; DENSITY and COLLAPSE then act over the whole mass |
| [[Drift]] | SMOOTH → DENSITY CV for slow structural change |
| [[Pulse]] | Gate → COLLAPSE for rhythmic stutter |
| [[Collapse-Saturator]] | L/R OUT → IN for edge beyond what PRESSURE gives |
| [[Feedback-Governor]] | Insert in an external loop for feedback with tone control |

---

## See also

[[Mass-Driver]] · [[Choke]] · [[Collapse-Saturator]] · [[Playbooks]]

**Full parameter spec:** [`docs/modules/WallConductor.md`](https://github.com/dboles99/amplified-futures-vcv/blob/master/docs/modules/WallConductor.md)
